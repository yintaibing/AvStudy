#include <jni.h>
#include <string>
#include <unistd.h>
#include <android/log.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "utils.h"
#include "me_yintaibing_avstudy_audio_Mp3FFmpegOpenSLPlayer.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"

/*############### FFmpeg代码 ##################*/

AVFormatContext *pFormatCtx;
int audio_stream_index = -1;
AVCodecContext *pCodecCtx;
AVCodec *pCodec;
AVFrame *frame;
AVPacket *packet;
SwrContext *pSwrCtx;
uint8_t *out_buffer;

int out_sample_rate = 44100;
int out_channels = 2;
uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;// 根据out_channels确定，这里是双声道
enum AVSampleFormat out_format = AV_SAMPLE_FMT_S16;
int out_channels_nb;

/**
 * 初始化FFmpeg
 * 
 * @param input 输入文件
 */
int createFFmpeg(const char *input) {
    LOGE("输入文件：%s", input)
    char errorbuf[1024] = {0};

    // 1、注册协议、格式与编解码器
//    av_register_all();
    avcodec_register_all();


    // 2、创建媒体格式上下文，打开源文件
    pFormatCtx = avformat_alloc_context();
    // 打开源文件
    int ret_code = avformat_open_input(&pFormatCtx, input, nullptr, nullptr);
    if (ret_code != 0) {
        av_strerror(ret_code, errorbuf, sizeof(errorbuf));
        LOGE("打开失败,%d,%s", ret_code, errorbuf)
        return -1;
    }


    // 3、寻找目标流（视频or音频流）
    ret_code = avformat_find_stream_info(pFormatCtx, nullptr);
    if (ret_code < 0) {
        av_strerror(ret_code, errorbuf, sizeof(errorbuf));
        LOGE("获取信息失败,%d,%s", ret_code, errorbuf)
        return -1;
    }
    // 遍历所有流，确定目标流的下标
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
    pCodecCtx = avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[audio_stream_index]->codecpar);
    // 得到、打开编解码器，新老版本调用相同
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    ret_code = avcodec_open2(pCodecCtx, pCodec, nullptr);
    if (ret_code < 0) {
        av_strerror(ret_code, errorbuf, sizeof(errorbuf));
        LOGE("打开编解码器失败,%d,%s", ret_code, errorbuf);
        return -1;
    }


    // 5、初始化格式转换上下文
    pSwrCtx = swr_alloc();
    swr_alloc_set_opts(pSwrCtx, out_ch_layout, out_format, out_sample_rate,
                       pCodecCtx->channel_layout, pCodecCtx->sample_fmt, pCodecCtx->sample_rate, 0,
                       nullptr);
    swr_init(pSwrCtx);


    // 6、读取流内容，解码并处理
    out_channels_nb = av_get_channel_layout_nb_channels(out_ch_layout);
    // 存放数据包和数据帧
    packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    frame = av_frame_alloc();
    // 分配一个转换后的数据的缓存
    int out_buffer_size = av_samples_get_buffer_size(
            nullptr, out_channels_nb, pCodecCtx->frame_size, out_format, 0);
    out_buffer = (uint8_t *) av_malloc(out_buffer_size);


    LOGE("FFmpeg初始化完成")
    return 0;
}

/**
 * 解码后的PCM数据
 * @param pcm
 * @param size
 */
void ffmpegDecode(SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue) {
    int ret_code;
    bool enqueued = false;
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        if (packet->stream_index == audio_stream_index) {
            ret_code = avcodec_send_packet(pCodecCtx, packet);
            if (ret_code < 0) {
                LOGE("avcodec_send_packet失败,%d", ret_code)
                setGlobalControl(false);
                return;
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
                    return;
                }

                swr_convert(pSwrCtx, &out_buffer, out_sample_rate * out_channels,
                            (const uint8_t **) frame->data, frame->nb_samples);
                int out_buffer_size = av_samples_get_buffer_size(
                        nullptr, out_channels_nb, frame->nb_samples, out_format, 1);

                // 塞给OpenSL播放器缓冲队列
                (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, out_buffer, out_buffer_size);
                enqueued = true;
            }
            if (enqueued) {
                break;
            }
        }
    }
}

/**
 * 释放FFmpeg
 */
void releaseFFmpeg() {
    av_packet_free(&packet);
    av_frame_free(&frame);
    av_free(out_buffer);
    swr_free(&pSwrCtx);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
}

