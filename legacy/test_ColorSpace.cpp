// Compile: g++ test_ColorSpace.cpp ../util/util.cpp  ../src/color_space.cpp -I./../libs/png++-0.2.5/ -I./../libs/eigen-eigen-1306d75b4a21 -I./../ -lpng15; ./a.out
#include "util/util.h"
#include "src/color_space.h"

int main(){
  
  string filename = "../images/yichang.png";
  ImageType_3 my_image;  
  imread(filename, &my_image);

  ColorSpace color_space; 

  /* test yuv space */
  ImageType_3 rgb_2_yuv, yuv_2_rgb;  
  color_space.rgb2yuv(my_image, &rgb_2_yuv); 
  //in_yuv(0) *= 0.7; 
  color_space.yuv2rgb(rgb_2_yuv, &yuv_2_rgb); 
  imwrite(rgb_2_yuv, "rgb_2_yuv.png");
  imwrite(yuv_2_rgb, "yuv_2_rgb.png");

  /* test lab space */
  ImageType_3 rgb_2_lab, lab_2_rgb; 
  color_space.rgb2lab(my_image, &rgb_2_lab); 
  imwrite(rgb_2_lab, "rgb_2_lab.png");
  rgb_2_lab(0) = rgb_2_lab(0) * 0.8; 
  color_space.lab2rgb(rgb_2_lab, &lab_2_rgb); 
  imwrite(lab_2_rgb, "lab_2_rgb.png");

  return 0;
}
