// Compile: g++ test_ColorSpace.cpp ../util/util.cpp  ../src/color_space.cpp -I./../libs/png++-0.2.5/ -I./../libs/eigen-eigen-1306d75b4a21 -I./../ -lpng15; ./a.out
#include "util/util.h"
#include "src/color_space.h"

int main(){
  
  string filename = "../images/yichang.png";
  ImageType_3 my_image;  
  imread(filename, &my_image);

  ColorSpace color_space; 

  ImageType_3 in_yuv, in_rgb; 
  color_space.rgb2yuv(my_image, &in_yuv); 
  in_yuv(0) *= 0.7; 
  color_space.yuv2rgb(in_yuv, &in_rgb); 

  imwrite(in_yuv, "yuv.png");
  imwrite(in_rgb, "rgb.png");

  return 0;
}
