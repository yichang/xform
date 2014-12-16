#include "gtest/gtest.h"
#include "ColorSpace.h"
#include "util.h"

TEST(ColorSpaceTest, rgb2yuv) {
  std::string filename = "../images/yichang.png";
  xform::XImage my_image; 
  my_image.read(filename);

  xform::ColorSpace color_space;
  xform::XImage rgb_2_yuv;
  color_space.rgb2yuv(my_image, &rgb_2_yuv); 
  rgb_2_yuv.write("ColorSpaceTest_rgb_2_yuv.png");
  ASSERT_TRUE(true);
}

TEST(ColorSpaceTest, yuv_2_rgb) {
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, rgb_2_yuv; 
  my_image.read(filename);

  xform::ColorSpace color_space;
  color_space.rgb2yuv(my_image, &rgb_2_yuv); 
  color_space.yuv2rgb(rgb_2_yuv, &my_image); 

  my_image.write("ColorSpaceTest_yuv_2_rgb.png");
  ASSERT_TRUE(true);
}
TEST(ColorSpaceTest, yuv_2_rgb_modified_y) {
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, rgb_2_yuv; 
  my_image.read(filename);

  xform::ColorSpace color_space;
  color_space.rgb2yuv(my_image, &rgb_2_yuv); 
  rgb_2_yuv.at(0) *= 0.8;
  color_space.yuv2rgb(rgb_2_yuv, &my_image); 

  my_image.write("ColorSpaceTest_yuv_2_rgb_modified_y.png");
  ASSERT_TRUE(true);
}

TEST(ColorSpaceTest, rgb2xyz) {
  std::string filename = "../images/yichang.png";
  xform::XImage my_image; 
  my_image.read(filename);

  xform::ColorSpace color_space;
  xform::XImage rgb_2_xyz;
  color_space.rgb2xyz(my_image, &rgb_2_xyz); 
  rgb_2_xyz.write("ColorSpaceTest_rgb_2_xyz.png");
  ASSERT_TRUE(true);
}

TEST(ColorSpaceTest, xyz_2_rgb) {
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, rgb_2_xyz; 
  my_image.read(filename);

  xform::ColorSpace color_space;
  color_space.rgb2xyz(my_image, &rgb_2_xyz); 
  color_space.xyz2rgb(rgb_2_xyz, &my_image); 

  my_image.write("ColorSpaceTest_xyz_2_rgb.png");
  ASSERT_TRUE(true);
}

TEST(ColorSpaceTest, rgb2lab) {
  std::string filename = "../images/yichang.png";
  xform::XImage my_image; 
  my_image.read(filename);

  xform::ColorSpace color_space;
  xform::XImage rgb_2_lab;
  color_space.rgb2lab(my_image, &rgb_2_lab); 
  rgb_2_lab.write("ColorSpaceTest_rgb_2_lab.png");
  ASSERT_TRUE(true);
}

TEST(ColorSpaceTest, lab_2_rgb) {
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, rgb_2_lab; 
  my_image.read(filename);

  xform::ColorSpace color_space;
  color_space.rgb2lab(my_image, &rgb_2_lab); 
  color_space.lab2rgb(rgb_2_lab, &my_image); 

  my_image.write("ColorSpaceTest_lab_2_rgb.png");
  ASSERT_TRUE(true);
}

TEST(ColorSpaceTest, lab_2_rgb_modified_y) {
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, rgb_2_lab; 
  my_image.read(filename);

  xform::ColorSpace color_space;
  color_space.rgb2lab(my_image, &rgb_2_lab); 
  rgb_2_lab.at(0) *= 0.75; 
  color_space.lab2rgb(rgb_2_lab, &my_image); 

  my_image.write("ColorSpaceTest_lab_2_rgb_modified_y.png");
  ASSERT_TRUE(true);
}

