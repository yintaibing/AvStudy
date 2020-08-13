#include <jni.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>
#include <unistd.h>
#include "utils.h"
#include "me_yintaibing_avstudy_audio_Mp3FFmpegAudioTrackPlayer.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"

/**
 * 解码后播放
 *
 * @param jni JNI环境
 * @param jObj Java类对象
 * @param playFunc Java的播放方法
 * @param frame 数据帧
 * @param out_buffer 解码数据存放的缓存
 * @param out_format 解码输出位深度
 * @param out_channels_nb 解码输出声道数
 */
void playAfterDecode(
        JNIEnv *jni,
        jobject jObj,
        jmethodID playFunc,
        AVFrame *frame,
        uint8_t *out_buffer,
        enum AVSampleFormat out_format,
        int out_channels_nb) {
    int size = av_samples_get_buffer_size(nullptr, out_channels_nb,
                                          frame->nb_samples, out_format, 1);
    jbyteArray jBytes = jni->NewByteArray(size);
    jni->SetByteArrayRegion(jBytes, 0, size, (const jbyte *) out_buffer);
    jni->CallVoidMethod(jObj, playFunc, jBytes, size);
    jni->DeleteLocalRef(jBytes);
}

/**
 * 老版本FFmpeg解码方式
 *
 * @param jni JNI环境
 * @param jObj Java类对象
 * @param playFunc Java的播放方法
 * @param pFormatCtx 媒体格式上下文
 * @param pCodecCtx 编解码器上下文
 * @param audio_stream_index 音频流下标
 * @param out_sample_rate 解码输出采样率
 * @param out_channels 解码输出声道数
 * @param out_ch_layout 解码输出声道模式
 * @param out_format 解码输出位深度
 */
void oldVersionDecode(
        JNIEnv *jni,
        jobject jObj,
        jmethodID playFunc,
        AVFormatContext *pFormatCtx,
        AVCodecContext *pCodecCtx,
        SwrContext *pSwrContext,
        int audio_stream_index,
        int out_sample_rate,
        int out_channels,
        uint64_t out_ch_layout,
        enum AVSampleFormat out_format) {
    // 6、读取流内容，解码并处理
    int out_channels_nb = av_get_channel_layout_nb_channels(out_ch_layout);
    // 存放数据包和数据帧
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    AVFrame *frame = av_frame_alloc();
    // 分配一个转换后的数据的缓存
    int out_buffer_size = av_samples_get_buffer_size(
            nullptr,
            av_get_channel_layout_nb_channels(out_ch_layout),
            pCodecCtx->frame_size,
            out_format, 0);
    uint8_t *out_buffer = (uint8_t *) av_malloc(out_buffer_size);
    int got_frame;

    // 解码直到文件结束
    while (getGlobalControl() && av_read_frame(pFormatCtx, packet) >= 0) {
        if (packet->stream_index == audio_stream_index) {
            // 解码数据包
            avcodec_decode_audio4(pCodecCtx, frame, &got_frame, packet);
            if (got_frame) {
                swr_convert(pSwrContext, &out_buffer, out_sample_rate * out_channels,
                            (const uint8_t **) frame->data, frame->nb_samples);
                playAfterDecode(jni, jObj, playFunc, frame, out_buffer, out_format,
                                out_channels_nb);
            }
        }
    }

    av_frame_free(&frame);
    av_packet_free(&packet);
}

/**
 * 新版本FFmpeg解码方式
 *
 * @param jni JNI环境
 * @param jObj Java类对象
 * @param playFunc Java的播放方法
 * @param pFormatCtx 媒体格式上下文
 * @param pCodecCtx 编解码器上下文
 * @param audio_stream_index 音频流下标
 * @param out_sample_rate 解码输出采样率
 * @param out_channels 解码输出声道数
 * @param out_ch_layout 解码输出声道模式
 * @param out_format 解码输出位深度
 */
void newVersionDecode(
        JNIEnv *jni,
        jobject jObj,
        jmethodID playFunc,
        AVFormatContext *pFormatCtx,
        AVCodecContext *pCodecCtx,
        SwrContext *pSwrContext,
        int audio_stream_index,
        int out_sample_rate,
        int out_channels,
        uint64_t out_ch_layout,
        enum AVSampleFormat out_format) {
    // 6、读取流内容，解码并处理
    int out_channels_nb = av_get_channel_layout_nb_channels(out_ch_layout);
    // 存放数据包和数据帧
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    AVFrame *frame = av_frame_alloc();
    // 分配一个转换后的数据的缓存
    int out_buffer_size = av_samples_get_buffer_size(
            nullptr,
            av_get_channel_layout_nb_channels(out_ch_layout),
            pCodecCtx->frame_size,
            out_format, 0);
    uint8_t *out_buffer = (uint8_t *) av_malloc(out_buffer_size);
    int ret_code;

    // 解码直到文件结束
    while (getGlobalControl() && av_read_frame(pFormatCtx, packet) >= 0) {
        if (packet->stream_index == audio_stream_index) {
            ret_code = avcodec_send_packet(pCodecCtx, packet);
            if (ret_code < 0) {
                LOGE("avcodec_send_packet失败,%d", ret_code)
                setGlobalControl(false);
                break;
            }
            av_packet_unref(packet);
            while (ret_code >= 0) {
                ret_code = avcodec_receive_frame(pCodecCtx, frame);
                if (ret_code == AVERROR(EAGAIN) || ret_code == AVERROR_EOF) {
                    // 出现此错误，基本是因为需要等待塞给解码器更多packet，无需停止整个解码流程
                    break;
                } else if (ret_code < 0) {
                    LOGE("avcodec_receive_frame失败,%d", ret_code)
                    setGlobalControl(false);
                    break;
                }

                swr_convert(pSwrContext, &out_buffer, out_sample_rate * out_channels,
                            (const uint8_t **) frame->data, frame->nb_samples);
                playAfterDecode(jni, jObj, playFunc, frame, out_buffer, out_format,
                                out_channels_nb);
            }
        }
    }

    av_frame_free(&frame);
    av_packet_free(&packet);
}

