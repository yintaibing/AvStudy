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
#include "libswscale/swscale.h"

JNIEXPORT void JNICALL Java_me_yintaibing_avstudy_audio_Mp3FFmpegAudioTrackPlayer_decodeMp3(
        JNIEnv *env,
        jobject instance, jstring filePath) {
    const char *input = env->GetStringUTFChars(filePath, 0);
    LOGE("输入文件：%s", input);
    char errorbuf[1024] = {0};

    av_register_all();

    AVFormatContext *pFormatContext = avformat_alloc_context();

    int ret_code = avformat_open_input(&pFormatContext, input, NULL, NULL);
    if (ret_code != 0) {
        av_strerror(ret_code, errorbuf, sizeof(errorbuf));
        LOGE("打开失败,%d,%s", ret_code, errorbuf);
        return;
    }

    ret_code = avformat_find_stream_info(pFormatContext, NULL);
    if (ret_code < 0) {
        av_strerror(ret_code, errorbuf, sizeof(errorbuf));
        LOGE("获取信息失败,%d,%s", ret_code, errorbuf);
        return;
    }

    int audio_stream_index = -1;

    for (int i = 0; i < pFormatContext->nb_streams; ++i) {
        LOGE("循环流 %d", i);
        if (pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_index = i;
            break;
        }
    }

//    AVCodecContext *pCodecCtx = pFormatContext->streams[audio_stream_index]->codec;
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(NULL);

    avcodec_parameters_to_context(pCodecCtx, pFormatContext->streams[audio_stream_index]->codecpar);

    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);


    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {

    }

    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));

    AVFrame *frame;
    frame = av_frame_alloc();

    SwrContext *swrContext = swr_alloc();

    int got_frame;

    uint8_t *out_buffer = (uint8_t *) av_malloc(44100 * 2);

    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;

    enum AVSampleFormat out_format = AV_SAMPLE_FMT_S16;

    int out_sample_rate = pCodecCtx->sample_rate;

    swr_alloc_set_opts(swrContext, out_ch_layout, out_format, out_sample_rate,
                       pCodecCtx->channel_layout, pCodecCtx->sample_fmt, pCodecCtx->sample_rate, 0,
                       NULL);

    swr_init(swrContext);


    int out_channals_nb = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    jclass my_player = env->GetObjectClass(instance);

    jmethodID createAudio = env->GetMethodID(my_player, "createAudioTrack", "(II)V");

    env->CallVoidMethod(instance,createAudio,44100,out_channals_nb);

    jmethodID playTrack = env->GetMethodID(my_player, "playTrack", "([BI)V");


    while (av_read_frame(pFormatContext, packet)>=0) {
        if (packet->stream_index == audio_stream_index) {

            avcodec_decode_audio4(pCodecCtx,frame,&got_frame,packet);

            if (got_frame) {
//                LOGE("解码");

                swr_convert(swrContext, &out_buffer, 44100 * 2, (const uint8_t **) frame->data, frame->nb_samples);

                int size = av_samples_get_buffer_size(NULL, out_channals_nb, frame->nb_samples, AV_SAMPLE_FMT_S16, 1);

                jbyteArray audio_sample_array = env->NewByteArray(size);

                env->SetByteArrayRegion(audio_sample_array, 0, size, (const jbyte *) out_buffer);
                env->CallVoidMethod(instance,playTrack,audio_sample_array,size);
                env->DeleteLocalRef(audio_sample_array);

            }
        }

    }

    av_frame_free(&frame);
    swr_free(&swrContext);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatContext);

    env->ReleaseStringUTFChars(filePath, input);
}

#ifdef __cplusplus
}
#endif