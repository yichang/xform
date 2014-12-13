#include <Eigen/Dense>
#include "util.h"

#ifndef SRC_COLOR_SPACE_H
#define SRC_COLOR_SPACE_H

class ColorSpace{
 public:
  ColorSpace();
  void rgb2yuv(const ImageType_3& im_in, ImageType_3* im_out) const;
  void yuv2rgb(const ImageType_3& im_in, ImageType_3* im_out) const;
  void rgb2xyz(const ImageType_3& im_in, ImageType_3* im_out) const;
  void xyz2rgb(const ImageType_3& im_in, ImageType_3* im_out) const;
  void rgb2lab(const ImageType_3& im_in, ImageType_3* im_out) const;
  void lab2rgb(const ImageType_3& im_in, ImageType_3* im_out) const;

 private:
  void linearMap(const ImageType_3& im_in, const ColorMatType& linearMat, 
                       ImageType_3* im_out) const; 
  double fMap(PixelType in_val) const; 
  double lMap(PixelType in_val) const; 
  double abMap(PixelType in_val) const; 

  ColorMatType rgb_2_yuv;
  ColorMatType yuv_2_rgb;
  ColorMatType rgb_2_xyz;
  ColorMatType xyz_2_rgb;
  PixelType thres_1; 
  PixelType thres_2; 
};

#endif //SRC_COLOR_SPACE_H
