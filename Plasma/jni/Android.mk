LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#LOCAL_MODULE    := plasma
#LOCAL_SRC_FILES := plasma.c

ROOT = ../../
NDK = ../../../android-ndk-r10d/

LOCAL_MODULE    := filter
LOCAL_SRC_FILES := boxblur.cpp\
		   $(ROOT)src/filter.cpp\
		   $(ROOT)src/x_image.cpp\
LOCAL_CFLAGS += -std=c++11 
LOCAL_CFLAGS += -Ofast
ARCH = armeabi-v7a
LOCAL_C_INCLUDES += $(ROOT)/src/
LOCAL_C_INCLUDES += $(ROOT)/third_party/eigen-eigen-1306d75b4a21/
LOCAL_C_INCLUDES += $(NDK)/sources/cxx-stl/gnu-libstdc++/4.9/include/
LOCAL_C_INCLUDES += $(NDK)/sources/cxx-stl/gnu-libstdc++/4.9/libs/$(ARCH)/include/

#LOCAL_MODULE    := ndk1
#LOCAL_SRC_FILES := naive.c

LOCAL_LDLIBS    := -lm -llog -ljnigraphics 

include $(BUILD_SHARED_LIBRARY)
