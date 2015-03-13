#include "style_transfer_wrapper.h"

int style_transfer_wrapper(Image<float> &input, Image<float> &model,int levels, Image<float> &output) {
    int n_iterations = 4;
    Image<float> Mnorm(model.width(), model.height(), 1);
    gradient_norm(model, Mnorm);
    Histogram Mhist(Mnorm.data(), Mnorm.height()*Mnorm.width());
    Mhist.compute_inverse();

    Histogram Mgray_hist(model.data(), Mnorm.height()*Mnorm.width());

    for (int i = 0; i < n_iterations; ++i) {
        Image<float> Inorm(input.width(), input.height(), 1);
        gradient_norm(input, Inorm);
        Histogram Ihist(Inorm.data(), Inorm.height()*Inorm.width());

        vector<float> lut;
        float inv_delta, clamp_min;
        Histogram::build_transfer_lut(Ihist, Mhist, lut,inv_delta,clamp_min);
        Image<float> histlut(n_inverse_points);
        std::memcpy(histlut.data(),lut.data(),n_inverse_points*sizeof(float));

        style_transfer(levels, inv_delta, clamp_min, input, histlut, output);
        input = output;
        if(i>0) {
            Histogram Igray_hist(input.data(), input.height()*input.width());
        }
    }
    return 0;
}
