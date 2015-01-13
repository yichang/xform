#include <Halide.h>
using namespace Halide;

int main(int argc, char **argv){

  ImageParam input(Float(32), 3), 
             lp_input(Float(32), 3), 
             ac(Float(32), 2), 
             dc_output(Float(32), 3);

  Param<int> step; 

  // Issue #1: if I hardcoded the param below, the results contain artifacts
  //int step = 4;

  Var x("x"), y("y"), c("c"), yi("yi"), yo("yo");

  Func hp_input("hp_input");
  hp_input(x, y, c) = input(x, y, c) - lp_input(x, y, c);

  Expr ac_width = cast<int>(ac.width()/3);
  Expr ac_height = cast<int>(ac.height()/3);
  Expr on_ac_x = cast<int>(x/step);
  Expr on_ac_y = cast<int>(y/step);

  Func hp_output("hp_output");
  hp_output(x, y, c) = 
    hp_input(x, y, 0) * ac(c * ac_width + on_ac_x, 
                           0 * ac_height + on_ac_y) + 
    hp_input(x, y, 1) * ac(c * ac_width + on_ac_x, 
                           1 * ac_height + on_ac_y) + 
    hp_input(x, y, 2) * ac(c * ac_width + on_ac_x, 
                           2 * ac_height + on_ac_y);
  Func output("output");
  output(x,y,c) = hp_output(x,y,c) + dc_output(x, y, c);

  Func final("final");
  final(x,y,c) = clamp(output(x,y,c), 0.0f, 1.0f);

  /* Scheduling */
  // Issue #2: if I hardcoded step (to make issue #1 happen), I can uncomment the below line to make it disappear. 
  final.split(y, yo, yi, 16).parallel(yo).vectorize(x,8);
  final.compile_to_file("halide_recon", input, lp_input, ac, dc_output, step);

  return 0;
}
