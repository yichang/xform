#include <Halide.h>
#include "tools/resample.h"
#include "tools/color_transform.h"
#include "tools/resize.h"

using namespace Halide;

int main(int argc, char **argv){

Var x("x"), y("y"), xi("xi"), xo("xo"), yi("yi"), yo("yo"), c("c"),
    k("k"), ni("ni"), no("no");

  const int J = std::atoi(argv[1]); //num_levels
  const int step = std::atoi(argv[2]); // step size
  const float scaleFactor = float(std::pow(2, J-1));

  ImageParam input(Float(32), 3);

  Func clamped("clamped");
  clamped(x, y, c) = input(clamp(x, 0, input.width()-1), clamp(y, 0, input.height()-1), c);

  Func my_yuv("yuv");
  my_yuv(x, y, c) = rgb2yuv(clamped)(x, y, c);

  Func ds("ds");
  Func ds_x("ds_x");
  ds_x(x, y, c) = resize_x(my_yuv, 1.0/scaleFactor)(x, y, c);
  ds(x, y, c) = resize_y(ds_x, 1.0/scaleFactor)(x, y, c);

   // YUV2RGB
  Func rgb_out("rgb_out");
  rgb_out(x, y, c) = yuv2rgb(ds)(x, y, c);

  Func final("final");
  final(x, y, c) = clamp(rgb_out(x, y, c), 0.0f, 1.0f);

  /* Scheduling */
  final.split(y, yo, yi, 16).parallel(yo).vectorize(x, 8); 
  ds.compute_root();
  ds.split(y, yo, yi, 16).parallel(yo).vectorize(x, 8);
  ds_x.store_at(ds, yo).compute_at(ds, yi).vectorize(x, 8);
  
  std::vector<Argument> args(1);
  args[0] = input;
  final.compile_to_file("halide_downsample", args);

  return 0;
}
