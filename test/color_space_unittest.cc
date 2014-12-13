#include "gtest/gtest.h"
#include "color_space.h"
#include "util.h"

TEST(ColorSpaceTest, rgb2yuv) {
  std::string filename = "../images/yichang.png";
  xform::ImageType_3 my_image; 
  ASSERT_TRUE(xform::imread(filename, &my_image));

  xform::ColorSpace color_space;
  xform::ImageType_3 rgb_2_yuv;
  color_space.rgb2yuv(my_image, &rgb_2_yuv); 
  xform::imwrite(rgb_2_yuv, "rgb_2_yuv.png");
  ASSERT_TRUE(true);
}

