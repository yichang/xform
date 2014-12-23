#include "gtest/gtest.h"
#include "XImage.h"
#include "Filter.h"
#include "util.h"

TEST(FilterTest, horizontal_gradient){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, out; 
  my_image.read(filename); 
  xform::Filter filt; 
  xform::KernelType_2D kernel(1,2);
  kernel(0,0) =   1;
  kernel(0,1) =  -1;
  filt.convolve(my_image, kernel, xform::Filter::REPLICATE, &out);
  for(int i=0; i < my_image.channels(); i++)
    out.at(i).array() += 0.5;
  out.write("FilterTest_gradient_h.png");
}
TEST(FilterTest, horizontal_laplacian){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, out; 
  my_image.read(filename); 
  xform::Filter filt; 
  xform::KernelType_2D kernel(1,3);
  kernel(0,0) =  -1;
  kernel(0,1) =  2;
  kernel(0,2) =  -1;
  filt.convolve(my_image, kernel, xform::Filter::REPLICATE, &out);
  for(int i=0; i < my_image.channels(); i++)
    out.at(i).array() += 0.5;
  out.write("FilterTest_laplacian_h.png");
}
TEST(FilterTest, vertical_gradient){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, out; 
  my_image.read(filename); 
  xform::Filter filt; 
  xform::KernelType_2D kernel(2,1);
  kernel(0,0) =   1;
  kernel(1,0) =  -1;
  filt.convolve(my_image, kernel, xform::Filter::REPLICATE, &out);
  for(int i=0; i < my_image.channels(); i++)
    out.at(i).array() += 0.5;
  out.write("FilterTest_gradient_v.png");
}
TEST(FilterTest, vertical_laplacian){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, out; 
  my_image.read(filename); 
  xform::Filter filt; 
  xform::KernelType_2D kernel(3,1);
  kernel(0,0) =  -1;
  kernel(1,0) =  2;
  kernel(2,0) =  -1;
  filt.convolve(my_image, kernel, xform::Filter::REPLICATE, &out);
  for(int i=0; i < my_image.channels(); i++)
    out.at(i).array() += 0.5;
  out.write("FilterTest_laplacian_v.png");
}
TEST(FilterTest, 2D_laplacian){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, out; 
  my_image.read(filename); 
  xform::Filter filt; 
  xform::KernelType_2D kernel(3,1);
  kernel(0,0) =  -1;
  kernel(1,0) =  2;
  kernel(2,0) =  -1;
  filt.sep_kernel(my_image, kernel, xform::Filter::REPLICATE, &out);
  for(int i=0; i < my_image.channels(); i++)
    out.at(i).array() += 0.5;
  out.write("FilterTest_laplacian_2d.png");
}


TEST(FilterTest, box_blur_sum_area_tab){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, blur_out; 
  my_image.read(filename); 
  xform::Filter filt; 
  filt.boxBySumArea(my_image, 33, xform::Filter::REPLICATE, &blur_out);
  blur_out.write("FilterTest_box_blur_sum_area.png");
}

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
