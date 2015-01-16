#include <fstream>
#include "halide_highpass.h"
#include "halide_downsample.h"
#include "static_image.h"
#include "image_io.h"
#include <iostream>
#include <sys/time.h>
#include <math.h>
using namespace std;

int main(int argc, char** argv){
  
  const int n = 5;
  Image<float> input = load<float>(argv[1]);

  // Downsample
  Image<float> ds(input.width()/pow(2,n-1), input.height()/pow(2,n-1),input.channels());
  halide_downsample(input, ds);

  // Highpass
  Image<float> output(input.width(), input.height(), input.channels());
  halide_highpass(input, ds, output);

  save(output, argv[2]);
}
