#include <stdio.h>
#include "style_transfer.h"
#include "static_image.h"
#include "image_io.h"
#include <sys/time.h>

int main(int argc, char **argv) {
    if (argc < 7) {
        printf("Usage: ./process input.png model.png levels alpha beta output.png\n"
               "e.g.: ./process input.png model.png 8 1 1 output.png\n");
        return 0;
    }

    Image<float> input = load<float>(argv[1]);
    Image<float> model = load<float>(argv[2]);
    int levels         = atoi(argv[3]);
    float alpha        = atof(argv[4]), beta   = atof(argv[5]);
    Image<float> output(input.width(), input.height(), 3);

    // // Timing code
    // timeval t1, t2;
    // unsigned int bestT = 0xffffffff;
    // for (int i = 0; i < 1; i++) {
    //   gettimeofday(&t1, NULL);
    //   style_transfer(levels, alpha/(levels-1), beta, input, model, output);
    //   gettimeofday(&t2, NULL);
    //   unsigned int t = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
    //   if (t < bestT) bestT = t;
    // }
    // printf("%u\n", bestT);


    style_transfer(levels, alpha/(levels-1), beta, input, model, output);

    save(output, argv[6]);

    return 0;
}
