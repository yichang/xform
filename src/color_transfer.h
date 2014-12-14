#include <Eigen/Dense>
#include "util/util.h"

#ifndef SRC_COLOR_TRANSFER_H
#define SRC_COLOR_TRANSFER_H

namespace xform{

class ColorTransfer{
 public:
  ColorTransfer(){};
  void reinhard(const XImage& im_in, const XImage& ex, 
                                    XImage* im_out) const;
 private:
};

} // namespace xform

#endif //SRC_COLOR_TRANSFER_H
