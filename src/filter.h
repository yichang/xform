#include <Eigen/Dense>
#include "util/util.h"

#ifndef SRC_FILTER_H
#define SRC_FILTER_H

class Filter{
 public:
  Filter(){};
  void box(const ImageType_3& im_in, int b_width, ImageType_3* im_out) const;
  void box(const ImageType_1& im_in, int b_width, ImageType_1* im_out) const;
 private:
  void recursiveBoxFilter(const ImageType_1& im_in, int b_width, 
                                ImageType_1* im_out) const;
};
#endif //SRC_FILTER_H
