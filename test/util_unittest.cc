#include "gtest/gtest.h"
#include "util.h"

TEST(UtilTest, read) {
  std::string filename = "../images/yichang.png";
  xform::ImageType_3 my_image;  
  ASSERT_TRUE(xform::imread(filename, &my_image));
}

TEST(UtilTest, write) {
  std::string filename = "../images/yichang.png";
  xform::ImageType_3 my_image;  
  xform::imread(filename, &my_image);
  my_image(1).array() = my_image(1).array() + 0.4; 
  ASSERT_TRUE(xform::imwrite(my_image, "here.png"));
}

