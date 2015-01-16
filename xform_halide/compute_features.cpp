#include <Halide.h>
using namespace Halide;

Var x, y, yi, yo, c;
// Downsample with a 1 3 3 1 filter
Func downsample(Func f) {
    Func downx, downy;

    downx(x, y, _) = (f(2*x-1, y, _) + 3.0f * (f(2*x, y, _) + f(2*x+1, y, _)) + f(2*x+2, y, _)) / 8.0f;
    downy(x, y, _) = (downx(x, 2*y-1, _) + 3.0f * (downx(x, 2*y, _) + downx(x, 2*y+1, _)) + downx(x, 2*y+2, _)) / 8.0f;

    return downy;
}
// Upsample using bilinear interpolation
Func upsample(Func f) {
    Func upx, upy;

    upx(x, y, _) = 0.25f * f((x/2) - 1 + 2*(x % 2), y, _) + 0.75f * f(x/2, y, _);
    upy(x, y, _) = 0.25f * upx(x, (y/2) - 1 + 2*(y % 2), _) + 0.75f * upx(x, y/2, _);

    return upy;
}
Func downsample_n(Func f, const int J){
  Func* gdPyramid = new Func[J];
  gdPyramid[0](x, y, _) = f(x, y, _);
  for (int j = 1; j < J; j++) {
      gdPyramid[j](x, y, _) = downsample(gdPyramid[j-1])(x, y, _);
  }
  for(int i = 0; i < J; i++){
    gdPyramid[i].compute_root();
    gdPyramid[i].split(y, yo, yi, 16).parallel(yo).vectorize(x, 8);
  }
  return gdPyramid[J-1];
}
Func upsample_n(Func f, const int J){
  Func* guPyramid = new Func[J];
  guPyramid[J-1](x, y, _) = f(x, y, _);
  for (int j = J-1; j > 0; j--) {
      guPyramid[j-1](x, y, _) = upsample(guPyramid[j])(x, y, _);
  }
  for(int i = 0; i < J; i++){
    guPyramid[i].compute_root();
    guPyramid[i].split(y, yo, yi, 16).parallel(yo).vectorize(x, 8);
  }
  return guPyramid[0];
}
Func gaussian_stack(Func f, const int j){
  Func ds;
  ds(x, y, _) = downsample_n(f, j)(x, y, _);
  Func us;
  us(x, y, _) = upsample_n(ds, j)(x, y, _);
  return us;
}
Func yuv2rgb(Func yuv){
  Func rgb("rgb");
  rgb(x, y, c) = yuv(x, y, c);
  rgb(x, y, 0) = yuv(x, y, 0)                            + 1.140 * yuv(x, y, 2);
  rgb(x, y, 1) = yuv(x, y, 0) - 0.395 * yuv(x, y, 1) - 0.581 * yuv(x, y, 2); 
  rgb(x, y, 2) = yuv(x, y, 0) + 2.032 * yuv(x, y, 1); 
  return rgb;
}
Func rgb2yuv(Func rgb_){
  Func yuv_("yuv_");
  yuv_(x, y, c) = rgb_(x, y, c);
  yuv_(x, y, 0) =  cast<float>(0.299) * rgb_(x, y, 0) + 0.587 * rgb_(x, y, 1) + 0.144 * rgb_(x, y, 2); 
  yuv_(x, y, 1) = cast<float>(-0.147) * rgb_(x, y, 0) - 0.289 * rgb_(x, y, 1) + 0.436 * rgb_(x, y, 2); 
  yuv_(x, y, 2) =  cast<float>(0.615) * rgb_(x, y, 0) - 0.515 * rgb_(x, y, 1) - 0.100 * rgb_(x, y, 2); 
  return yuv_;
}
int main(int argc, char **argv){

  const int J = 5;
  const int nbins = 4;
  const int step = 16;

  Var x, y, c, yi, yo, xi, xo, ni, no;

  ImageParam input(Float(32), 3);

  Param<int>  level;

  Func clamped("clamped");
  clamped(x, y, c) = input(clamp(x, 0, input.width()-1), clamp(y, 0, input.height()-1), clamp(c, 0 , input.channels()-1));

  Func my_yuv("yuv");
  my_yuv(x, y, c) = rgb2yuv(clamped)(x, y, c);

  // High-pass features
  Func ds("ds");
  ds(x,y,c) = downsample_n(my_yuv, J)(x,y,c);

  Func hp("hp");
  hp(x, y, c) = my_yuv(x, y, c) - upsample_n(ds, J)(x, y, c);

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
  RDom r(0, input.width(), 0, input.height());
  Func maxi("maxi"), mini("mini");
  maxi() = maximum(lumin_hp(r.x, r.y));
  mini() = minimum(lumin_hp(r.x, r.y));
  Func range("range"); 
  range() = maxi() - mini();
  for(int i = 0; i < nbins - 1; i++){
    Func thresh("thresh"); 
    thresh() = static_cast<float>(i+1) * range() / static_cast<float>(nbins) + mini();
    curve_feat[i](x, y) = max(lumin_hp(x, y) - thresh(), 0); 
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
  //final.tile(x, y, xo, xi, yo, yi, 16, 32).parallel(yo).vectorize(xi,8);
  maxi.compute_root();
  mini.compute_root();
  range.compute_root();
  
  std::vector<Argument> args(1);
  args[0] = input;
  final.compile_to_file("halide_compute_features", args);

  return 0;
}
