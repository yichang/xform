#include "gtest/gtest.h"
#include "XImage.h"
#include "ColorSpace.h"
#include "Curve.h"
#include "LocalLaplacian.h"
#include "util.h"

TEST(LocalLaplacianTest, adjust_details){
  std::string filename = "../images/alice.png";
  xform::XImage my_image, lab, new_lab(3), out; 
  my_image.read(filename); 

  xform::ColorSpace color_space;
  color_space.rgb2lab(my_image, &lab);
  
  xform::Curve curve;
  xform::LocalLaplacian local_laplacian;
  const xform::PixelType sigma = 8;
  const xform::PixelType alpha = 4;
  const int num_levels = 7;
  const float interval = 7.0f;
  const float l_range = 100.0f;
  local_laplacian.adjustDetails(lab.at(0), sigma, alpha, num_levels, interval, 
                                l_range, &(new_lab.at(0)));
  new_lab.at(1) = lab.at(1);
  new_lab.at(2) = lab.at(2);

  color_space.lab2rgb(new_lab, &out);
  out.write("LocalLaplacianTest_adjus_details.png");
}
