#include <jni.h>
#include <string>
#include <assert.h>
#include <unistd.h>
#include <android/log.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "me_yintaibing_avstudy_audio_Mp3FFmpegOpenSLPlayer.h"

#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"jason",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"jason",FORMAT,##__VA_ARGS__);

#ifdef __cplusplus
extern "C" {
#endif
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"

/*############### FFmpeg代码 ##################*/

AVFormatContext *pFormatCtx;
int audio_stream_idx = -1;
AVCodecContext *pCodecCtx;
AVCodec *pCodec;
AVFrame * frame;
AVPacket * packet;

SwrContext *swrCtx;
uint8_t *out_buffer;

/**
 * 初始化FFmpeg
 * @param rate
 * @param channal
 * @param input
 * @return
 */
int createFFmpeg(int *rate,int *channal,const char* input) {
    av_register_all();

    pFormatCtx = avformat_alloc_context();

    if(avformat_open_input(&pFormatCtx,input,NULL,NULL)!=0) {
        LOGI("打开失败");
        return -1 ;
    }

    if(avformat_find_stream_info(pFormatCtx,NULL)<0){
        LOGI("获取流信息失败")
        return -1;
    }

    for (int i = 0; i < pFormatCtx->nb_streams; ++i) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_idx = i;
            LOGI("找到音频流");
            break;
        }
    }

    pCodecCtx = pFormatCtx->streams[audio_stream_idx]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

    if(avcodec_open2(pCodecCtx,pCodec,NULL)<0) {
        LOGI("avcodec_open failed");
    }

    frame = av_frame_alloc();

    packet = (AVPacket *) av_malloc(sizeof(AVPacket));

    swrCtx = swr_alloc();

    swr_alloc_set_opts(swrCtx,
                       AV_CH_LAYOUT_STEREO,
                       AV_SAMPLE_FMT_S16,
                       pCodecCtx->sample_rate,
                       pCodecCtx->channel_layout,
                       pCodecCtx->sample_fmt,
                       pCodecCtx->sample_rate,0,NULL);

    swr_init(swrCtx);

    *rate = pCodecCtx->sample_rate;
    *channal = pCodecCtx->channels;
    out_buffer = (uint8_t *) av_malloc(44100 * 2);
    LOGI("初始刷完毕");
    return 0;
}

/**
 * 解码后的PCM数据
 * @param pcm
 * @param size
 */
void getPCM(void **pcm,size_t *size) {
    int got_frame;
    int out_channer_nb = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    while (av_read_frame(pFormatCtx, packet)>=0) {

        if (packet->stream_index == audio_stream_idx) {

            avcodec_decode_audio4(pCodecCtx, frame, & got_frame, packet);
            if (got_frame) {
                LOGI("解码");

                swr_convert(swrCtx, &out_buffer, 44100 * 2, (const uint8_t **) frame->data, frame->nb_samples);
                int out_buffer_size=av_samples_get_buffer_size(NULL, out_channer_nb, frame->nb_samples,AV_SAMPLE_FMT_S16, 1);
                *pcm =out_buffer;
                *size =out_buffer_size;
                break;
            }
        }
    }
}

/**
 * 释放FFmpeg
 */
void releaseFFmpeg() {
    av_free_packet(packet);
    av_frame_free(&frame);
    av_free(out_buffer);
    swr_free(&swrCtx);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
}

/*############### OpenSL代码 ##################*/
SLObjectItf slObjectItf = NULL;
//混音器
SLObjectItf outputMixObject = NULL;
SLEngineItf engineEngine = NULL;

SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;

const SLEnvironmentalReverbSettings settings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;

SLObjectItf pPlayer;
SLPlayItf playerI;

//缓冲器队列接口
SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;

SLVolumeItf bqPlayerVolume;

