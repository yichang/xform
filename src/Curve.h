#include <Eigen/Dense>
#include "XImage.h"

#ifndef SRC_CURVE_H
#define SRC_CURVE_H

namespace xform{

/* Global curve for tone adjusemtn*/

class Curve{
 public:
  Curve(){};
  void sShape(const ImageType_1& im_in, const PixelValue sigma, 
            const PixelValue g, const double alpha, ImageType_1* im_out) const;
  void expShape(const ImageType_1& im_in) const; //More parameters...


};

} // namespace xform

#endif // SRC_CURVE_H
