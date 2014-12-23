#include <Eigen/Dense>
#include "XImage.h"

#ifndef SRC_BILATERAL_FILTER_H
#define SRC_BILATERAL_FILTER_H

namespace xform{

class BilateralFilter{
 public:
  enum ImplType{
    EXACT,
    BILATERAL_GRID,
    BILATERAL_LATTICE,
    ADAPTIVE_MANIFOLD,
    KD_TREE,
    UN_NORMALIZED,
    PATCH,
    PATCH_SHIFT,
  };
  BilateralFilter(){};

};

} // namespace xform

#endif // SRC_BILATERAL_FILTER_H
