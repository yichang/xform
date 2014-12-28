#include <limits>
#include <math.h>
#include "SparseGrid.h"
#include <iostream>

using namespace xform;

void SparseGrid::setDimensions(const Eigen::VectorXf& cell_size_, 
                               const Eigen::VectorXf& pixel_range_){
  cell_size = cell_size_;
  pixel_range = pixel_range_;

  // Compute max length at each dimension
  max_grid_num = (pixel_range.array()/cell_size.array()).cast<Index>() + 1;

  // Compute accumulated max dimension
  acc_grid_num = Eigen::Matrix<LongIndex, Eigen::Dynamic, 1>(dims());
  acc_grid_num[0] = 1; 
  for(int i = 1; i < max_grid_num.size(); i++){
    Index m = max_grid_num[i-1];
    acc_grid_num[i] = static_cast<LongIndex>(m) * acc_grid_num[i-1];
    assert(acc_grid_num[i] < std::numeric_limits<LongIndex>::max());
  }
}
void SparseGrid::splat(const XImage& guidance, const ImageType_1& data){
  assert(guidance.rows() == data.rows());
  assert(guidance.cols() == data.cols());

  const int height = guidance.rows();
  const int width = guidance.cols();
  pixels = Pixels(num_vertices());
  pixels.setZero(); 

  pixel_weights = PixelWeights(num_vertices());
  pixel_weights.setZero();

  Sample sample(dims());
  LongIndex sample_index;

  for(int i=0; i < height; i++){
    for(int j=0; j < width; j++){
      // Sample coordinates
      sample(0) = i/cell_size(0); 
      sample(1) = j/cell_size(1); 
      for(int k=2; k < cell_size.size(); k++)
        sample(k) = guidance.at(k-2)(i,j)/cell_size(k);

      // Index of the grid at the rounded sample
      // TODO(yichang): actual splatting by multi-linear interp.
      sample_index = (sample.cast<LongIndex>().array() * 
                             acc_grid_num.array()).sum(); 

        LongIndex vertex_index = sample_to_vertex[sample_index];
        pixels(vertex_index) += data(i,j);
        pixel_weights(vertex_index)++;
    }}
}
void SparseGrid::construct(const XImage& guidance){

  const int height = guidance.rows();
  const int width = guidance.cols();

  vector<Vertex> vertices;
 
  // Create index (including boundary), hash table, and the grid, and splat
  Sample sample(dims());
  LongIndex sample_index;
  LongIndex vertex_index=0;

  for(int i=0; i < height; i++){
    for(int j=0; j < width; j++){
      // Sample coordinate
      sample(0) = i/cell_size(0); 
      sample(1) = j/cell_size(1); 
      for(int k=2; k < cell_size.size(); k++)
        sample(k) = guidance.at(k-2)(i,j)/cell_size(k);

      // Index of the grid at the rounded sample
      sample_index = (sample.cast<LongIndex>().array() * 
                             acc_grid_num.array()).sum(); 

      // Build the mapping through hash table
      if (sample_to_vertex.find(sample_index) == sample_to_vertex.end()){
        sample_to_vertex[sample_index] = vertex_index;
        vertices.push_back(sample.cast<Index>());
        vertex_index++;
      }
    }}

  // Create the graph
  const int num_vert = num_vertices();
  edge = Edge(num_vert, num_vert);
  for(int i = 0; i < num_vert; i++){
    for(int j = i + 1; j < num_vert; j++){
      if (isNeighbor(vertices[i], vertices[j])){ // is neighbor
        edge.insert(i, j) = true;
        edge.insert(j, i) = true;
    }}}
}
bool SparseGrid::isNeighbor(const Vertex& a, const Vertex& b) const{
  assert(a.rows() == b.rows());
  unsigned int count = 0;
  for(int i = 0; i < a.rows(); i++){
    Index diff = abs(a(i) - b(i));
    if (diff > 1)
      return false;

    if (diff == 1)
      count++;

    if (count > 1)
      return false;
  }
  return true;
}

void SparseGrid::blur(){
  pixels = edge.cast<float>() * pixels + dims() * pixels;
  pixel_weights = edge.cast<float>() * pixel_weights + 
                                       dims() * pixel_weights;
}

void SparseGrid::slice(const XImage& guidance, ImageType_1* im_out) const{
  // Compute boundary index through hash table
  // Interpolation
  const int height = guidance.rows();
  const int width  = guidance.cols();

  Sample sample(dims());

  *im_out = ImageType_1(height, width);

  LongIndex sample_index, vertex_index;

  for(int i=0; i < height; i++){
    for(int j=0; j < width; j++){
      // Sample coordinate
      sample(0) = i/cell_size(0); 
      sample(1) = j/cell_size(1); 
      for(int k=2; k < cell_size.size(); k++)
        sample(k) = guidance.at(k-2)(i,j)/cell_size(k);

      // Index of the grid at the rounded sample
      sample_index = (sample.cast<LongIndex>().array() 
                        * acc_grid_num.array()).sum(); 

      vertex_index = sample_to_vertex.at(sample_index);
      // TODO(yichang): multi-linear interpolation
      
      (*im_out)(i, j) = pixels(vertex_index)/pixel_weights(vertex_index); 
  }}
}

int SparseGrid::dims() const{ 
  return cell_size.size(); 
}
int SparseGrid::num_vertices() const{ 
  return sample_to_vertex.size(); 
}
