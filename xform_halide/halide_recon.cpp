#include <Halide.h>
using namespace Halide;

int main(int argc, char **argv){

  ImageParam input(UInt(16), 3);
  Var x("x"), y("y"), xi("xi"), yi("yi"),  z("z"), yo;
  
  /* Algorithm */
  Expr clamped_x = clamp(x, 0, input.width()-1);
  Expr clamped_y = clamp(y, 0, input.height()-1);
  
  Func floating("floating");
  floating(x,y,z) =cast<float>(input(x,y,z))/65535.0f;

  Func clamped("clamped"); 
  clamped(x,y,z) = floating(clamped_x, clamped_y, z);

  Func blur_x("blur_x");
  blur_x(x, y, z) = (clamped(x,y,z) + clamped(x+1, y,z) +clamped(x+2, y,z))/3;

  Func blur_y("blur_y");
  blur_y(x, y, z) = (blur_x(x,y,z) + blur_x(x, y+1,z) + blur_x(x, y+2,z))/3;

  Func blur_int("blur_int");
  blur_int(x,y,z) = cast<uint16_t>(blur_y(x,y,z)*65535.0f);

  /* Scheduling */
  blur_int.split(y, y, yi, 8).parallel(y).vectorize(x, 8);
  //blur_y.store_at(blur_int, y).compute_at(blur_int, yi).vectorize(x, 8);
  blur_x.store_at(blur_int, y).compute_at(blur_int, yi).vectorize(x, 8);

  blur_int.compile_to_file("halide_blur", input);

  return 0;
}
