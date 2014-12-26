#include "gtest/gtest.h"
#include "XImage.h"
#include "SparseGrid.h"
#include "util.h"

TEST(SparseGridTest, splat_3D){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, out; 
  my_image.read(filename); 
  xform::SparseGrid sparse_grid; 
  Eigen::VectorXf cell_size(3);
  cell_size(0) = 4;
  cell_size(1) = 4;
  cell_size(2) = 0.2;
  sparse_grid.setCellSize(cell_size);

  Eigen::VectorXf pixel_range(3);
  pixel_range(0) = my_image.rows();
  pixel_range(1) = my_image.cols();
  pixel_range(2) = xform::PIX_UPPER_BOUND;
  sparse_grid.setPixelRange(pixel_range);

  xform::XImage guidance(1);
  guidance.at(0) = my_image.at(0);
  sparse_grid.splat(guidance, guidance.at(0));
}

TEST(SparseGridTest, splat_5D){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, out; 
  my_image.read(filename); 
  xform::SparseGrid sparse_grid; 
  Eigen::VectorXf cell_size(5);
  cell_size(0) = 4;
  cell_size(1) = 4;
  cell_size(2) = 0.2;
  cell_size(3) = 0.2;
  cell_size(4) = 0.2;
  sparse_grid.setCellSize(cell_size);

  Eigen::VectorXf pixel_range(5);
  pixel_range(0) = my_image.rows();
  pixel_range(1) = my_image.cols();
  pixel_range(2) = xform::PIX_UPPER_BOUND;
  pixel_range(3) = xform::PIX_UPPER_BOUND;
  pixel_range(4) = xform::PIX_UPPER_BOUND;
  sparse_grid.setPixelRange(pixel_range);

  sparse_grid.splat(my_image, my_image.at(0));
  
}
