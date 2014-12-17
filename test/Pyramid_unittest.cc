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
TEST(PyramidTest, construction_gaussian){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image; 
  my_image.read(filename); 
  xform::Pyramid my_gau(xform::Pyramid::GAUSSIAN); 
  my_gau.construct(my_image.at(0), 5);
  my_gau.write("PyramidTest_construction_gaussian");
}


