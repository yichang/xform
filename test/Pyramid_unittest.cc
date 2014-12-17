#include "gtest/gtest.h"
#include "XImage.h"
#include "Pyramid.h"

TEST(PyramidTest, construction_laplacian){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image; 
  my_image.read(filename); 
  xform::Pyramid my_lap(xform::Pyramid::LAPLACIAN); 
  my_lap.construct(my_image.at(0), 5);
  my_lap.write("PyramidTest_construction_laplacian");
}
TEST(PyramidTest, collapse_laplacian){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image; 
  my_image.read(filename); 
  xform::Pyramid my_lap(xform::Pyramid::LAPLACIAN); 
  my_lap.construct(my_image.at(0), 5);

  xform::ImageType_1 my_collapse;
  my_lap.collapse(&my_collapse);

  xform::XImage my_out(3);
  my_out.at(0) = my_collapse;
  my_out.at(1) = my_collapse;
  my_out.at(2) = my_collapse;
  my_out.write("PyramidTest_collapse_laplacian.png");
}
TEST(PyramidTest, collapse_laplacian_modified){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image; 
  my_image.read(filename); 
  xform::Pyramid my_lap(xform::Pyramid::LAPLACIAN); 
  my_lap.construct(my_image.at(0), 5);
  my_lap.at(2) *= 1.5; 

  xform::ImageType_1 my_collapse;
  my_lap.collapse(&my_collapse);

  xform::XImage my_out(3);
  my_out.at(0) = my_collapse;
  my_out.at(1) = my_collapse;
  my_out.at(2) = my_collapse;
  my_out.write("PyramidTest_collapse_laplacian_modified.png");
}


TEST(PyramidTest, construction_gaussian){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image; 
  my_image.read(filename); 
  xform::Pyramid my_gau(xform::Pyramid::GAUSSIAN); 
  my_gau.construct(my_image.at(0), 5);
  my_gau.write("PyramidTest_construction_gaussian");
}


