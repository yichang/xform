#include <Eigen/Dense>
#include "XImage.h"

#ifndef SRC_FILTER_H
#define SRC_FILTER_H

namespace xform{


class Filter{
 public:

  enum  BoundaryType{
        SYMMETRIC, 
        REPLICATE, 
        CIRCULAR, 
        ZERO_PAD}; 

  Filter(){};
  void box(const XImage& im_in, const int b_width, XImage* im_out) const;
  void box_iteration(const XImage& im_in, const int b_width, 
                     const int n_iter, XImage* im_out) const;
  void box(const XImage& im_in, const int b_width, 
           const BoundaryType boundary_type, XImage* im_out) const;
  void box(const ImageType_1& im_in, const int b_width, 
    const BoundaryType boundary_type, ImageType_1* im_out) const;
 private:
  void recursiveBoxFilterZeroPad(const ImageType_1& im_in, const int b_width, 
                                ImageType_1* im_out) const;
  void recursiveBoxFilterNoPad(const ImageType_1& im_in, const int b_width, 
      const BoundaryType boundary_type, ImageType_1* im_out) const;

  /* Helpers */
  int reflect(const int val, const int width, 
              const BoundaryType boundary_type) const;
};
} // namespace xform
#endif //SRC_FILTER_H
