#include "gtest/gtest.h"
#include "XImage.h"

TEST(XImageTest, init_by_nchannels) {
  xform::XImage image(3); 
  ASSERT_TRUE(true);
}

TEST(XImageTest, init_by_dims) {
  xform::XImage image(100, 200, 3); 
  ASSERT_TRUE(true);
}

TEST(XImageTest, dims) {
  xform::XImage image(100, 200, 3); 
  EXPECT_EQ(image.rows(), 100);
  EXPECT_EQ(image.cols(), 200);
  EXPECT_EQ(image.channels(), 3);
}

TEST(XImageTest, at) {
  xform::XImage image(100, 200, 3); 
  image.at(2).setZero(); 
  ASSERT_TRUE(true);
}

TEST(XImageTest, at_per_coeff) {
  xform::XImage image(10, 20, 3); 
  image.at(2).setZero(); 
  image.at(2)(3,4) = 76;  
  EXPECT_EQ(image.at(2)(3,4), 76);
}

TEST(XImageTest, setZero) {
  xform::XImage image(100, 200, 4); 
  image.at(3)(19,114) = 4.5; 
  EXPECT_EQ(image.at(3)(19, 114),4.5);
  image.setZero();
  EXPECT_EQ(image.at(3)(19, 114), 0);
}

TEST(XImageTest, read_and_write) {
  string input_filename = "../images/yichang.png";
  string output_filename = "XImageTest_read_and_write.png";
  xform::XImage image;
  ASSERT_TRUE(image.read(input_filename));
  image.at(1).array() += 0.2; 
  ASSERT_TRUE(image.write(output_filename));
}


