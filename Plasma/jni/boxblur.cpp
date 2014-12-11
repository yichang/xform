#include <jni.h>
#include <string.h>
#include <stdio.h>
#include "util/util.h"
#include <android/log.h>

#define DEBUG_TAG "NDK_AndroidNDK1SampleActivity"

#ifdef __cplusplus
extern "C" {
#endif

void Java_com_example_plasma_Plasma_boxblur(JNIEnv * env, jobject blah, jstring logThis)
{
	ImageType_3 hello;
	//printf("hi");
    /*jboolean isCopy;
    const char * szLogThis = (*env)->GetStringUTFChars(env, logThis, &isCopy);

    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "NDK:LC: [%s]", szLogThis);

    (*env)->ReleaseStringUTFChars(env, logThis, szLogThis);*/
}

#ifdef __cplusplus
}
#endif
