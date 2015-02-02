//#include "halide_blur.h"
#include "halide_resize.h"
#include "static_image.h"
#include "image_io.h"
#include <sys/time.h>


int main(int argc, char** argv){
  
  Image<float> input = load<float>(argv[1]);
  int height = 108, width = 192 ; 
  Image<float> ds(width, height, 3),
      output(input.width(), input.height(), 3);

  timeval t1, t2;
  gettimeofday(&t1, NULL);
  halide_resize(input, ds.height(), ds.width(), ds);
  halide_resize(ds, output.height(), output.width(), output);
  gettimeofday(&t2, NULL);

  unsigned int t = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
  printf("%u\n", t);
  save(output, argv[2]);

}
