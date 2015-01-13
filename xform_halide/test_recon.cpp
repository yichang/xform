#include "halide_recon.h"
#include "halide_resize.h"
#include "static_image.h"
#include "image_io.h"
#include <sys/time.h>


int main(int argc, char** argv){
  
  Image<float> input = load<float>(argv[1]);
  Image<float> ac = load<float>(argv[2]);
  Image<float> dc = load<float>(argv[3]);
  Image<float> output(input.width(), input.height(),3);

  // Low pass
  int height = 81, width = 64 ; 
  Image<float> ds(width, height, 3), 
               lp_input(input.width(), input.height(),3),
               dc_output(input.width(), input.height(), 3);

  halide_resize(input, ds.height(), ds.width(), ds);
  halide_resize(ds, lp_input.height(), lp_input.width(), lp_input);
  halide_resize(dc, output.height(), output.width(), dc_output);

  timeval t1, t2;
  gettimeofday(&t1, NULL);
  int step=4;
  halide_recon(input, lp_input, ac, dc_output, step, output); 
  //halide_recon(input, ac, output); 
  gettimeofday(&t2, NULL);

  unsigned int t = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
  printf("%u\n", t);
  save(output, argv[4]);

}
