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

    // Server side
    TransformModel model;
    model.set_images(input, output);
    model.process();

    // Client side
    XImage reconstructed = model.reconstruct();
    reconstructed.write("../output/reconstructed.png");
    output.write("../output/ref.png");
    return 0;
}
