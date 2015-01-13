#include "halide_dequant.h"
#include "static_image.h"
#include "image_io.h"
#include <sys/time.h>


int main(int argc, char** argv){
  
  Image<float> ac = load<float>(argv[1]);
  Image<float> mins(3,3), maxs(3,3), out(ac.width(), ac.height(), ac.channels());

  int nchannels=3;
  timeval t1, t2;
  gettimeofday(&t1, NULL);
  halide_dequant(ac, mins, maxs, 3, ac); 
  gettimeofday(&t2, NULL);

  unsigned int t = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
  printf("%u\n", t);

}
