//
// Created by Administrator on 2020/8/13.
//
#include <jni.h>
#include <android/log.h>

#ifndef AVSTUDY_UTILS_H
#define AVSTUDY_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "libavutil/samplefmt.h"

#ifdef __cplusplus
}
#endif

#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"jason",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"jason",FORMAT,##__VA_ARGS__);

extern bool globalControl;

bool getGlobalControl();

void setGlobalControl(bool value);

jstring avSampleFormatToString(JNIEnv* env, AVSampleFormat format);

#endif //AVSTUDY_UTILS_H
