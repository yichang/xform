# Upload_To_Server
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#LOCAL_MODULE    := plasma
#LOCAL_SRC_FILES := plasma.c

ROOT = ../../

LOCAL_MODULE    := xformRecon
LOCAL_SRC_FILES := recon.cpp\
		   $(ROOT)src/Filter.cpp\
		   $(ROOT)src/Warp.cpp\
		   $(ROOT)src/Recipe.cpp\
		   $(ROOT)src/TransformModel.cpp\
		   $(ROOT)src/ColorSpace.cpp\
		   $(ROOT)src/XImage.cpp\
		   $(ROOT)src/MapImage.cpp\
		   $(ROOT)src/Pyramid.cpp\

LOCAL_CFLAGS += -std=c++11 
LOCAL_CFLAGS += -Ofast
ARCH = armeabi-v7a
LOCAL_C_INCLUDES += $(ROOT)/src/
# LOCAL_C_INCLUDES += $(ROOT)/third_party/eigen-eigen-1306d75b4a21/
LOCAL_C_INCLUDES += $(EIGEN3_INCLUDE_DIR)

include $(ROOT)/third_party/halide/apps/support/Makefile.inc    
LOCAL_C_INCLUDES += $(ROOT)/third_party/halide/apps/support/
LOCAL_C_INCLUDES += $(ROOT)/third_party/halide/include/
LOCAL_C_INCLUDES += $(ROOT)/third_party/halide/apps/local_laplacian/
LOCAL_C_INCLUDES += ./halide_generated_$(TARGET_ARCH_ABI)/

LOCAL_C_INCLUDES += $(NDK)/sources/cxx-stl/gnu-libstdc++/4.9/include/
LOCAL_C_INCLUDES += $(NDK)/sources/cxx-stl/gnu-libstdc++/4.9/libs/$(ARCH)/include/

#LOCAL_MODULE    := ndk1
#LOCAL_SRC_FILES := naive.c

LOCAL_LDLIBS    := -lm -llog -ljnigraphics  halide_generated_$(TARGET_ARCH_ABI)/halide_recon.o halide_generated_$(TARGET_ARCH_ABI)/halide_dequant.o halide_generated_$(TARGET_ARCH_ABI)/halide_resize.o  halide_generated_$(TARGET_ARCH_ABI)/halide_downsample.o  halide_generated_$(TARGET_ARCH_ABI)/halide_highpass.o  halide_generated_$(TARGET_ARCH_ABI)/halide_compute_features.o  halide_generated_$(TARGET_ARCH_ABI)/halide_compute_cfeatures.o  halide_generated_$(TARGET_ARCH_ABI)/halide_recon_separate.o# -lOpenCL -lllvm-a3xx

include $(BUILD_SHARED_LIBRARY)
