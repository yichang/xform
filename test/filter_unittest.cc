#include "gtest/gtest.h"
#include "x_image.h"
#include "filter.h"

TEST(FilterTest, box_blur_default){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, blur_out; 
  my_image.read(filename); 
  xform::Filter filt; 
  filt.box(my_image, 33, &blur_out);
  blur_out.write("FilterTest_box_blur_default.png");
}

TEST(FilterTest, box_blur_zero_pad){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, blur_out; 
  my_image.read(filename); 
  xform::Filter filt; 
  filt.box(my_image, 33, xform::Filter::ZERO_PAD, &blur_out);
  blur_out.write("FilterTest_box_blur_zero_pad.png");
}
TEST(FilterTest, box_blur_symmetric){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, blur_out; 
  my_image.read(filename); 
  xform::Filter filt; 
  filt.box(my_image, 33, xform::Filter::SYMMETRIC, &blur_out);
  blur_out.write("FilterTest_box_blur_symmetric.png");
}
TEST(FilterTest, box_blur_replicate){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, blur_out; 
  my_image.read(filename); 
  xform::Filter filt; 
  filt.box(my_image, 33, xform::Filter::REPLICATE, &blur_out);
  blur_out.write("FilterTest_box_blur_replicate.png");
}
TEST(FilterTest, box_blur_circular){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, blur_out; 
  my_image.read(filename); 
  xform::Filter filt; 
  filt.box(my_image, 33, xform::Filter::CIRCULAR, &blur_out);
  blur_out.write("FilterTest_box_blur_circular.png");
}
TEST(FilterTest, box_blur_3){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, blur_out; 
  my_image.read(filename); 
  xform::Filter filt; 
  filt.box(my_image, 33, &blur_out);
  filt.box(blur_out, 33, &my_image);
  filt.box(my_image, 33, &blur_out);
  blur_out.write("FilterTest_box_blur_3.png");
}
TEST(FilterTest, box_blur_n){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, blur_out; 
  my_image.read(filename); 
  xform::Filter filt; 
  filt.box_iteration(my_image, 33, 3, &blur_out);
  blur_out.write("FilterTest_box_blur_n.png");
}
