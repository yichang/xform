#include <Eigen/Dense>
#include "util.h"
#include "XImage.h"

#ifndef SRC_SPARSE_GRID_H
#define SRC_SPARSE_GRID_H

namespace xform{

class SparseGrid{
 public:

  enum Dim{
    X, Y, R, G, B,
  }; 
  SparseGrid(){};
  void construct(const ImageType_1& im_in);  
  void construct(const XImage& im_in);
  void blur(int dim); 

  // Interpolation
  void resample(const ImageType_1& im_in, ImageType_1* im_out) const;
  void resample(const XImage& im_in, ImageType_1* im_out) const;

  // Properties
  int dims() const; // dimension of the grid
 private:
  // SparseMatrix
};

} // namespace xform
#endif // SRC_SPARSE_GRID_H
