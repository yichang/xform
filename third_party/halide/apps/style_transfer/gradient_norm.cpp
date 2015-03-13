#include <Halide.h>
using namespace Halide;

Var x, y, c;

int main(int argc, char **argv) {

    ImageParam input(Float(32), 3);
    Expr width       = input.width();
    Expr height      = input.height();

    // Set a boundary condition
    Func I_clamped;
    I_clamped(x, y, c) = input(clamp(x, 0, width-1), clamp(y, 0, height-1), c);

    // Get the luminance channel
    Func I_gray;
    I_gray(x, y) = 0.299f * I_clamped(x, y, 0) + 0.587f * I_clamped(x, y, 1) + 0.114f * I_clamped(x, y, 2);

    // Get gradient norm
    Func I_gray_dx,I_gray_dy;
    I_gray_dx(x,y)     = I_gray(x,y)-I_gray(x-1,y);
    I_gray_dy(x,y)     = I_gray(x,y)-I_gray(x,y-1);

    Func output("gradient_norm");
    output(x,y,c)   = sqrt(I_gray_dx(x,y)*I_gray_dx(x,y)+I_gray_dy(x,y)*I_gray_dy(x,y));

    std::vector<Argument> args(1);
    args[0] = input;

    output.compile_to_file("gradient_norm", args);

    return 0;
}

