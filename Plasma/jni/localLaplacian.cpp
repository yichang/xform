#include <jni.h>
#include <string.h>
#include <stdio.h>
#include <android/log.h>
#include <android/bitmap.h>
#include "local_laplacian.h"
#include "static_image.h"
#define DEBUG_TAG "NDK_AndroidNDK1SampleActivity"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t createPixel(int r, int g, int b, int a) {
  return ((a & 0xff) << 24)
       | ((r & 0xff) << 16)
       | ((g & 0xff) << 8)
       | ((b & 0xff));
}
void copy_to_HImage(JNIEnv * env, const jobject& bitmap, Image<uint16_t>* input){

	AndroidBitmapInfo  info;
	uint32_t          *pixels;
	int                ret;

	AndroidBitmap_getInfo(env, bitmap, &info);

	if(info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
	  __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "NDK:LC: [%s]", "NOT RGBA_8888");
	}

	void* bitmapPixels;
	AndroidBitmap_lockPixels(env, bitmap, &bitmapPixels);
	uint32_t* src = (uint32_t*) bitmapPixels;
	int stride = info.stride;
	int pixelsCount = info.height * info.width;
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "HEIGHT: [%d]",info.height);
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "WIDTH: [%d]", info.width);

	/* Parse */
	for (int x = info.width - 1; x >= 0; --x){
	    for (int y = 0; y < info.height; ++y)
	      {
	    	uint32_t zz = src[info.width * y + x];
                uint16_t b = static_cast<uint16_t>((zz%256));  zz /= 256;
                uint16_t g = static_cast<uint16_t>((zz%256));  zz /= 256;
                uint16_t r = static_cast<uint16_t>((zz%256));  zz /= 256;
	    	(*input)(x,y,0) = r;
	    	(*input)(x,y,1) = g;
	    	(*input)(x,y,2) = b;
	    	//__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "NDK:LC: [%d]", (int)(r*255));
	      }
	}
}

void copy_to_jBuffer(JNIEnv * env, const Image<uint16_t>& output, jobject& bitmap){

	AndroidBitmapInfo  info;
	uint32_t          *pixels;
	int                ret;

	AndroidBitmap_getInfo(env, bitmap, &info);

	if(info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
	  __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "NDK:LC: [%s]", "NOT RGBA_8888");
	}

	void* bitmapPixels;
	AndroidBitmap_lockPixels(env, bitmap, &bitmapPixels);
	uint32_t* src = (uint32_t*) bitmapPixels;
	int stride = info.stride;
	int pixelsCount = info.height * info.width;
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "HEIGHT: [%d]",info.height);
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "WIDTH: [%d]", info.width);

	for (int x = info.width - 1; x >= 0; --x){
	    for (int y = 0; y < info.height; ++y){
	    	int r = static_cast<int>(output(x,y,0));
	    	int g = static_cast<int>(output(x,y,1));
	    	int b = static_cast<int>(output(x,y,2));
	    	src[info.width * y + x] = createPixel(r, g, b, 0xff);
            }
	}
}


void Java_com_example_plasma_Plasma_localLaplacian(JNIEnv * env, jobject obj, jobject bitmap)
{
    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "NDK:LC: [%s]", "Into LocalLaplacian");

    AndroidBitmapInfo  info;
    uint32_t          *pixels;
    int                ret;

    AndroidBitmap_getInfo(env, bitmap, &info);

    if(info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
      __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "NDK:LC: [%s]", "NOT RGBA_8888");
    }

    Image<uint16_t> input(info.width,info.height,3);
    copy_to_HImage(env, bitmap, &input);

    Image<uint16_t> output(input.width(), input.height(), input.channels());
    int levels = 8;
    float alpha =1;
    float beta = 1;
    local_laplacian(levels, alpha/(float(levels-1)), beta, input, output);

    // Clamp to 0-255
    for(int z=0; z < input.channels(); z++){
      for(int x=0; x < input.width(); x++){
    	for(int y=0; y < input.height(); y++){
    		if(output(x,y,z) > 255)
    			output(x,y,z)=255;
    		else if(output(x,y,z)<0)
    			output(x,y,z) = 0;
    	} 
      }
    }
    copy_to_jBuffer(env, output, bitmap);
}

#ifdef __cplusplus
}
#endif
