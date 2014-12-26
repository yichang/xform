#include <Eigen/Dense>
#include <unordered_map>
#include <vector>

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
  void blur(const int dim); 
  void blur(const int dim, const int tap); 

  // Interpolation
  void resample(const ImageType_1& im_in, ImageType_1* im_out) const;
  void resample(const XImage& im_in, ImageType_1* im_out) const;

  // Properties
  int dims() const; // dimension of the grid
 private:
  class Sample{ // Nested class to hide impl details
   public:  
     Sample(){};
     unsigned int x, y, r, g, b; // TODO(yichang): init
  }
  SampleCount query(Sample& sample) const;
  void add_sample(Sample& sample) const;
  std::unordered_map<Sample, SampleCount> grid_; // spare grid
  std::vector<float> cell_size
};

} // namespace xform
#endif // SRC_SPARSE_GRID_H
