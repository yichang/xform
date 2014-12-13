#include "gtest/gtest.h"
#include "util/util.h"

TEST(UtilTest, Read) {
  string filename = "../images/yichang.png";
  ImageType_3 my_image;  
  ASSERT_TRUE(imread(filename, &my_image));
}

TEST(UtilTest, Write) {
  string filename = "../images/yichang.png";
  ImageType_3 my_image;  
  imread(filename, &my_image);
  my_image(1).array() = my_image(1).array() + 0.4; 
  ASSERT_TRUE(imwrite(my_image, "here.png"));
}

