//
// Created by Administrator on 2020/8/13.
//

#include <jni.h>
#include <string>
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "libavutil/samplefmt.h"

#ifdef __cplusplus
}
#endif

#ifndef UTILS_CPP
#define UTILS_CPP

bool globalControl = true;

bool getGlobalControl() {
    return globalControl;
}

void setGlobalControl(bool value) {
    globalControl = value;
}

jstring avSampleFormatToString(JNIEnv* env, AVSampleFormat format) {
    const char* s;
    if (format == AV_SAMPLE_FMT_NONE) s = "AV_SAMPLE_FMT_NONE";
    else if (format == AV_SAMPLE_FMT_U8) s = "AV_SAMPLE_FMT_U8";
    else if (format == AV_SAMPLE_FMT_S16) s = "AV_SAMPLE_FMT_S16";
    else if (format == AV_SAMPLE_FMT_S32) s = "AV_SAMPLE_FMT_S32";
    else if (format == AV_SAMPLE_FMT_FLT) s = "AV_SAMPLE_FMT_FLT";
    else if (format == AV_SAMPLE_FMT_DBL) s = "AV_SAMPLE_FMT_DBL";
    else if (format == AV_SAMPLE_FMT_U8P) s = "AV_SAMPLE_FMT_U8P";
    else if (format == AV_SAMPLE_FMT_S16P) s = "AV_SAMPLE_FMT_S16P";
    else if (format == AV_SAMPLE_FMT_S32P) s = "AV_SAMPLE_FMT_S32P";
    else if (format == AV_SAMPLE_FMT_FLTP) s = "AV_SAMPLE_FMT_FLTP";
    else if (format == AV_SAMPLE_FMT_DBLP) s = "AV_SAMPLE_FMT_DBLP";
    else if (format == AV_SAMPLE_FMT_S64) s = "AV_SAMPLE_FMT_S64";
    else if (format == AV_SAMPLE_FMT_S64P) s = "AV_SAMPLE_FMT_S64P";
    else if (format == AV_SAMPLE_FMT_NB) s = "AV_SAMPLE_FMT_NB";
    else s = "UNKNOWN";
    return env->NewStringUTF(s);
}

#endif