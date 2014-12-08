#include <Eigen/Dense>
#include "util/util.h"

#ifndef SRC_COLOR_TRANSFER_H
#define SRC_COLOR_TRANSFER_H

class ColorTransfer{
 public:
  ColorTransfer(){};
  void reinhard(const ImageType_3& im_in, const ImageType_3& ex, 
                                    ImageType_3* im_out) const;
 private:
};

#endif //SRC_COLOR_TRANSFER_H
