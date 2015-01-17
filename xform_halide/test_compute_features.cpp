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

  halide_compute_features(input, output);
}
