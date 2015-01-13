#include <Halide.h>
using namespace Halide;

enum InterpolationType {
    BOX, LINEAR, CUBIC, LANCZOS
};

Expr kernel_box(Expr x) {
    Expr xx = abs(x);
    return select(xx <= 0.5f, 1.0f, 0.0f);
}

Expr kernel_linear(Expr x) {
    Expr xx = abs(x);
    return select(xx < 1.0f, 1.0f - xx, 0.0f);
}

Expr kernel_cubic(Expr x) {
    Expr xx = abs(x);
    Expr xx2 = xx * xx;
    Expr xx3 = xx2 * xx;
    float a = -0.5f;

    return select(xx < 1.0f, (a + 2.0f) * xx3 - (a + 3.0f) * xx2 + 1,
                  select (xx < 2.0f, a * xx3 - 5 * a * xx2 + 8 * a * xx - 4.0f * a,
                          0.0f));
}

Expr sinc(Expr x) {
    return sin(float(M_PI) * x) / x;
}

Expr kernel_lanczos(Expr x) {
    Expr value = sinc(x) * sinc(x/3);
    value = select(x == 0.0f, 1.0f, value); // Take care of singularity at zero
    value = select(x > 3 || x < -3, 0.0f, value); // Clamp to zero out of bounds
    return value;
}

struct KernelInfo {
    const char *name;
    float size;
    Expr (*kernel)(Expr);
};

static KernelInfo kernelInfo[] = {
    { "box", 0.5f, kernel_box },
    { "linear", 1.0f, kernel_linear },
    { "cubic", 2.0f, kernel_cubic },
    { "lanczos", 3.0f, kernel_lanczos }
};


int main(int argc, char **argv){

  ImageParam input(Float(32), 3);
  Param<float>  output_height, output_width; 
  //InterpolationType interpolationType = LINEAR;
  InterpolationType interpolationType = CUBIC;
  Var x("x"), yo("yo"), y("y"), c("c"), k("k"), yi("yi"), xi("xi");
  
  /* Algorithm */
  Func clamped("clamped");
  clamped(x, y, c) = input(clamp(x, 0, input.width()-1), clamp(y, 0, input.height()-1), c);

  Expr scaleFactor_x = output_width/input.width();
  Expr scaleFactor_y = output_height/input.height();

  Expr kernelScaling_x = min(scaleFactor_x, 1.0f);
  Expr kernelScaling_y = min(scaleFactor_y, 1.0f);

  Expr kernelSize_x = kernelInfo[interpolationType].size / kernelScaling_x;
  Expr kernelSize_y = kernelInfo[interpolationType].size / kernelScaling_y;

  /* Source coordinate */
  Expr sourcex = (x + 0.5f) / scaleFactor_x;
  Expr sourcey = (y + 0.5f) / scaleFactor_y;

  /* kernel coordinate */
  Func kernelx("kernelx"), kernely("kernely");
  Expr beginx = cast<int>(sourcex - kernelSize_x + 0.5f);
  Expr beginy = cast<int>(sourcey - kernelSize_y + 0.5f);

  RDom domx(0, cast<int>(2.0f * kernelSize_x) + 1, "domx");
  RDom domy(0, cast<int>(2.0f * kernelSize_y) + 1, "domy");

    {
        const KernelInfo &info = kernelInfo[interpolationType];
        Func kx, ky;
        kx(x, k) = info.kernel((k + beginx - sourcex) * kernelScaling_x);
        ky(y, k) = info.kernel((k + beginy - sourcey) * kernelScaling_y);
        kernelx(x, k) = kx(x, k) / sum(kx(x, domx));
        kernely(y, k) = ky(y, k) / sum(ky(y, domy));
    }

    Func resized_x("resized_x");
    Func resized_y("resized_y");

    resized_x(x, y, c) = sum(kernelx(x, domx) * cast<float>(clamped(domx + beginx, y, c)));
    resized_y(x, y, c) = sum(kernely(y, domy) * resized_x(x, domy + beginy, c));

    Func final("final");
    final(x, y, c) = clamp(resized_y(x, y, c), 0.0f, 1.0f);

  /* Scheduling */
  kernelx.compute_root();
  kernely.compute_at(final, yo);
  final.split(y, yo, yi, 32).parallel(yo).vectorize(x, 8);
  resized_x.store_at(final, yo).compute_at(final, yi).vectorize(x, 8); 

  final.compile_to_file("halide_resize",input,  output_height, output_width);

  return 0;
}
