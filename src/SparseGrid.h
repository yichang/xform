#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <unordered_map>
#include <vector>

#include "util.h"
#include "XImage.h"

#ifndef SRC_SPARSE_GRID_H
#define SRC_SPARSE_GRID_H

namespace xform{

class SparseGrid{
 public:

  typedef long int Index;
  typedef Eigen::Matrix<Index, Eigen::Dynamic, 1> IndexVec;
  typedef vector<IndexVec> Grid;
  typedef Eigen::Matrix<PixelType, Eigen::Dynamic, 1> Pixels;
  typedef Eigen::Matrix<float, Eigen::Dynamic, 1> PixelWeights;
  typedef Eigen::SparseMatrix<bool> Graph; 
  

  SparseGrid(){};

  // Construct the grid by guidance and splat the datag
  void splat(const XImage& guidance, const ImageType_1& data);

  // Blur by [1 2 1] filter along each dim
  void blur(); 

  // Interpolation
  void slice(const XImage& guidance, ImageType_1* im_out) const;

  void setCellSize(const Eigen::VectorXf& cell_size_);
  void setPixelRange(const Eigen::VectorXf& pixel_range_);
  // Properties
  int dims() const; // dimension of the grid
  int num_grids() const; 
 private:
  Grid grid;
  Graph graph;
  Pixels pixels;
  PixelWeights pixel_weights;
  Eigen::VectorXf cell_size;
  Eigen::VectorXf pixel_range;
  IndexVec max_grid_num;
  IndexVec acc_grid_num;
  std::unordered_map <Index, Index> grid_to_sparse;
};
} // namespace xform
#endif // SRC_SPARSE_GRID_H
