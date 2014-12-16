#include <Eigen/Dense>
#include "x_image.h"
#include "util.h"

#ifndef SRC_WARP_H
#define SRC_WARP_H

namespace xform{

class Warp{
 public:

  enum InterpType{ // for now only BILINEAR and NN 
    BILINEAR,
    NEAREST,
    BICUBIC,
  };

  Warp(){};
  void imresize(const XImage& im_in, const float scale, XImage* im_out) const; 
  void imresize(const XImage& im_in, const float scale, 
               const InterpType interp_type, XImage* im_out) const; 
  void imresize(const XImage& im_in, const int new_h, 
        const int new_w, const InterpType interp_type, XImage* im_out) const; 
  void imresize(const ImageType_1& im_in, const int new_h, 
    const int new_w, const InterpType interp_type, ImageType_1* im_out) const; 
 private:

};

} // namespace xform

#endif //SRC_WARP_H
