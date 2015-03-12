#include <Halide.h>
#include "tools/resize.h"
#include "tools/color_transform.h"
#include "tools/resample.h"

using namespace Halide;
using namespace std;

int main(int argc, char **argv){

Var x("x"), y("y"), xi("xi"), xo("xo"), yi("yi"), yo("yo"), c("c"),
    k("k"), ni("ni"), no("no");

  const int J = atoi(argv[1]); //num_levels
  const int step = atoi(argv[2]); // step size
  const float scaleFactor = float(std::pow(2, J-1));

  ImageParam input(Float(32), 3), 
             lp_input(Float(32), 3);

  Func clamped("clamped");
  clamped(x, y, c) = input(clamp(x, 0, input.width()-1), clamp(y, 0, input.height()-1), c);

  Func my_yuv("yuv");
  my_yuv(x, y, c) = rgb2yuv(clamped)(x, y, c);

  Func clamped_lp("clamped_lp");
  clamped_lp(x, y, c) = lp_input(clamp(x, 0, lp_input.width()-1), clamp(y, 0, lp_input.height()-1), c);

  Func lp_yuv("yuv");
  lp_yuv(x, y, c) = rgb2yuv(clamped_lp)(x, y, c);

  Func us_ds("us_ds");
  Func us_ds_x("us_ds_x");
  us_ds_x(x, y, c) = resize_x(lp_yuv, scaleFactor)(x, y, c);
  us_ds(x, y, c) = resize_y(us_ds_x, scaleFactor)(x, y, c);

  Func hp("hp");
  hp(x, y, c) = my_yuv(x, y, c) - us_ds(x, y, c);

  /* Scheduling */
  hp.split(y, yo, yi, 16).parallel(yo).vectorize(x, 8);
  us_ds_x.store_at(hp, yo).compute_at(hp, yi).vectorize(x, 8);
  
  std::vector<Argument> args(2);
  args[0] = input;
  args[1] = lp_input;
  hp.compile_to_file("halide_highpass", args);

  return 0;
}
