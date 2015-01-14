#include <iostream>
#include <fstream>
#include "gtest/gtest.h"
#include "XImage.h"
#include "Warp.h"
#include "Curve.h"
#include "ColorSpace.h"
#include "TransformModel.h"
#include "LocalLaplacian.h"
#include "static_image.h"
#include "image_io.h"
#include <sys/time.h>
TEST(TransformModelTest, fit_recipe){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, lab, new_lab(3), out; 
  my_image.read(filename); 

  // Processing by Laplacian filter
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

  xform::TransformModel server_model;
  server_model.use_halide=false;
  server_model.fit_recipe(my_image, out);
}


TEST(TransformModelTest, recon_from_recipe){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, lab, new_lab(3), out; 
  my_image.read(filename); 

  // Processing by Laplacian filter
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
  out.write("TransformTest_recon_by_recipe_gnt.png");

  xform::XImage output; 

  // Server side
  xform::TransformModel server_model;
  server_model.use_halide=false;
  server_model.fit_recipe(my_image, out);

  /* Client side */
  std::ifstream in_file ; // Quant data
  in_file.open("quant.meta");
  xform::PixelType* meta = new xform::PixelType[2*3*3];
  for(int i = 0; i < 2*3*3; i++)
    in_file >> meta[i];

  xform::XImage ac, dc, client_image;
  ac.read("recipe_ac.png");
  dc.read("recipe_dc.png");
  client_image.read(filename);

  // Build model and recipe from client side
  xform::TransformModel client_model;
  client_model.use_halide=false;
  client_model.set_from_recipe(client_image, ac.at(0), dc, meta);

  /* timing */
  timeval t0, t_recon;
  gettimeofday(&t0, NULL);
  xform::XImage reconstructed = client_model.reconstruct();
  gettimeofday(&t_recon, NULL);
  unsigned int t_rec = (t_recon.tv_sec - t0.tv_sec) * 1000000 + (t_recon.tv_usec - t0.tv_usec);
  std::cout<< "t_recon = " << t_rec << std::endl;

  reconstructed.write("TransformTest_recon_by_recipe.png");
}
 TEST(TransformModelTest, recon_from_halide){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, lab, new_lab(3), out; 
  my_image.read(filename); 

  // Processing by Laplacian filter
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
  out.write("TransformTest_recon_by_recipe_gnt.png");

  xform::XImage output; 

  // Server side
  xform::TransformModel server_model;
  server_model.set_images(my_image, out);
  server_model.fit_recipe(my_image, out);

  /* Client side */
  std::ifstream in_file ; // Quant data
  in_file.open("quant.meta");
  xform::PixelType* meta = new xform::PixelType[2*3*3];
  for(int i = 0; i < 2*3*3; i++)
    in_file >> meta[i];

  Image<float> HL_input = load<float>(filename);
  Image<float> HL_ac = load<float>("recipe_ac.png");
  Image<float> HL_dc = load<float>("recipe_dc.png");
  Image<float> HL_output(HL_input.width(), HL_input.height(), 3);
  xform::TransformModel client_model_halide;

  timeval t0, t_recon;
  gettimeofday(&t0, NULL);

  client_model_halide.reconstruct_by_Halide(HL_input, HL_ac, HL_dc, meta, &HL_output);

  /* timing */
  gettimeofday(&t_recon, NULL);
  unsigned int t_rec = (t_recon.tv_sec - t0.tv_sec) * 1000000 + (t_recon.tv_usec - t0.tv_usec);
  std::cout<< "t_recon = " << t_rec << std::endl;

  save(HL_output, "TransformTest_recon_by_halide.png");
}
 
