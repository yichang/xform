#include <iostream>
#include <algorithm>
#include "color_space.h"
#include "util/util.h"

ColorSpace::ColorSpace(){
  rgb_2_yuv <<  0.299,    0.587, 0.144, 
             -0.14713, -0.28886, 0.436,   
                0.615, -0.51499, -0.10001; 

  yuv_2_rgb << 1.0, 0.0, 1.13983,  
              1.0, -0.39465, -0.58060,
              1.0, 2.03211, 0.0; 

  rgb_2_xyz << 0.412453, 0.357580, 0.180423,
               0.212671, 0.715160, 0.072169,
               0.019334, 0.119193, 0.950227;

  thres =  0.008856;
}
void ColorSpace::rgb2yuv(const ImageType_3& im_in, ImageType_3* im_out) const{
  linearMap(im_in, rgb_2_yuv, im_out);
}

void ColorSpace::yuv2rgb(const ImageType_3& im_in, ImageType_3* im_out) const{
  linearMap(im_in, yuv_2_rgb, im_out);
}

void ColorSpace::rgb2xyz(const ImageType_3& im_in, ImageType_3* im_out) const{
  linearMap(im_in, rgb_2_xyz , im_out);
}

void ColorSpace::rgb2lab(const ImageType_3& im_in, ImageType_3* im_out) const{

  ImageType_3 xyz; 
  rgb2xyz(im_in, &xyz);

  xyz(0) = xyz(0)/0.950456;
  xyz(2) = xyz(2)/1.088754;


  (*im_out)(0) = ImageType_1(im_in(0).rows(), im_in(0).cols());

  /* L channel */
  for(int i=0; i < im_in.rows(); i++)
    for(int j=0; j < im_in.cols(); j++)
      (*im_out)(0)(i,j) = lMap( xyz(1)(i, j));

  /* ab channels */
  for(int c=0; c<im_in.rows(); c++){
    for(int i=0; i < im_in.rows(); i++){ 
      for(int j=0; j < im_in.cols(); j++){
        xyz(c)(i,j) = fMap(xyz(c)(i,j));
      }}}

  (*im_out)(1) = 500.0 * (xyz(0) - xyz(1));
  (*im_out)(2) = 200.0 * (xyz(1) - xyz(2));

}

double ColorSpace::fMap(PixelType in_val) const{
  if (in_val > thres) 
    return std::cbrt(in_val);
  else
    return (7.787f * in_val + 16.0f/116.0f);
}

double ColorSpace::lMap(PixelType in_val) const{
  if (in_val > thres) 
    return (116.0 * std::cbrt(in_val) - 16.0);
  else
    return (903.3 * in_val);
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


