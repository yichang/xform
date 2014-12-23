#include <jni.h>
#include <string.h>
#include <stdio.h>
#include <android/log.h>
#include <android/bitmap.h>
#include "XImage.h"
#include "Filter.h"

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

void Java_com_example_plasma_Plasma_boxblur(JNIEnv * env, jobject obj, jobject bitmap)
{

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

        xform::XImage input(info.height, info.width, 3), output;
	//for(int i=0; i < .rows(); i++)
	//	input(i) = ImageType_1(info.height,info.width);

	/* Parse */
	for (int x = info.width - 1; x >= 0; --x){
	    for (int y = 0; y < info.height; ++y)
	      {
	    	uint32_t zz = src[info.width * y + x];
                xform::PixelType b = static_cast<xform::PixelType>((zz%256))/255.0;  zz /= 256;
                xform::PixelType g = static_cast<xform::PixelType>((zz%256))/255.0;  zz /= 256;
                xform::PixelType r = static_cast<xform::PixelType>((zz%256))/255.0;  zz /= 256;
	    	input.at(0)(y,x) = r;
	    	input.at(1)(y,x) = g;
	    	input.at(2)(y,x) = b;
	    	//__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "NDK:LC: [%d]", (int)(r*255));
	      }
	}

	/* Blur */
        xform::Filter filt;
	filt.box(input, 101, &output);

	/* Fill */
	for (int x = info.width - 1; x >= 0; --x){
	    for (int y = 0; y < info.height; ++y)
	      {
	    	int r = static_cast<int>(output.at(0)(y,x) * 255.0);
	    	int g = static_cast<int>(output.at(1)(y,x) * 255.0);
	    	int b = static_cast<int>(output.at(2)(y,x) * 255.0);
	    	src[info.width * y + x] = createPixel(r, g, b, 0xff);
	      }
	}
	AndroidBitmap_unlockPixels(env, bitmap);
}

#ifdef __cplusplus
}
#endif
