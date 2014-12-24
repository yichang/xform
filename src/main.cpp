#include <iostream>
#include "TransformModel.h"
#include "XImage.h"
#include "Warp.h"

using namespace xform;

int main(int argc, char *argv[])
{
    // TransformModel model;
    XImage input; 
    input.read("../data/unprocessed.png");
    XImage output; 
    output.read("../data/processed.png");

    // XImage lp_input;
    // Warp warp;
    // int wSize = 8;
    // int height = input.rows();
    // int width = input.cols();
    // warp.imresize(input, height/wSize, width/wSize,Warp::BILINEAR, &lp_input);
    // lp_input.write("../output/lowpass.png");

    TransformModel model;
    model.set_images(input, output);
    model.process();
    XImage reconstructed = model.reconstruct();
    reconstructed.write("../output/reconstructed.png");
    output.write("../output/ref.png");
    return 0;
}
