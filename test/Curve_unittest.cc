#include "gtest/gtest.h"
#include "XImage.h"
#include "ColorSpace.h"
#include "Curve.h"
#include "util.h"

TEST(CurveTest, s_shape){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, lab, new_lab(3), out; 
  my_image.read(filename); 

  xform::ColorSpace color_space;
  color_space.rgb2lab(my_image, &lab);
  
  xform::Curve curve;
  curve.sShape(lab.at(0), 10, 35, 0.25, &(new_lab.at(0)));
  new_lab.at(1) = lab.at(1);
  new_lab.at(2) = lab.at(2);

  color_space.lab2rgb(new_lab, &out);
  out.write("CurveTest_s_shape.png");
}
