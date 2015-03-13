#include <stdio.h>
#include <vector>
#include "style_transfer.h"
#include "gradient_norm.h"
#include "static_image.h"
#include "image_io.h"
#include <iostream>
#include <sys/time.h>

#include "const.h"
#include "hist.h"

using namespace std;


int main(int argc, char **argv) {
    if (argc < 7) {
        printf("Usage: ./process input.png model.png levels alpha beta output.png\n"
               "e.g.: ./process input.png model.png 8 1 1 output.png\n");
        return 0;
    }

    int n_iterations = 4;

    Image<float> input = load<float>(argv[1]);
    Image<float> model = load<float>(argv[2]);
    int levels         = atoi(argv[3]);
    float alpha        = atof(argv[4]), beta   = atof(argv[5]);
    Image<float> output(input.width(), input.height(), 3);

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

    save(output, argv[6]);

    return 0;
}
