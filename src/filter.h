#include <Eigen/Dense>
#include "x_image.h"

#ifndef SRC_FILTER_H
#define SRC_FILTER_H

namespace xform{

class Filter{
 public:
  Filter(){};
  void box(const XImage& im_in, int b_width, XImage* im_out) const;
  void box(const ImageType_1& im_in, int b_width, ImageType_1* im_out) const;
 private:
  void recursiveBoxFilter(const ImageType_1& im_in, int b_width, 
                                ImageType_1* im_out) const;
};
} // namespace xform
#endif //SRC_FILTER_H
