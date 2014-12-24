#include <Eigen/Dense>
#include "XImage.h"
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
    void resize_1d(const VecType &in, const InterpType interp_type, double inv_scale, VecType* out) const;
    double kernel_linear(double x, double scale) const;

};

} // namespace xform

#endif //SRC_WARP_H