size_t bufferSize;
void *pcmBuffer;
void playerCallBack( SLAndroidSimpleBufferQueueItf caller,void *pContext){
    bufferSize = 0;

    getPCM(&pcmBuffer, &bufferSize);
    if (pcmBuffer != NULL && bufferSize != 0) {
        SLresult result;

        result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue,pcmBuffer,bufferSize);

//        assert()
        LOGE("keke  bqPlayerCallback :%d", result);
    }
}

void openSLPlay(const char *input) {
    SLresult sLresult;

    //初始化引擎
    slCreateEngine(&slObjectItf,0,NULL,0,NULL,NULL);

    (*slObjectItf)->Realize(slObjectItf, SL_BOOLEAN_FALSE);

    (*slObjectItf)->GetInterface(slObjectItf,SL_IID_ENGINE,&engineEngine);
    LOGE("地址 %p",engineEngine);

    (*engineEngine)->CreateOutputMix(engineEngine,&outputMixObject,0,0,0);

    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);

    sLresult = (*outputMixObject)->GetInterface(outputMixObject,SL_IID_ENVIRONMENTALREVERB,&outputMixEnvironmentalReverb);

    if (SL_RESULT_SUCCESS == sLresult) {
        (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(outputMixEnvironmentalReverb,&settings);

    }

    int rate;
    int channels;
    createFFmpeg(&rate,&channels,input);
    LOGE(" 比特率%d  ,channels %d" ,rate,channels);

    SLDataLocator_AndroidSimpleBufferQueue andorid_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};

    SLDataFormat_PCM pcm = {SL_DATAFORMAT_PCM,2,SL_SAMPLINGRATE_44_1,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                            SL_BYTEORDER_LITTLEENDIAN};

    SLDataSource slDataSource = {&andorid_queue,&pcm};

    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX,outputMixObject};

    SLDataSink slDataSink = {&outputMix,NULL};

    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,
            /*SL_IID_MUTESOLO,*/ SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            /*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_TRUE};

    sLresult =(*engineEngine)->CreateAudioPlayer(engineEngine,&pPlayer,&slDataSource,&slDataSink,3,ids,req);

    LOGE("初始化播放器是否成功  %d    bqPlayerObject  %d",sLresult,pPlayer);

    (*pPlayer)->Realize(pPlayer,SL_BOOLEAN_FALSE);

    (*pPlayer)->GetInterface(pPlayer,SL_IID_PLAY,&playerI);

    //注册回调缓冲区 //获取缓冲队列接口
    (*pPlayer)->GetInterface(pPlayer, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueue);
    LOGE("获取缓冲区数据");

    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue,playerCallBack,NULL);

    //    获取音量接口
    (*pPlayer)->GetInterface(pPlayer, SL_IID_VOLUME, &bqPlayerVolume);

//    获取播放状态接口
    (*playerI)->SetPlayState(playerI, SL_PLAYSTATE_PLAYING);

    playerCallBack(bqPlayerBufferQueue, NULL);
}

void openSLShutdown() {
    if (pPlayer != NULL) {
        (*pPlayer)->Destroy(pPlayer);
        pPlayer = NULL;
        playerI = NULL;
        bqPlayerBufferQueue = NULL;
        bqPlayerVolume = NULL;
    }
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }
    if (slObjectItf != NULL) {
        (*slObjectItf)->Destroy(slObjectItf);
        slObjectItf = NULL;
        engineEngine = NULL;
    }
    releaseFFmpeg();
}

/*############### JNI代码 ##################*/

JNIEXPORT void JNICALL Java_me_yintaibing_avstudy_audio_Mp3FFmpegOpenSLPlayer_nativeStart
        (JNIEnv *env, jobject, jstring filePath) {
    const char *input = env->GetStringUTFChars(filePath, 0);
    openSLPlay(input);
    env->ReleaseStringUTFChars(filePath, input);
}

JNIEXPORT void JNICALL Java_me_yintaibing_avstudy_audio_Mp3FFmpegOpenSLPlayer_nativeStop
        (JNIEnv *, jobject) {
    openSLShutdown();
}

#ifdef __cplusplus
}
#endif