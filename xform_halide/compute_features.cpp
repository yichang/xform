#include <Halide.h>
#include "tools/resize.h"
#include "tools/color_transform.h"
#include "tools/resample.h"

using namespace Halide;

int main(int argc, char **argv){

Var x("x"), y("y"), xi("xi"), xo("xo"), yi("yi"), yo("yo"), c("c"),
    k("k"), ni("ni"), no("no");

  const int J = 5;
  const int nbins = 4;
  const int step = 16;
  const float scaleFactor = float(std::pow(2, J-1));

  ImageParam input(Float(32), 3);

  Param<int>  level;

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

  /*Func ds("ds");
  ds(x,y,c) = downsample_n(my_yuv, J)(x,y,c);

  Func hp("hp");
  hp(x, y, c) = my_yuv(x, y, c) - upsample_n(ds, J)(x, y, c);*/

  // Laplacian features 
  Func lumin("lumin");
  lumin(x, y) = my_yuv(x, y, 0);

  Func gaussian[J];
  for(int i = 0; i < J; i++)
    gaussian[i](x, y) = gaussian_stack(lumin, i+1)(x, y);

  Func laplacian[J-1];
  for(int i = 0; i < J-1; i++)
    laplacian[i](x, y) = gaussian[i](x, y) - gaussian[i+1](x, y);

  // Lumin curve features
  Func lumin_hp("lumin_hp");
  lumin_hp(x, y) = hp(x, y, 0);
  Func curve_feat[nbins-1];
  RDom r(0, step, 0, step);
  Func maxi("maxi"), mini("mini");
  maxi(x, y) = maximum(lumin_hp(step * x + r.x, step * y + r.y));
  mini(x, y) = minimum(lumin_hp(step * x + r.x, step * y + r.y));
  Func range("range"); 
  range(x, y) = maxi(x, y) - mini(x, y);
  for(int i = 0; i < nbins - 1; i++){
    Func thresh("thresh"); 
    thresh(x, y) = static_cast<float>(i+1) * range(x, y) / static_cast<float>(nbins) + mini(x, y);
    curve_feat[i](x, y) = max(lumin_hp(x, y) - thresh(x/step, y/step), 0); 
  }

  Func final("final");
  final(x, y, c) = clamped(x, y, c);
  final(x, y, 0) = hp(x, y, 0);  
  final(x, y, 1) = hp(x, y, 1);  
  final(x, y, 2) = hp(x, y, 2);  
  final(x, y, 3) = hp(x, y, 0)*0 + 1;  
  final(x, y, 4) = laplacian[0](x, y);  
  final(x, y, 5) = laplacian[1](x, y);  
  final(x, y, 6) = laplacian[2](x, y);  
  final(x, y, 7) = laplacian[3](x, y);  
  final(x, y, 8) = curve_feat[0](x, y);  
  final(x, y, 9) = curve_feat[1](x, y);  
  final(x, y, 10) = curve_feat[2](x, y);  

  /* Scheduling */
  final.split(y, yo, yi, 32).parallel(yo).vectorize(x, 8);
  us_ds.compute_root();
  us_ds.split(y, yo, yi, 32).parallel(yo).vectorize(x, 8);
  us_ds_x.store_at(us_ds, yo).compute_at(us_ds, yi);
  maxi.compute_at(final, y);
  mini.compute_at(final, y);
  ds.compute_root();
  //ds.split(y, yo, yi, 8).parallel(yo); 
  //ds_x.store_at(ds, yo).compute_at(ds, yi).vectorize(x, 8);

  std::vector<Argument> args(1);
  args[0] = input;
  final.compile_to_file("halide_compute_features", args);

  return 0;
}
