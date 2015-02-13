#include <sys/time.h>
#include "gtest/gtest.h"
#include "MapImage.h"

#include "static_image.h"
#include "image_io.h"

class MapImageTest : public testing::Test{
 protected:
  virtual void SetUp() {
    string input_filename = "../images/4MP.png";
    h_image = load<float>(input_filename);
  }
  Image <float> h_image;
};

TEST_F(MapImageTest, channel_2) {
  //timeval t1, t_copy;
  //gettimeofday(&t1, NULL);
  xform::MapImage map_image(h_image); 
  //gettimeofday(&t_copy, NULL);
  //unsigned int t_copying = (t_copy.tv_sec - t1.tv_sec) * 1000000 + (t_copy.tv_usec - t1.tv_usec);
  //std::cout<< "t_copying = " << t_copying << std::endl;
  int i = 100, j = 400, c = 2;
  ASSERT_EQ(h_image(j, i, c), map_image.at(c)(i, j));
}
TEST_F(MapImageTest, channel_0) {
  xform::MapImage map_image(h_image); 
  int i = 150, j = 400, c = 0;
  ASSERT_EQ(h_image(j, i, c), map_image.at(c)(i, j));
}


