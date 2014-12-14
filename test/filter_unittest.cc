#include "gtest/gtest.h"
#include "x_image.h"
#include "filter.h"

TEST(FilterTest, box_blur){

  std::string filename = "../images/yichang.png";
  xform::XImage my_image, blur_out; 
  my_image.read(filename); 
  xform::Filter filt; 
  filt.box(my_image, 33, &blur_out);
  blur_out.write("FilterTest_box_blur.png");
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
