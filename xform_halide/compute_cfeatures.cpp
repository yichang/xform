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

  ImageParam input(Float(32), 3);

  Func clamped("clamped");
  clamped(x, y, c) = input(clamp(x, 0, input.width()-1), clamp(y, 0, input.height()-1), clamp(c, 0 , input.channels()-1));

  Func my_yuv("yuv");
  my_yuv(x, y, c) = rgb2yuv(clamped)(x, y, c);

  // High-pass features
  Func ds("ds");
  Func ds_x("ds_x");
  ds_x(x, y, c) = resize_x(my_yuv, 1.0/scaleFactor)(x, y, c);
  ds(x, y, c) = resize_y(ds_x, 1.0/scaleFactor)(x, y, c);

  Func us_ds("us_ds");
  Func us_ds_x("us_ds_x");
  us_ds_x(x, y, c) = resize_x(ds, scaleFactor)(x, y, c);
  us_ds(x, y, c) = resize_y(us_ds_x, scaleFactor)(x, y, c);

  Func hp("hp");
  hp(x, y, c) = my_yuv(x, y, c) - us_ds(x, y, c);

  Func final("final");
  final(x, y, c) = select(c==0, hp(x, y, 0),
                          c==1, hp(x, y, 1),
                          c==2, hp(x, y, 2),
                               1);


  /* Scheduling */
  final.split(y, yo, yi, 32).parallel(yo).vectorize(x, 8);
  hp.compute_root().split(y, yo, yi, 32).parallel(yo).vectorize(x, 8);
  us_ds_x.store_at(hp, yo).compute_at(hp, yi).vectorize(x, 8);
  ds.compute_root().split(y, yo, yi, 8).parallel(yo); 
  ds_x.store_at(ds, yo).compute_at(ds, yi).vectorize(x, 8);
  
  std::vector<Argument> args(1);
  args[0] = input;
  final.compile_to_file("halide_compute_cfeatures", args);

  return 0;
}
