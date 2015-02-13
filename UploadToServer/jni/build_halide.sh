#!/bin/bash/

num_levels=5
num_bins=4
step_size=16

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
    HL_TARGET=$hl_target DYLD_LIBRARY_PATH=../../../third_party/halide/bin LD_LIBRARY_PATH=../../../third_party/halide/bin  ./../../../xform_halide/recon_separate.out $num_levels $num_bins $step_size
    HL_TARGET=$hl_target DYLD_LIBRARY_PATH=../../../third_party/halide/bin LD_LIBRARY_PATH=../../../third_party/halide/bin  ./../../../xform_halide/downsample.out $num_levels $step_size
    HL_TARGET=$hl_target DYLD_LIBRARY_PATH=../../../third_party/halide/bin LD_LIBRARY_PATH=../../../third_party/halide/bin  ./../../../xform_halide/highpass.out $num_levels $step_size
    HL_TARGET=$hl_target DYLD_LIBRARY_PATH=../../../third_party/halide/bin LD_LIBRARY_PATH=../../../third_party/halide/bin  ./../../../xform_halide/compute_features.out $num_levels $num_bins $step_size
    HL_TARGET=$hl_target DYLD_LIBRARY_PATH=../../../third_party/halide/bin LD_LIBRARY_PATH=../../../third_party/halide/bin  ./../../../xform_halide/compute_cfeatures.out $num_levels $step_size
    cd ..
    unset IFS
done


