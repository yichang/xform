#include <Halide.h>
using namespace Halide;

int main(int argc, char **argv){

  ImageParam  ac(Float(32), 2), 
              mins(Float(32), 2),
              maxs(Float(32), 2);
  Param<int> nchannels;

  Var x("x"), y("y"), ni("ni"), no("no"),
       yi("yi"), yo("yo");

  Func range("range");
  range(ni, no) = maxs(ni, no) - mins(ni, no);

  Func gain("gain");
  gain(x,y) = ac(x, y) * range((x*nchannels)/ac.width(), (y*nchannels)/ac.height());

  Func final("final");
  final(x,y) = gain(x,y) + mins((x*nchannels)/ac.width(), (y*nchannels)/ac.height());

  /* Scheduling */
  //range.store_at(final, yo).compute_at(final, yi);
  // Issue #3: uncomment the below changes the result
  //final.split(y, yo, yi, 8).parallel(yo).vectorize(x,4);
  final.compile_to_file("halide_dequant", ac, mins, maxs, nchannels);

  return 0;
}
