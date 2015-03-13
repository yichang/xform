#!/bin/bash
#android update project -p ../ --target android-21
#c++ halide.cpp -L ../../../bin -lHalide -I ../../../include -ldl -lpthread -lz
halide_root=../../third_party/halide/
#c++ ../../third_party/halide/apps/local_laplacian/process.cpp\
#c++  ../../third_party/halide/apps/HelloAndroid/jni/halide.cpp\
c++  ../../third_party/halide/apps/local_laplacian/local_laplacian.cpp\
  ../../third_party/halide/apps/style_transfer/style_transfer.cpp\
  ../../third_party/halide/apps/style_transfer/style_transfer_wrapper.cpp\
  ../../third_party/halide/apps/style_transfer/hist.cpp\
  ../../third_party/halide/apps/style_transfer/gradient_norm.cpp\
    ../../third_party/halide/bin/libHalide.a\
    -I ../../third_party/halide/apps/support/\
    -I ../../third_party/halide/include/\
    -L ../../third_party/halide/bin/ -lHalide\
    -ldl -lpthread -lz -lpng16\

#./a.out

# 64-bit MIPS (mips-64-android,mips64) currently does not build since
# llvm will not compile for the R6 version of the ISA without Nan2008
# and the gcc toolchain used by the Android build setup requires those
# two options together.
#for archs in arm-32-android,armeabi arm-32-android-armv7s,armeabi-v7a arm-64-android,arm64-v8a mips-32-android,mips x86-64-android-sse41,x86_64 x86-32-android,x86 ; do
for archs in arm-32-android,armeabi arm-32-android-armv7s,armeabi-v7a, x86-32-android,x86; do
    IFS=,
    set $archs
    hl_target=$1
    android_abi=$2
    mkdir -p halide_generated_$android_abi
    cd halide_generated_$android_abi
    HL_TARGET=$hl_target DYLD_LIBRARY_PATH=../../../third_party/halide/bin LD_LIBRARY_PATH=../../../third_party/halide/bin ../a.out 8
    cd ..
    unset IFS
done


