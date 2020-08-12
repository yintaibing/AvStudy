#include <jni.h>
#include <string>
#include <android/log.h>
#include <unistd.h>
#include "me_yintaibing_avstudy_audio_Mp3FFmpegAudioTrackPlayer.h"

#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"jason",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"jason",FORMAT,##__VA_ARGS__);

#ifdef __cplusplus
extern "C" {
#endif
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"

JNIEXPORT void JNICALL Java_me_yintaibing_avstudy_audio_Mp3FFmpegAudioTrackPlayer_decodeMp3(
        JNIEnv *javaEnv,
        jobject instance,
        jstring filePath) {
    const char *input = javaEnv->GetStringUTFChars(filePath, 0);
    LOGE("输入文件：%s", input);
    char errorbuf[1024] = {0};

    // 注册所有支持的编解码器
    av_register_all();


    // 创建媒体格式上下文
    AVFormatContext *pFormatContext = avformat_alloc_context();
    // 打开源文件
    int ret_code = avformat_open_input(&pFormatContext, input, nullptr, nullptr);
    if (ret_code != 0) {
        av_strerror(ret_code, errorbuf, sizeof(errorbuf));
        LOGE("打开失败,%d,%s", ret_code, errorbuf);
        return;
    }


    // 解析流信息
    ret_code = avformat_find_stream_info(pFormatContext, nullptr);
    if (ret_code < 0) {
        av_strerror(ret_code, errorbuf, sizeof(errorbuf));
        LOGE("获取信息失败,%d,%s", ret_code, errorbuf);
        return;
    }
    // 遍历所有流，确定目标流的下标
    int audio_stream_index = -1;
    for (int i = 0; i < pFormatContext->nb_streams; ++i) {
        if (pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_index = i;
            break;
        }
    }


    // 比较老的版本，直接从流对象中得到编解码器
//    AVCodecContext *pCodecCtx = pFormatContext->streams[audio_stream_index]->codec;
    // 新版本，需手动创建编解码器，然后将流的编码器信息给过去
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(pCodecCtx, pFormatContext->streams[audio_stream_index]->codecpar);
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
    jclass jclazz = javaEnv->GetObjectClass(instance);
    jmethodID onGotAudioInfo = javaEnv->GetMethodID(jclazz, "onGotAudioInfo",
                                                    "(IILjava/lang/Long;)V");
    jmethodID createAudio = javaEnv->GetMethodID(jclazz, "createAudioTrack", "(II)V");
    jmethodID playTrack = javaEnv->GetMethodID(jclazz, "playTrack", "([BI)V");
    javaEnv->CallVoidMethod(instance, onGotAudioInfo,
            pCodecCtx->sample_rate, pCodecCtx->sample_fmt, (jlong)pCodecCtx->channel_layout);
    javaEnv->CallVoidMethod(instance, createAudio, out_sample_rate, out_channels);


    // 存放数据包和数据帧
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    AVFrame *frame = av_frame_alloc();
    // 分配一个转换后的数据的缓存
    uint8_t *out_buffer = (uint8_t *) av_malloc(out_sample_rate * out_channels);

    SwrContext *swrContext = swr_alloc();

    int got_frame;

    swr_alloc_set_opts(swrContext, out_ch_layout, out_format, out_sample_rate,
                       pCodecCtx->channel_layout, pCodecCtx->sample_fmt, pCodecCtx->sample_rate, 0,
                       nullptr);

    swr_init(swrContext);


    int out_channals_nb = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);


    while (av_read_frame(pFormatContext, packet)>=0) {
        if (packet->stream_index == audio_stream_index) {

            avcodec_decode_audio4(pCodecCtx,frame,&got_frame,packet);

            if (got_frame) {
//                LOGE("解码");

                swr_convert(swrContext, &out_buffer, 44100 * 2, (const uint8_t **) frame->data, frame->nb_samples);

                int size = av_samples_get_buffer_size(nullptr, out_channals_nb, frame->nb_samples, AV_SAMPLE_FMT_S16, 1);

                jbyteArray audio_sample_array = javaEnv->NewByteArray(size);

                javaEnv->SetByteArrayRegion(audio_sample_array, 0, size, (const jbyte *) out_buffer);
                javaEnv->CallVoidMethod(instance, playTrack, audio_sample_array, size);
                javaEnv->DeleteLocalRef(audio_sample_array);

            }
        }

    }

    av_frame_free(&frame);
    swr_free(&swrContext);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatContext);

    javaEnv->ReleaseStringUTFChars(filePath, input);
}

#ifdef __cplusplus
}
#endif