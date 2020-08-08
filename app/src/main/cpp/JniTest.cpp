#include <jni.h>
#include <string>
#include <malloc.h>
#include "include/me_yintaibing_avstudy_test_JniTest.h"

char*   Jstring2CStr(JNIEnv*   env,   jstring   jstr){
	char* rtn = NULL;
	jclass   clsstring   =   env->FindClass("java/lang/String");
	jstring   strencode   =   env->NewStringUTF("UTF-8");
	jmethodID   mid   =   env->GetMethodID(clsstring,   "getBytes",   "(Ljava/lang/String;)[B");
	jbyteArray   barr=   (jbyteArray)env->CallObjectMethod(jstr,mid,strencode);
	jsize   alen   =   env->GetArrayLength(barr);
	jbyte*   ba   =   env->GetByteArrayElements(barr,JNI_FALSE);
	if(alen   >   0){
		rtn   =   (char*)malloc(alen+1);
		memcpy(rtn,ba,alen);
		rtn[alen]=0;
	}
	env->ReleaseByteArrayElements(barr,ba,0);
	return rtn;
}

JNIEXPORT jstring JNICALL Java_me_yintaibing_avstudy_test_JniTest_nativeStringFromJNI
  (JNIEnv *env, jobject jobj, jstring inputStr) {
  char* chars = Jstring2CStr(env, inputStr);
  char* concatStr = strncat(chars, " from JNI", 100);
  return env->NewStringUTF(concatStr);
}