/**
 * JNI 调用，开始播放
 *
 * @param filePath 文件路径
 */
JNIEXPORT void JNICALL Java_me_yintaibing_avstudy_audio_Mp3FFmpegAudioTrackPlayer_nativeStart(
        JNIEnv *jni,
        jobject jObj,
        jstring filePath) {
    const char *input = jni->GetStringUTFChars(filePath, nullptr);
    LOGE("输入文件：%s", input)
    char errorbuf[1024] = {0};

    // 1、注册协议、格式与编解码器
//    av_register_all();
    avcodec_register_all();


    // 2、创建媒体格式上下文，打开源文件
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    // 打开源文件
    int ret_code = avformat_open_input(&pFormatCtx, input, nullptr, nullptr);
    if (ret_code != 0) {
        av_strerror(ret_code, errorbuf, sizeof(errorbuf));
        LOGE("打开失败,%d,%s", ret_code, errorbuf)
        return;
    }


    // 3、寻找目标流（视频or音频流）
    ret_code = avformat_find_stream_info(pFormatCtx, nullptr);
    if (ret_code < 0) {
        av_strerror(ret_code, errorbuf, sizeof(errorbuf));
        LOGE("获取信息失败,%d,%s", ret_code, errorbuf)
        return;
    }
    // 遍历所有流，确定目标流的下标
    int audio_stream_index = -1;
    for (int i = 0; i < pFormatCtx->nb_streams; ++i) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_index = i;
            break;
        }
    }


    // 4、得到目标流对应的编解码器
    // 比较老的版本，直接从流对象中得到编解码器
//    AVCodecContext *pCodecCtx = pFormatCtx->streams[audio_stream_index]->codec;
    // 新版本，需手动创建编解码器，然后将流的编码器信息给过去
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[audio_stream_index]->codecpar);
    // 得到、打开编解码器，新老版本调用相同
    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    ret_code = avcodec_open2(pCodecCtx, pCodec, nullptr);
    if (ret_code < 0) {
        av_strerror(ret_code, errorbuf, sizeof(errorbuf));
        LOGE("打开编解码器失败,%d,%s", ret_code, errorbuf);
        return;
    }


    // 指定解码后的参数
    //int out_sample_rate = pCodecCtx->sample_rate;
    int out_sample_rate = 44100;
    int out_channels = 2;
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;// 根据out_channels确定，这里是双声道
    enum AVSampleFormat out_format = AV_SAMPLE_FMT_S16;


    // 获取Java类对象，调用Java方法
    jclass jclazz = jni->GetObjectClass(jObj);
    jmethodID onGotAudioInfo = jni->GetMethodID(jclazz, "onGotAudioInfo",
                                                "(IJLjava/lang/String;)V");
    jmethodID createAudio = jni->GetMethodID(jclazz, "createAudioTrack", "(II)V");
    jmethodID playTrack = jni->GetMethodID(jclazz, "playTrack", "([BI)V");
    jni->CallVoidMethod(jObj, onGotAudioInfo,
                        pCodecCtx->sample_rate,
                        (jlong) pCodecCtx->channel_layout,
                        avSampleFormatToString(jni, pCodecCtx->sample_fmt));
    jni->CallVoidMethod(jObj, createAudio, out_sample_rate, out_channels);


    // 5、初始化格式转换上下文
    SwrContext *pSwrContext = swr_alloc();
    swr_alloc_set_opts(pSwrContext, out_ch_layout, out_format, out_sample_rate,
                       pCodecCtx->channel_layout, pCodecCtx->sample_fmt, pCodecCtx->sample_rate, 0,
                       nullptr);
    swr_init(pSwrContext);


    // 6、读取流内容，解码并处理
    oldVersionDecode(jni, jObj, playTrack, pFormatCtx, pCodecCtx, pSwrContext, audio_stream_index,
            out_sample_rate, out_channels, out_ch_layout, out_format);
//    newVersionDecode(jni, jObj, playTrack, pFormatCtx, pCodecCtx, pSwrContext, audio_stream_index,
//                     out_sample_rate, out_channels, out_ch_layout, out_format);


    // 7、释放资源
    swr_free(&pSwrContext);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
    jni->ReleaseStringUTFChars(filePath, input);
    setGlobalControl(false);
}

/**
 * JNI调用，停止播放
 */
JNIEXPORT void JNICALL Java_me_yintaibing_avstudy_audio_Mp3FFmpegAudioTrackPlayer_nativeStop(
        JNIEnv *jni,
        jobject jObj) {
    setGlobalControl(false);
}

#ifdef __cplusplus
}
#endif