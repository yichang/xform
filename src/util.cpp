#include <algorithm>
#include "util.h"
#include "png.hpp"

namespace xform{
/* Implment through png++ */
bool imread(const string& filename, ImageType_3* image){

  //TODO(yichang): replace reference with pointer
  // TODO(yichang): error handling when sizes are not equal.
  png::image<png::rgb_pixel> buf_image(filename);
  int height = buf_image.get_height();
  int width = buf_image.get_width();

  for(int i=0; i<image->rows(); i++){
    (*image)(i) = ImageType_1(height, width);
  }

  png::rgb_pixel pix; 
  for(int i=0; i<height; i++){
    for(int j=0; j<width; j++){
      pix = buf_image.get_pixel(j, i);
      (*image)(0)(i,j) = static_cast<PixelType>(pix.red)/255.0;
      (*image)(1)(i,j) = static_cast<PixelType>(pix.green)/255.0;
      (*image)(2)(i,j) = static_cast<PixelType>(pix.blue)/255.0;
    }
  }
  return true;
}

bool imwrite(const ImageType_3& image, const string& filename){
  int height = image(0).rows();
  int width = image(0).cols();
  int r, g, b;
  png::image<png::rgb_pixel> buf_image(width, height);
  for (size_t y = 0; y < height; ++y){
    for (size_t x = 0; x < width; ++x){
      r = 255.0f * std::max(std::min(image(0)(y,x), UPPER_BOUND), LOWER_BOUND);
      g = 255.0f * std::max(std::min(image(1)(y,x), UPPER_BOUND), LOWER_BOUND);
      b = 255.0f * std::max(std::min(image(2)(y,x), UPPER_BOUND), LOWER_BOUND);
      buf_image[y][x] = png::rgb_pixel(r, g, b);
         // non-checking equivalent of image.set_pixel(x, y, ...);
    }
  }
 buf_image.write(filename);
 return true;
}
}// namespace xform
