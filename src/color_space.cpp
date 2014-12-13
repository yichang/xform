#include <iostream>
#include <algorithm>
#include "color_space.h"
#include "util.h"

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

  xyz_2_rgb <<  3.240479, -1.537150, -0.498535,
               -0.969256,  1.875992,  0.041556,
                0.055648, -0.204043,  1.057311;

  thres_1 =  0.008856;
  thres_2 =  0.206893;
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

void ColorSpace::xyz2rgb(const ImageType_3& im_in, ImageType_3* im_out) const{
  linearMap(im_in, xyz_2_rgb , im_out);
}

void ColorSpace::rgb2lab(const ImageType_3& im_in, ImageType_3* im_out) const{

  ImageType_3 xyz; 
  rgb2xyz(im_in, &xyz);

  xyz(0) = xyz(0)/0.950456;
  xyz(2) = xyz(2)/1.088754;

  /* L channel */
  (*im_out)(0) = ImageType_1(im_in(0).rows(), im_in(0).cols());
  for(int i=0; i < im_in(0).rows(); i++){
    for(int j=0; j < im_in(0).cols(); j++){
      (*im_out)(0)(i,j) = lMap(xyz(1)(i, j));
    }}

  /* ab channels */
  for(int c=0; c<im_in.rows(); c++){
    for(int i=0; i < im_in(0).rows(); i++){ 
      for(int j=0; j < im_in(0).cols(); j++){
        xyz(c)(i,j) = fMap(xyz(c)(i,j));
      }}}

  (*im_out)(1) = 500.0 * (xyz(0) - xyz(1));
  (*im_out)(2) = 200.0 * (xyz(1) - xyz(2));
}


double ColorSpace::fMap(PixelType in_val) const{
  if (in_val > thres_1) 
    return std::cbrt(in_val);
  else
    return (7.787 * in_val + 16.0/116.0);
}

double ColorSpace::lMap(PixelType in_val) const{
  if (in_val > thres_1) 
    return (116.0 * std::cbrt(in_val) - 16.0);
  else
    return (903.3 * in_val);
}

void ColorSpace::lab2rgb(const ImageType_3& im_in, ImageType_3* im_out) const{

  ImageType_3 xyz; 
  ImageType_1 fY; 

  fY  = (im_in(0).array() + 16.0)/116.0;
  fY  = fY.array().pow(3).matrix();

  /* Compute Y */
  xyz(1) = fY; 
  for(int i=0; i < im_in(0).rows(); i++){
    for(int j=0; j < im_in(0).cols(); j++){
      if (fY(i,j) < thres_1)
        xyz(1)(i,j) = im_in(2)(i,j)/903.3; 
    }}

  /* Alter fY */
  for(int i=0; i < im_in(0).rows(); i++)
    for(int j=0; j < im_in(0).cols(); j++)
      fY(i, j) = fMap(fY(i, j));

  /* X and Z */
  xyz(0) = im_in(1)/500.0 + fY;  
  xyz(2) = fY - im_in(2)/200.0; 
  for(int i=0; i < im_in(0).rows(); i++){
    for(int j=0; j < im_in(0).cols(); j++){
      xyz(0)(i,j) = abMap(xyz(0)(i,j));
      xyz(2)(i,j) = abMap(xyz(2)(i,j));
    }}
  xyz(0) = xyz(0) * 0.950456;
  xyz(2) = xyz(2) * 1.088754;

  xyz2rgb(xyz, im_out);
  
  /* Truncation */
  for(int k=0; k<im_in.rows(); k++)
    for(int i=0; i <im_in(0).rows(); i++)
      for(int j=0; j<im_in(0).cols(); j++)
        (*im_out)(k)(i,j) = std::max(std::min((*im_out)(k)(i,j), 1.0), 0.0);
}

double ColorSpace::abMap(PixelType in_val) const{
  if (in_val > thres_2) 
    return std::pow(in_val,3);
  else
    return (in_val - 16.0/116.0) / 7.787; 
}

void ColorSpace::linearMap(const ImageType_3& im_in, 
  const ColorMatType& colorMat, ImageType_3* im_out) const{

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


