#include <Halide.h>
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
    const int hist_bins = 200;
    const int inverse_oversampling = 10;
    const int n_inverse_points = hist_bins*inverse_oversampling;
    const int inverse_delta = 1.0/(n_inverse_points-1);

    // number of intensity levels
    Param<int> levels;
    // Parameters controlling the filter
    Param<float> alpha, beta;

    ImageParam input(Float(32), 3);
    ImageParam model(Float(32), 3);
    Image<float> i_buffer;
    input.set(i_buffer);
    Image<float> m_buffer;
    model.set(m_buffer);

    Expr width = input.width();
    Expr height = input.height();
    Expr M_width = input.width();
    Expr M_height = input.height();
    Expr n_samples = width*height;
    Expr n_M_samples = M_width*M_height;

    // loop variables
    Var c, k;

    // Make the remapping function as a lookup table.
    // Func remap;
    // Expr fx = cast<float>(x) / 256.0f;
    // remap(x) = alpha*fx*exp(-fx*fx/2.0f);

    // Set a boundary condition
    Func I_clamped;
    I_clamped(x, y, c) = input(clamp(x, 0, width-1), clamp(y, 0, height-1), c);
    Func M_clamped;
    M_clamped(x, y, c) = model(clamp(x, 0, width-1), clamp(y, 0, height-1), c);

    // Get the luminance channel
    Func I_gray;
    I_gray(x, y) = 0.299f * I_clamped(x, y, 0) + 0.587f * I_clamped(x, y, 1) + 0.114f * I_clamped(x, y, 2);
    Func M_gray;
    M_gray(x, y) = 0.299f * M_clamped(x, y, 0) + 0.587f * M_clamped(x, y, 1) + 0.114f * M_clamped(x, y, 2);

    Func joe;
    joe(x) = input(x,0,0);
    // joe.realize(1);

    // Get gradient norm
    Func I_gray_dx,I_gray_dy,I_grad_norm;
    I_gray_dx(x,y)     = I_gray(x,y)-I_gray(x-1,y);
    I_gray_dy(x,y)     = I_gray(x,y)-I_gray(x,y-1);
    I_grad_norm(x,y)   = sqrt(I_gray_dx(x,y)*I_gray_dx(x,y)+I_gray_dy(x,y)*I_gray_dy(x,y));
    Func M_gray_dx,M_gray_dy,M_grad_norm;
    M_gray_dx(x,y)     = M_gray(x,y)-M_gray(x-1,y);
    M_gray_dy(x,y)     = M_gray(x,y)-M_gray(x,y-1);
    M_grad_norm(x,y)   = sqrt(M_gray_dx(x,y)*M_gray_dx(x,y)+M_gray_dy(x,y)*M_gray_dy(x,y));
    Func I_norm_max, I_norm_min;
    I_norm_max(x) = 0.0f;
    I_norm_min(x) = 0.0f;
    RDom rr(0,width,0,height);
    I_norm_max(0) = maximum(I_grad_norm(rr.x, rr.y));
    I_norm_min(0) = minimum(I_grad_norm(rr.x, rr.y));
    Func M_norm_max, M_norm_min;
    M_norm_max(x) = 0.0f;
    M_norm_min(x) = 0.0f;
    RDom rr_M(0,width,0,height);
    M_norm_max(0) = maximum(M_grad_norm(rr.x, rr.y));
    M_norm_min(0) = minimum(M_grad_norm(rr.x, rr.y));

    // Compute histogram
    Expr I_bin_delta   = (I_norm_max(0)-I_norm_min(0)) / (hist_bins-1);
    Expr I_first_value = I_norm_min(0) - 0.5f*I_bin_delta;
    Expr M_bin_delta   = (M_norm_max(0)-M_norm_min(0)) / (hist_bins-1);
    Expr M_first_value = M_norm_min(0) - 0.5f*M_bin_delta;
    
    Func I_grad_histogram("grad_histogram");
    I_grad_histogram(x) = 0;
    RDom r(0,input.width(),0,input.height());
    I_grad_histogram(clamp(cast<int>( (I_grad_norm(r.x,r.y) - I_first_value)/I_bin_delta ), 0, hist_bins)) += 1;
    Func M_grad_histogram("grad_histogram");
    M_grad_histogram(x) = 0;
    M_grad_histogram(clamp(cast<int>( (M_grad_norm(r.x,r.y) - M_first_value)/M_bin_delta ), 0, hist_bins)) += 1;

    Func I_grad_cdf("Igrad_cdf");
    Func M_grad_cdf("Mgrad_cdf");
    I_grad_cdf(x) = 0.0f;
    M_grad_cdf(x) = 0.0f;
    for (int i = 1; i < hist_bins; ++i) {
        I_grad_cdf(i) = I_grad_cdf(i-1) + I_grad_histogram(i);
        M_grad_cdf(i) = M_grad_cdf(i-1) + M_grad_histogram(i);
    }
    I_grad_cdf(x) /= n_samples;
    M_grad_cdf(x) /= n_M_samples;

    // Inverse M's histogram
    Func M_inv_cdf;
    M_inv_cdf(x) = 0;

    int bottom_j = 0;
    for(int i=1;i<n_inverse_points;i++){

        const float y = i * inverse_delta;
        Func bottom_y;
        bottom_y(x) = M_grad_cdf(cast<int>(M_first_value + x*M_bin_delta));
    }


    // Schedule
    I_norm_max.compute_root();
    I_norm_min.compute_root();
    I_grad_histogram.compute_root();
    I_grad_cdf.compute_root();
    M_norm_max.compute_root();
    M_norm_min.compute_root();
    M_grad_histogram.compute_root();
    M_grad_cdf.compute_root();

    Func transfer_LUT;

    // // Make the output Gaussian pyramid.
    // Func gPyramid[J];
    //
    // // 2 - Do a lookup into a lut with 256 entries per intensity level
    // Expr level = k * (1.0f / (levels - 1));
    // Expr idx = gray(x, y)*cast<float>(levels-1)*256.0f;
    // idx = clamp(cast<int>(idx), 0, (levels-1)*256);
    //
    // // Processed input
    // gPyramid[0](x, y, k) = beta*(gray(x, y) - level) + level + remap(idx - 256*k);
    // for (int j = 1; j < J; j++) {
    //     gPyramid[j](x, y, k) = downsample(gPyramid[j-1])(x, y, k);
    // }
    //
    // // Get its laplacian pyramid
    // Func lPyramid[J];
    // lPyramid[J-1](x, y, k) = gPyramid[J-1](x, y, k);
    // for (int j = J-2; j >= 0; j--) {
    //     lPyramid[j](x, y, k) = gPyramid[j](x, y, k) - upsample(gPyramid[j+1])(x, y, k);
    // }
    //
    // // 1 - Make the Gaussian pyramid of the input
    // Func inGPyramid[J];
    // inGPyramid[0](x, y) = gray(x, y);
    // for (int j = 1; j < J; j++) {
    //     inGPyramid[j](x, y) = downsample(inGPyramid[j-1])(x, y);
    // }
    //
    // // 4 - Make the laplacian pyramid of the output
    // Func outLPyramid[J];
    // for (int j = 0; j < J; j++) {
    //     // Split input pyramid value into integer and floating parts
    //     Expr level = inGPyramid[j](x, y) * cast<float>(levels-1);
    //     Expr li = clamp(cast<int>(level), 0, levels-2);
    //     Expr lf = level - cast<float>(li);
    //
    //     // Linearly interpolate between the nearest processed pyramid levels
    //     outLPyramid[j](x, y) = (1.0f - lf) * lPyramid[j](x, y, li) + lf * lPyramid[j](x, y, li+1);
    // }
    //
    // // 5 - Make the Gaussian pyramid of the output
    // Func outGPyramid[J];
    // outGPyramid[J-1](x, y) = outLPyramid[J-1](x, y);
    // for (int j = J-2; j >= 0; j--) {
    //     outGPyramid[j](x, y) = upsample(outGPyramid[j+1])(x, y) + outLPyramid[j](x, y);
    // }
    //
    // // Reintroduce color (Connelly: use eps to avoid scaling up noise w/ apollo3.png input)
    // Func color;
    // float eps = 0.01f;
    // color(x, y, c) = outGPyramid[0](x, y) * (I_clamped(x, y, c)+eps) / (gray(x, y)+eps);

    Func output("style_transfer");
    // output(x, y, c) = clamp(color(x, y, c), 0.0f, 1.0f);
    // output(x, y, c) = grad_norm(x, y);
    output(x, y, c) = cast<float>(I_grad_cdf(x))/n_samples;

    /* THE SCHEDULE */
    // remap.compute_root();

    // Target target = get_target_from_environment();

    // cpu schedule
    // Var yi;
    // output.parallel(y, 4).vectorize(x, 8);
    // gray.compute_root().parallel(y, 4).vectorize(x, 8);
    // for (int j = 0; j < 4; j++) {
    //     if (j > 0) inGPyramid[j].compute_root().parallel(y, 4).vectorize(x, 8);
    //     if (j > 0) gPyramid[j].compute_root().parallel(y, 4).vectorize(x, 8);
    //     outGPyramid[j].compute_root().parallel(y, 4).vectorize(x, 8);
    // }
    // for (int j = 4; j < J; j++) {
    //     inGPyramid[j].compute_root().parallel(y);
    //     gPyramid[j].compute_root().parallel(k);
    //     outGPyramid[j].compute_root().parallel(y);
    // }

    std::vector<Argument> args(5);
    args[0] = levels;
    args[1] = alpha;
    args[2] = beta;
    args[3] = input;
    args[4] = model;
    // brighter.compile_to_file("lesson_10_halide", args);

    output.compile_to_file("style_transfer", args);
    // output.compile_to_file("style_transfer", levels, alpha, beta, input, model, target);

    return 0;
}

