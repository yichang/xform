#include <math.h>
#include "Curve.h"
#include "util.h"

using namespace xform;

void Curve::sShape(const ImageType_1& im_in, const PixelType sigma, 
    const PixelType g0, const double alpha, ImageType_1* im_out) const{
  
  const int height = im_in.rows();
  const int width  = im_in.cols();
  *im_out = ImageType_1(height, width);

  for(int i = 0; i < height; i++){ 
    for(int j = 0; j < width; j++){
      const PixelType g = im_in(i, j); 
      
      if ((g < (g0 + sigma)) && (g >= g0)) 
        (*im_out)(i, j) = g0 + sigma * pow((g - g0)/sigma, alpha);
      else if ((g > (g0 - sigma)) && (g < g0)) 
        (*im_out)(i, j) = g0 - sigma * pow((g0 - g)/sigma, alpha);
      else
        (*im_out)(i, j) = g;
   }}
}
