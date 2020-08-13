//
// Created by Administrator on 2020/8/10.
//
#include <jni.h>

#ifndef _Included_me_yintaibing_avstudy_audio_Mp3FFmpegAudioTrackPlayer
#define _Included_me_yintaibing_avstudy_audio_Mp3FFmpegAudioTrackPlayer
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     me_yintaibing_avstudy_video_Mp4Decoder
 */
JNIEXPORT void JNICALL Java_me_yintaibing_avstudy_audio_Mp3FFmpegAudioTrackPlayer_nativeStart
        (JNIEnv *, jobject, jstring);

JNIEXPORT void JNICALL Java_me_yintaibing_avstudy_audio_Mp3FFmpegAudioTrackPlayer_nativeStop
        (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
