#include <Halide.h>
using namespace Halide;

Func yuv2rgb(Func yuv_){
  Var x("x"), y("y"), c("c");
  Func rgb("rgb");

  Expr r =   yuv_(x, y, 0) +                       + 1.14 * yuv_(x, y, 2); 
  Expr g =   yuv_(x, y, 0) - 0.395 * yuv_(x, y, 1) - 0.581 * yuv_(x, y, 2); 
  Expr b =   yuv_(x, y, 0) + 2.032 * yuv_(x, y, 1); 

  rgb(x,y,c) = select(c == 0, r, c == 1, g,  b);
  return rgb;
}
Func rgb2yuv(Func rgb_){
  Var x("x"), y("y"), c("c");
  Func yuv_("yuv_");

  Expr yy =  cast<float>(0.299) * rgb_(x, y, 0) + 0.587 * rgb_(x, y, 1) + 0.144 * rgb_(x, y, 2); 
  Expr u = cast<float>(-0.147) * rgb_(x, y, 0) - 0.289 * rgb_(x, y, 1) + 0.436 * rgb_(x, y, 2); 
  Expr v =  cast<float>(0.615) * rgb_(x, y, 0) - 0.515 * rgb_(x, y, 1) - 0.100 * rgb_(x, y, 2); 
  yuv_(x,y,c) = select(c == 0, yy, c == 1, u,  v);

  return yuv_;
}

