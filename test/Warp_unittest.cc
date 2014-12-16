#include "gtest/gtest.h"
#include "XImage.h"
#include "Warp.h"

TEST(WarpTest, resize_05){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, resize_out; 
  my_image.read(filename); 
  xform::Warp warp; 
  warp.imresize(my_image, 0.5, &resize_out);
  resize_out.write("WarpTest_resize_05.png");
}
TEST(WarpTest, resize_04){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, resize_out; 
  my_image.read(filename); 
  xform::Warp warp; 
  warp.imresize(my_image, 0.4, &resize_out);
  resize_out.write("WarpTest_resize_04.png");
}
TEST(WarpTest, resize_18){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, resize_out; 
  my_image.read(filename); 
  xform::Warp warp; 
  warp.imresize(my_image, 1.8, &resize_out);
  resize_out.write("WarpTest_resize_18.png");
}
TEST(WarpTest, resize_20){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, resize_out; 
  my_image.read(filename); 
  xform::Warp warp; 
  warp.imresize(my_image, 2.0, &resize_out);
  resize_out.write("WarpTest_resize_20.png");
}
TEST(WarpTest, resize_05_nn){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, resize_out; 
  my_image.read(filename); 
  xform::Warp warp; 
  warp.imresize(my_image, 0.5, xform::Warp::NEAREST, &resize_out);
  resize_out.write("WarpTest_resize_05_nn.png");
}
TEST(WarpTest, resize_04_nn){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, resize_out; 
  my_image.read(filename); 
  xform::Warp warp; 
  warp.imresize(my_image, 0.4, xform::Warp::NEAREST, &resize_out);
  resize_out.write("WarpTest_resize_04_nn.png");
}
TEST(WarpTest, resize_18_nn){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, resize_out; 
  my_image.read(filename); 
  xform::Warp warp; 
  warp.imresize(my_image, 1.8, xform::Warp::NEAREST, &resize_out);
  resize_out.write("WarpTest_resize_18_nn.png");
}
TEST(WarpTest, resize_20_nn){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, resize_out; 
  my_image.read(filename); 
  xform::Warp warp; 
  warp.imresize(my_image, 2.0, xform::Warp::NEAREST, &resize_out);
  resize_out.write("WarpTest_resize_20_nn.png");
}







