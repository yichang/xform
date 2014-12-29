#include <Eigen/Dense>
#include "XImage.h"
#include "util.h"

#ifndef SRC_LOCAL_LAPLACIAN_H
#define SRC_LOCAL_LAPLACIAN_H

namespace xform{

class LocalLaplacian{
 public:
  LocalLaplacian(){};
  void adjustDetails(const ImageType_1& im_in, ImageType_1* im_out) const;

  void adjustDetails(const ImageType_1& im_in,  
                                   const PixelType sigma, // Detail range
                                   const PixelType alpha, // Power curve
                                         ImageType_1* im_out) const;
};

} // namespace xform
#endif // SRC_LOCAL_LAPLACIAN_H
