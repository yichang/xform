#include <fstream>
#include "halide_compute_features.h"
#include "static_image.h"
#include "image_io.h"
#include <iostream>
#include <sys/time.h>
using namespace std;
// Compute luminance features

int main(int argc, char** argv){
  
  Image<float> input = load<float>(argv[1]);

  const int num_lumin_feat = 11;
  const int num_chrom_feat = 4;
  Image<float> output(input.width(), 
                      input.height(), 
                      num_lumin_feat);

  timeval t1, t2;
  gettimeofday(&t1, NULL);
  halide_compute_features(input, output);
  gettimeofday(&t2, NULL);
  unsigned int t = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
  printf("%u\n", t);
}
