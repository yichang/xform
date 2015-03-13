#include <Halide.h>
#include "const.h"
using namespace Halide;

Var x, y;

// Downsample with a 1 3 3 1 filter
Func downsample(Func f) {
    Func downx, downy;

    downx(x, y, _) = (f(2*x-1, y, _) + 3.0f * (f(2*x, y, _) + f(2*x+1, y, _)) + f(2*x+2, y, _)) / 8.0f;
    downy(x, y, _) = (downx(x, 2*y-1, _) + 3.0f * (downx(x, 2*y, _) + downx(x, 2*y+1, _)) + downx(x, 2*y+2, _)) / 8.0f;

    return downy;
}

// Upsample using bilinear interpolation
Func upsample(Func f) {
    Func upx, upy;

    upx(x, y, _) = 0.25f * f((x/2) - 1 + 2*(x % 2), y, _) + 0.75f * f(x/2, y, _);
    upy(x, y, _) = 0.25f * upx(x, (y/2) - 1 + 2*(y % 2), _) + 0.75f * upx(x, y/2, _);

    return upy;

}

int main(int argc, char **argv) {

    /* THE ALGORITHM */

    // Number of pyramid levels
    const int J = 8;

    // number of intensity levels
    Param<int> levels;

    // Parameters controlling the filter
    Param<float> inv_delta, clamp_min;

    ImageParam input(Float(32), 3);
    ImageParam lut(Float(32), 1);

    Expr width       = input.width();
    Expr height      = input.height();

    // loop variables
    Var c, k;


    // Set a boundary condition
    Func I_clamped;
    I_clamped(x, y, c) = input(clamp(x, 0, width-1), clamp(y, 0, height-1), c);

    // Get the luminance channel
    Func gray;
    gray(x, y) = 0.299f * I_clamped(x, y, 0) + 0.587f * I_clamped(x, y, 1) + 0.114f * I_clamped(x, y, 2);

    // Make the output Gaussian pyramid.
    Func gPyramid[J];

    // 2 - Do a lookup into a lut with 256 entries per intensity level
    Expr level = k * (1.0f / (levels - 1));
    Func lut_idx;
    Expr fx = cast<float>(x)/256.0f;
    lut_idx(x) = clamp(cast<int>((fx-clamp_min)*inv_delta),0,n_inverse_points-1);

    // Processed input
    float eps = 1e-3;
    Expr sign = (level-gray(x,y))/(abs(level-gray(x,y))+eps);
    gPyramid[0](x, y, k) = level-sign*lut(lut_idx(cast<int>(256*(abs(level-gray(x,y))))));
    for (int j = 1; j < J; j++) {
        gPyramid[j](x, y, k) = downsample(gPyramid[j-1])(x, y, k);
    }

    // Get its laplacian pyramid
    Func lPyramid[J];
    lPyramid[J-1](x, y, k) = gPyramid[J-1](x, y, k);
    for (int j = J-2; j >= 0; j--) {
        lPyramid[j](x, y, k) = gPyramid[j](x, y, k) - upsample(gPyramid[j+1])(x, y, k);
    }

    // 1 - Make the Gaussian pyramid of the input
    Func inGPyramid[J];
    inGPyramid[0](x, y) = gray(x, y);
    for (int j = 1; j < J; j++) {
        inGPyramid[j](x, y) = downsample(inGPyramid[j-1])(x, y);
    }

    // 4 - Make the laplacian pyramid of the output
    Func outLPyramid[J];
    for (int j = 0; j < J; j++) {
        // Split input pyramid value into integer and floating parts
        Expr level = inGPyramid[j](x, y) * cast<float>(levels-1); // range level
        Expr li = clamp(cast<int>(level), 0, levels-2);
        Expr lf = level - cast<float>(li);

        // Linearly interpolate between the nearest processed pyramid levels
        outLPyramid[j](x, y) = (1.0f - lf) * lPyramid[j](x, y, li) + lf * lPyramid[j](x, y, li+1);
    }

    // 5 - Make the Gaussian pyramid of the output
    Func outGPyramid[J];
    outGPyramid[J-1](x, y) = outLPyramid[J-1](x, y);
    for (int j = J-2; j >= 0; j--) {
        outGPyramid[j](x, y) = upsample(outGPyramid[j+1])(x, y) + outLPyramid[j](x, y);
    }

    // Reintroduce color (Connelly: use eps to avoid scaling up noise w/ apollo3.png input)
    Func color;
    color(x, y, c) = outGPyramid[0](x, y) * (I_clamped(x, y, c)+eps) / (gray(x, y)+eps);

    Func output("style_transfer");
    output(x, y, c) = clamp(color(x, y, c), 0.0f, 1.0f);

    /* THE SCHEDULE */

    // Target target = get_target_from_environment();

    // cpu schedule
    Var yi;
    output.parallel(y, 4).vectorize(x, 8);
    gray.compute_root().parallel(y, 4).vectorize(x, 8);
    for (int j = 0; j < 4; j++) {
        if (j > 0) inGPyramid[j].compute_root().parallel(y, 4).vectorize(x, 8);
        if (j > 0) gPyramid[j].compute_root().parallel(y, 4).vectorize(x, 8);
        outGPyramid[j].compute_root().parallel(y, 4).vectorize(x, 8);
    }
    for (int j = 4; j < J; j++) {
        inGPyramid[j].compute_root().parallel(y);
        gPyramid[j].compute_root().parallel(k);
        outGPyramid[j].compute_root().parallel(y);
    }

    std::vector<Argument> args(5);
    args[0] = levels;
    args[1] = inv_delta;
    args[2] = clamp_min;
    args[3] = input;
    args[4] = lut;

    output.compile_to_file("style_transfer", args);

    return 0;
}

