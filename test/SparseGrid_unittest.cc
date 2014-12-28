#include "gtest/gtest.h"
#include "XImage.h"
#include "SparseGrid.h"
#include "util.h"

TEST(SparseGridTest, splat_3D){
  std::string filename = "../images/noisy.png";
  xform::XImage my_image; 
  my_image.read(filename); 
  xform::SparseGrid sparse_grid; 
  Eigen::VectorXf cell_size(3);
  cell_size(0) = 14;
  cell_size(1) = 14;
  cell_size(2) = 0.1;

  Eigen::VectorXf pixel_range(3);
  pixel_range(0) = my_image.rows();
  pixel_range(1) = my_image.cols();
  pixel_range(2) = xform::PIX_UPPER_BOUND;

  sparse_grid.setDimensions(cell_size, pixel_range);
  xform::XImage guidance(1);
  guidance.at(0) = my_image.at(0);
  sparse_grid.construct(guidance);

  xform::XImage out(3);

  for (int i = 0; i < 3; i++){
    sparse_grid.splat(guidance, my_image.at(i));
    sparse_grid.blur();
    sparse_grid.blur();
    sparse_grid.blur();
    sparse_grid.slice(my_image, &(out.at(i)));
  }

  out.write("SparseGridTest_splat_3D.png");
}

TEST(SparseGridTest, splat_5D){
  std::string filename = "../images/noisy.png";
  xform::XImage my_image; 
  my_image.read(filename); 
  xform::SparseGrid sparse_grid; 
  Eigen::VectorXf cell_size(5);
  cell_size(0) = 14;
  cell_size(1) = 14;
  cell_size(2) = 0.2;
  cell_size(3) = 0.2;
  cell_size(4) = 0.2;

  Eigen::VectorXf pixel_range(5);
  pixel_range(0) = my_image.rows();
  pixel_range(1) = my_image.cols();
  pixel_range(2) = xform::PIX_UPPER_BOUND;
  pixel_range(3) = xform::PIX_UPPER_BOUND;
  pixel_range(4) = xform::PIX_UPPER_BOUND;

  sparse_grid.setDimensions(cell_size, pixel_range);
  sparse_grid.construct(my_image);

  xform::XImage out(3);
  for (int i = 0; i < 3; i++){
    sparse_grid.splat(my_image, my_image.at(i));
    sparse_grid.blur();
    sparse_grid.blur();
    sparse_grid.blur();
    sparse_grid.slice(my_image, &(out.at(i)));
  }
  out.write("SparseGridTest_splat_5D.png");
}
