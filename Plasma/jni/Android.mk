# Plasma
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

NDK = /usr/local/Cellar/android-ndk/r10d/
ARCH = armeabi-v7a

LOCAL_MODULE    := native
LOCAL_ARM_MODE  := arm
LOCAL_SRC_FILES := localLaplacian.cpp
LOCAL_LDFLAGS   := -Ljni
LOCAL_LDLIBS    := -ljnigraphics -lm -llog -landroid halide_generated_$(TARGET_ARCH_ABI)/local_laplacian.o # -lOpenCL -lllvm-a3xx
LOCAL_STATIC_LIBRARIES := android_native_app_glue
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../include $(LOCAL_PATH)/halide_generated_$(TARGET_ARCH_ABI)/ $(LOCAL_PATH)/../../third_party/halide/apps/support/ $(LOCAL_PATH)/../../third_party/halide/include/
# LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../include $(LOCAL_PATH)/halide_generated_$(TARGET_ARCH_ABI)/ $(LOCAL_PATH)/../../third_party/halide/apps/support/
LOCAL_C_INCLUDES += $(NDK)/sources/cxx-stl/gnu-libstdc++/4.9/include/
LOCAL_C_INCLUDES += $(NDK)/sources/cxx-stl/gnu-libstdc++/4.9/libs/$(ARCH)/include/

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
