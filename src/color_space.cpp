#include <iostream>
#include "color_space.h"
#include "util/util.h"

ColorSpace::ColorSpace(){
  rgb_2_yuv <<  0.299,    0.587, 0.144, 
             -0.14713, -0.28886, 0.436,   
                0.615, -0.51499, -0.10001; 

  yuv_2_rgb << 1.0, 0.0, 1.13983,  
              1.0, -0.39465, -0.58060,
              1.0, 2.03211, 0.0; 
}
void ColorSpace::rgb2yuv(const ImageType_3& im_in, ImageType_3* im_out) const{
  linearMap(im_in, rgb_2_yuv, im_out);
}

void ColorSpace::yuv2rgb(const ImageType_3& im_in, ImageType_3* im_out) const{
  linearMap(im_in, yuv_2_rgb, im_out);
}

void ColorSpace::linearMap(const ImageType_3& im_in, const ColorMatType& colorMat, ImageType_3* im_out) const{

  int height = im_in(0).rows();
  int width = im_in(0).cols(); 
  ImageType_1 buf(height, width); 
  for(int i=0; i<3; i++){
    buf.setZero(height, width);
    for(int j=0; j<3; j++){
      buf += colorMat(i,j) * im_in(j); 
    }
    (*im_out)(i) = buf;
  }
}