/*############### OpenSL代码 ##################*/
// OpenSL接口
SLObjectItf slObjectItf = nullptr;
// 引擎接口
SLEngineItf engineEngine = nullptr;
// 混音器接口
SLObjectItf outputMixObject = nullptr;

SLEnvironmentalReverbItf outputMixEnvironmentalReverb = nullptr;

const SLEnvironmentalReverbSettings settings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;

// 播放器对象和接口
SLObjectItf pPlayer;
SLPlayItf playerI;

//缓冲队列接口
SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;

SLVolumeItf bqPlayerVolume;

/**
 * 停止OpenSL
 */
void openSLShutdown() {
    LOGE("openSLShutdown")
    if (pPlayer != nullptr) {
        (*pPlayer)->Destroy(pPlayer);
        pPlayer = nullptr;
        playerI = nullptr;
        bqPlayerBufferQueue = nullptr;
        bqPlayerVolume = nullptr;
    }
    if (outputMixObject != nullptr) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = nullptr;
        outputMixEnvironmentalReverb = nullptr;
    }
    if (slObjectItf != nullptr) {
        (*slObjectItf)->Destroy(slObjectItf);
        slObjectItf = nullptr;
        engineEngine = nullptr;
    }
    releaseFFmpeg();
}


/**
 * OpenSL播放器缓冲队列回调方法
 */
void openslPlayerCallBack(SLAndroidSimpleBufferQueueItf caller, void *pContext) {
    if (getGlobalControl()) {
        ffmpegDecode(bqPlayerBufferQueue);
    } else {
        releaseFFmpeg();
    }
}

/**
 * 准备OpenSL
 */
void openSLPlay() {
    SLresult sLresult;

    // 1、初始化OpenSL和引擎
    slCreateEngine(&slObjectItf, 0, nullptr, 0, nullptr, nullptr);
    (*slObjectItf)->Realize(slObjectItf, SL_BOOLEAN_FALSE);
    (*slObjectItf)->GetInterface(slObjectItf, SL_IID_ENGINE, &engineEngine);


    // 2、创建混音器
    (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, 0, 0);
    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    sLresult = (*outputMixObject)->GetInterface(
            outputMixObject, SL_IID_ENVIRONMENTALREVERB, &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == sLresult) {
        (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &settings);
    }


    // 3、创建播放器
    SLDataLocator_AndroidSimpleBufferQueue andorid_queue =
            {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    // 参数要与FFmepg解析输出参数对上
    SLDataFormat_PCM pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                            SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource slDataSource = {&andorid_queue, &pcm};
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink slDataSink = {&outputMix, nullptr};
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,
            /*SL_IID_MUTESOLO,*/ SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            /*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_TRUE};
    sLresult = (*engineEngine)->CreateAudioPlayer(engineEngine, &pPlayer, &slDataSource,
                                                  &slDataSink, 3, ids, req);
    LOGE("初始化播放器是否成功  %d    bqPlayerObject  %d", sLresult, pPlayer);
    (*pPlayer)->Realize(pPlayer, SL_BOOLEAN_FALSE);
    (*pPlayer)->GetInterface(pPlayer, SL_IID_PLAY, &playerI);


    // 4、获取缓冲队列，注册队列回调
    (*pPlayer)->GetInterface(pPlayer, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueue);
    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, openslPlayerCallBack, nullptr);


    // 获取音量接口
    (*pPlayer)->GetInterface(pPlayer, SL_IID_VOLUME, &bqPlayerVolume);

    // 切换到播放状态
    (*playerI)->SetPlayState(playerI, SL_PLAYSTATE_PLAYING);

    // 调用一次缓冲队列的回调，通知FFmpeg开始读取、解析音频流
    openslPlayerCallBack(bqPlayerBufferQueue, nullptr);
}

/*############### JNI代码 ##################*/

JNIEXPORT void JNICALL Java_me_yintaibing_avstudy_audio_Mp3FFmpegOpenSLPlayer_nativeStart
        (JNIEnv *env, jobject, jstring filePath) {
    const char *input = env->GetStringUTFChars(filePath, 0);
    if (createFFmpeg(input) == 0) {
        openSLPlay();
    }
    env->ReleaseStringUTFChars(filePath, input);
}

JNIEXPORT void JNICALL Java_me_yintaibing_avstudy_audio_Mp3FFmpegOpenSLPlayer_nativeStop
        (JNIEnv *, jobject) {
    setGlobalControl(false);
    openSLShutdown();
}

#ifdef __cplusplus
}
#endif