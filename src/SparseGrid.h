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

  typedef long int LongIndex; // linear indices for sample and vertex
  typedef int Index; // for vertex along each dimension
  // Note that different notion from Vertex to IndexVec
  typedef Eigen::Matrix<Index, Eigen::Dynamic, 1> Vertex; 
  typedef Eigen::Matrix<float, Eigen::Dynamic, 1> Sample; 
  typedef Eigen::Matrix<PixelType, Eigen::Dynamic, 1> Pixels;
  typedef Eigen::Matrix<float, Eigen::Dynamic, 1> PixelWeights;
  typedef Eigen::SparseMatrix<bool> Edge; 

  SparseGrid(){};

  // Construct the grid by building adding vertices and edges
  void construct(const XImage& guidance);

  // Splat the data
  void splat(const XImage& guidance, const ImageType_1& data);

  // Blur by [1 2 1] filter along each dim
  void blur(); 

  // Interpolation
  void slice(const XImage& guidance, ImageType_1* im_out) const;

  // Set properties
  void setDimensions(const Eigen::VectorXf& cell_size_, 
                     const Eigen::VectorXf& pixel_range_);
  // Properties
  int dims() const; // dimension of the grid
  int num_vertices() const; 

 private:
  bool isNeighbor(const Vertex& a, const Vertex& b) const;

  Eigen::VectorXf cell_size;
  Eigen::VectorXf pixel_range;

  Eigen::Matrix<Index, Eigen::Dynamic, 1> max_grid_num;
  Eigen::Matrix<LongIndex, Eigen::Dynamic, 1> acc_grid_num;

  std::unordered_map <LongIndex, LongIndex> sample_to_vertex;
  Edge edge;

  Pixels pixels;
  PixelWeights pixel_weights;
};
} // namespace xform
#endif // SRC_SPARSE_GRID_H
