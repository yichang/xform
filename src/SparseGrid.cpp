#include <limits>
#include <math.h>
#include "SparseGrid.h"
#include <iostream>

using namespace xform;
using namespace std;

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
      // TODO(yichang): pad the grid.
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
  pixels = edge.cast<PixelType>() * pixels + dims() * pixels;
  pixel_weights = edge.cast<PixelType>() * pixel_weights + 
                                       dims() * pixel_weights;
}
void SparseGrid::normalize(){
  pixels.array() /= pixel_weights.array();
}
void SparseGrid::neighborPattern(const int dim, vector<Vertex>* out) const{ 
  const int num_neighbors = pow(2, dim);
  for(int i=0; i < num_neighbors; i++){
    Vertex z = Vertex(dim);
    int count = i;
    for(int j=0; j < dim; j++){
      z(j) = count%2;
      count /=2;
    }
    out->push_back(z);
  }
}
void SparseGrid::slice(const XImage& guidance, ImageType_1* im_out) const{
  // Compute boundary index through hash table
  // Interpolation
  const int height = guidance.rows();
  const int width  = guidance.cols();
  const bool FLAG_multi_linear_interp = true;

  Sample sample(dims());

  *im_out = ImageType_1(height, width);

  LongIndex sample_index, vertex_index;

  vector<Vertex> neighbors;
  neighborPattern(dims(), &neighbors); 
  const int num_neighbors = neighbors.size();

  for(int i=0; i < height; i++){
    for(int j=0; j < width; j++){
      // Sample coordinate
      sample(0) = i/cell_size(0); 
      sample(1) = j/cell_size(1); 
      for(int k=2; k < cell_size.size(); k++)
        sample(k) = guidance.at(k-2)(i,j)/cell_size(k);

      if (FLAG_multi_linear_interp){
        Weight weight_sum = 0;
        PixelType pixel = 0;
        Weight weight;
        for(int k = 0; k < num_neighbors; k++){
          Vertex cur_vert = sample.cast<Index>() + neighbors[k];
          sample_index = (cur_vert.cast<LongIndex>().array() 
                          * acc_grid_num.array()).sum(); 
          
          if (sample_to_vertex.find(sample_index) != sample_to_vertex.end()){
            // Multi-linear weight
            Sample rel_sample = sample - cur_vert.cast<float>();
            weight = multiLinearWeight(rel_sample); 
            // Sample at neightbors
            vertex_index = sample_to_vertex.at(sample_index);
            pixel += weight * pixels(vertex_index);
            weight_sum += weight;
          }
        }
        (*im_out)(i, j) = pixel/weight_sum; 
      } else { // Hard truncate
        // Index of the grid at the rounded sample
        sample_index = (sample.cast<LongIndex>().array() 
                          * acc_grid_num.array()).sum(); 
        vertex_index = sample_to_vertex.at(sample_index);
        (*im_out)(i, j) = pixels(vertex_index); 
     }
  }}
}

SparseGrid::Weight SparseGrid::multiLinearWeight(const Sample& rel_sample) 
const{ 
  // Corresponding weight in multi-linear interp
  const int dim = dims();
  Weight out = 1.0f;
  for(int i=0; i < dim; i++){
    if (rel_sample(i) > 0.0f)
      out *= (1.0f - rel_sample(i));
    else if (rel_sample(i) < 0)
      out *= (1.0f + rel_sample(i));
    else if (rel_sample(i) == 1)
      return 0.0f;
    else{}
  }
  assert(out >= 0);
  assert(out <= 1);
  return abs(out);
}

int SparseGrid::dims() const{ 
  return cell_size.size(); 
}
int SparseGrid::num_vertices() const{ 
  return sample_to_vertex.size(); 
}
