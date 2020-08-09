#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_window_jni.h>
#include <unistd.h>
#include "include/me_yintaibing_avstudy_video_Mp4Decoder.h"

#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"jason",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"jason",FORMAT,##__VA_ARGS__);

#ifdef __cplusplus
extern "C" {
#endif
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
//编码
#include "libavcodec/avcodec.h"
//封装格式处理
#include "libavformat/avformat.h"
//像素处理
#include "libswscale/swscale.h"

JNIEXPORT void JNICALL Java_me_yintaibing_avstudy_video_Mp4Decoder_decodeMp4
        (JNIEnv *env, jobject thiz, jstring filePath, jobject surface) {
    const char *input = env->GetStringUTFChars(filePath, 0);
    LOGE("输入文件：%s", input);
    char errorbuf[1024] = {0};

    av_register_all();

    AVFormatContext *pFormatCtx = avformat_alloc_context();

    int ret_code = avformat_open_input(&pFormatCtx, input, NULL, NULL);
    if (ret_code != 0) {
        av_strerror(ret_code, errorbuf, sizeof(errorbuf));
        LOGE("打开失败,%d,%s", ret_code, errorbuf);
        return;
    }

    ret_code = avformat_find_stream_info(pFormatCtx, NULL);
    if (ret_code < 0) {
        av_strerror(ret_code, errorbuf, sizeof(errorbuf));
        LOGE("获取信息失败,%d,%s", ret_code, errorbuf);
        return;
    }

    int video_stream_idx = -1;

    for (int i = 0; i < pFormatCtx->nb_streams; ++i) {
        LOGE("循环流 %d", i);
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            break;
        }
    }

    //    获取到解码器上下文
    //AVCodecContext *pCodecCtx = pFormatCtx->streams[video_stream_idx]->codec;
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(NULL);

    avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[video_stream_idx]->codecpar);

    AVCodec *pCodex = avcodec_find_decoder(pCodecCtx->codec_id);

    ret_code = avcodec_open2(pCodecCtx, pCodex, NULL);
    if (ret_code < 0) {
        LOGE("解码失败,%d", ret_code);
        return;
    }

    AVPacket *avPacket = (AVPacket *) av_malloc(sizeof(AVPacket));

    AVFrame *frame;
    frame = av_frame_alloc();

    //surface只支持RGB
    AVFrame *rgb_frame = av_frame_alloc();

    //缓冲区分配内存
    uint8_t *out_buffer = (uint8_t *) av_malloc(
            avpicture_get_size(AV_PIX_FMT_RGBA, pCodecCtx->width, pCodecCtx->height));

    LOGE("宽  %d,  高  %d  ", pCodecCtx->width, pCodecCtx->height);

    //设置yuvFrame的缓冲区，像素格式
    int re = avpicture_fill((AVPicture *) rgb_frame, out_buffer, AV_PIX_FMT_RGBA, pCodecCtx->width,
                            pCodecCtx->height);

    int length = 0;
    int got_frame;

    int frameCount = 0;

    SwsContext *swsContext = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                            pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGBA,
                                            SWS_BICUBIC, NULL, NULL, NULL
    );

    //需用到Android的window
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);

    //视频缓冲区
    ANativeWindow_Buffer outBuffer;

    while (av_read_frame(pFormatCtx, avPacket) >= 0) {

        if (avPacket->stream_index == video_stream_idx) {
            length = avcodec_decode_video2(pCodecCtx, frame, &got_frame, avPacket);
            LOGE(" 获得长度   %d ", length);
            LOGE(" 获得   %d ", got_frame);
            if (got_frame) {
                LOGE("解码=%d", frameCount++);

                //设置window 缓冲器的 format 和size等
                ANativeWindow_setBuffersGeometry(nativeWindow,
                                                 pCodecCtx->width,
                                                 pCodecCtx->height,
                                                 WINDOW_FORMAT_RGBA_8888);

                ANativeWindow_lock(nativeWindow, &outBuffer, NULL);
                //转换为RGB
                sws_scale(swsContext, (const uint8_t *const *) frame->data, frame->linesize, 0,
                          pCodecCtx->height, rgb_frame->data,
                          rgb_frame->linesize);


                //window缓冲区 的内存首地址
                uint8_t *dst = (uint8_t *) outBuffer.bits;

                //window缓冲区一行内存大小 R G B A 所以*4
                int destStride = outBuffer.stride * 4;

                //解码出的 rgb_frame数据首地址
                uint8_t *src = rgb_frame->data[0];

                //解码出的 rgb_frame数据一行 大小
                int srcStride = rgb_frame->linesize[0];

                //一行一行的 拷贝
                for (int i = 0; i < pCodecCtx->height; ++i) {
                    memcpy(dst + i * destStride, src + i * srcStride, srcStride);
                }

                ANativeWindow_unlockAndPost(nativeWindow);

                //模拟速度
                usleep(1000 * 16);
            }
        }
        av_free_packet(avPacket);
    }
    ANativeWindow_release(nativeWindow);
    av_frame_free(&frame);
    avcodec_close(pCodecCtx);
    avformat_free_context(pFormatCtx);
    env->ReleaseStringUTFChars(filePath, input);
}

#ifdef __cplusplus
}
#endif