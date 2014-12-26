#include <limits>
#include <math.h>
#include "SparseGrid.h"
#include <iostream>

using namespace xform;

void SparseGrid::setCellSize(const Eigen::VectorXf& cell_size_){
  cell_size = cell_size_;
}
void SparseGrid::setPixelRange(const Eigen::VectorXf& pixel_range_){
  pixel_range = pixel_range_;
}

void SparseGrid::splat(const XImage& guidance, const ImageType_1& data){

  assert(guidance.rows() == data.rows());
  assert(guidance.cols() == data.cols());

  const int height = guidance.rows();
  const int width = guidance.cols();

  // Compute MaxCellNum
  max_grid_num = (pixel_range.array()/cell_size.array()).cast<Index>() + 1;

  // Compute acc. MaxCellNum
  acc_grid_num = IndexVec(max_grid_num.size());
  acc_grid_num[0] = 1; 
  for(int i = 1; i < max_grid_num.size(); i++){
    acc_grid_num[i] = max_grid_num[i-1] * acc_grid_num[i-1];
    assert(acc_grid_num[i] < std::numeric_limits<Index>::max());
  }
  
  // Create index (including boundary), hash table, and the grid, and splat
  Eigen::VectorXf sample(cell_size.size());
  Index ind;
  int count=0;
  std::vector<PixelType> pixel_vec;
  std::vector<float> pixel_weight_vec;

  /*for(int i=0; i < cell_size.size(); i++)
    std::cout<<max_grid_num[i]<<" ";
  std::cout<<endl;
  for(int i=0; i < cell_size.size(); i++)
    std::cout<<acc_grid_num[i]<<" ";
  std::cout<<endl;*/

  for(int i=0; i < height; i++){
    for(int j=0; j < width; j++){
      // Sample coordinate
      sample(0) = i/cell_size(0); 
      sample(1) = j/cell_size(1); 
      for(int k=2; k < cell_size.size(); k++)
        sample(k) = guidance.at(k-2)(i,j)/cell_size(k);

      // Index of the grid at the rounded sample
      ind = (sample.cast<Index>().array() * acc_grid_num.array()).sum(); 

      // Hash tabl, add to grid, and splat
      if (grid_to_sparse.find(ind) == grid_to_sparse.end()){
        grid_to_sparse[ind] = count;
        grid.push_back(sample.cast<Index>());
        pixel_vec.push_back(data(i,j));
        pixel_weight_vec.push_back(1);
        count++;
      }else{
        Index ind_vec = grid_to_sparse[ind];
        pixel_vec[ind_vec] += data(i,j);
        pixel_weight_vec[ind_vec]++;
      }
  }}
  cout << "NUM vertices: "<< count << endl; 
  // TODO:Create the graph
        
}

void SparseGrid::blur(){
  pixels = graph.cast<float>() * pixels + 2.0f * pixels;
  pixels /= 4.0f;
}

void SparseGrid::slice(const XImage& guidance, ImageType_1* im_out) const{
  // Compute boundary index through hash table
  // Interpolation
}

int SparseGrid::dims() const{ 
  return cell_size.size(); 
}
int SparseGrid::num_grids() const{ 
  return grid.size(); 
}
