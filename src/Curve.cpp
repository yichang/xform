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
      sShape(g, sigma, g0, alpha, &((*im_out)(i,j))); 
    }}
}

void Curve::sShape(const PixelType& in, const PixelType sigma, 
            const PixelType g0, const double alpha, PixelType* out) const{

      if ((in < (g0 + sigma)) && (in >= g0)) 
        *out = g0 + sigma * pow((in - g0)/sigma, alpha);
      else if ((in > (g0 - sigma)) && (in < g0)) 
        *out = g0 - sigma * pow((g0 - in)/sigma, alpha);
      else
        *out = in;
}
