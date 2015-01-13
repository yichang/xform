#!/bin/bash/
for archs in arm-32-android,armeabi arm-32-android-armv7s,armeabi-v7a, x86-32-android,x86; do
    IFS=,
    set $archs
    hl_target=$1
    android_abi=$2
    mkdir -p halide_generated_$android_abi
    cd halide_generated_$android_abi
    HL_TARGET=$hl_target DYLD_LIBRARY_PATH=../../../third_party/halide/bin LD_LIBRARY_PATH=../../../third_party/halide/bin  ./../../../xform_halide/recon.out
    HL_TARGET=$hl_target DYLD_LIBRARY_PATH=../../../third_party/halide/bin LD_LIBRARY_PATH=../../../third_party/halide/bin  ./../../../xform_halide/resize.out
    HL_TARGET=$hl_target DYLD_LIBRARY_PATH=../../../third_party/halide/bin LD_LIBRARY_PATH=../../../third_party/halide/bin  ./../../../xform_halide/dequant.out
    cd ..
    unset IFS
done


