#include <iostream>
#include <fstream>
#include "gtest/gtest.h"
#include "XImage.h"
#include "Warp.h"
#include "Curve.h"
#include "ColorSpace.h"
#include "Recipe.h"
#include "TransformModel.h"
#include "LocalLaplacian.h"
#include "static_image.h"
#include "image_io.h"

TEST(TransformModelTest, reconstruction){
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
  out.write("TransformTest_recon_gnt.png");

  xform::XImage output; 

  // Server side
  xform::TransformModel server_model;
  server_model.set_images(my_image, out);
  server_model.fit();

  // Client side
  xform::XImage ac, dc, client_image;
  ac.read("recipe_ac.png");
  dc.read("recipe_dc.png");
  client_image.read(filename);

  // Build model and recipe from client side
  xform::TransformModel client_model;
  const int height = ceil(static_cast<float>(client_image.rows())/static_cast<float>(client_model.get_step()));
  const int width = ceil(static_cast<float>(client_image.cols())/static_cast<float>(client_model.get_step()));
  const int n_chan_i = 3; 
  const int n_chan_o = 3; 
  xform::Recipe* recipe = new xform::Recipe(height, width, n_chan_i, n_chan_o);

  /* DC and AC */
  recipe->set_dc(dc);
  recipe->set_ac(ac.at(0));
  std::ifstream in_file;
  in_file.open("quant.meta");
  int num_chan = recipe->n_chan_i * recipe->n_chan_o;
  xform::PixelType* buf = new xform::PixelType[num_chan];

  /* Quant metadata */
  for(int i = 0; i < num_chan; i++)
    in_file >> buf[i];
  recipe->set_quantize_mins(buf, num_chan);

  for(int i = 0; i < num_chan; i++)
    in_file >> buf[i];
  recipe->set_quantize_maxs(buf, num_chan);

  client_model.set_images(client_image, client_image); // the second one is dummy
  client_model.set_recipe(recipe); 

  /* Recon. */
  xform::XImage reconstructed = client_model.predict();
  reconstructed.write("TransformTest_recon.png");
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
  server_model.set_images(my_image, out);
  server_model.fit();

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
  client_model.set_from_recipe(client_image, ac.at(0), dc, meta);
  xform::XImage reconstructed = client_model.predict();
  reconstructed.write("TransformTest_recon_by_recipe.png");

  /*Image<float> HL_input = load<float>(filename);
  Image<float> HL_ac = load<float>("recipe_ac.png");
  Image<float> HL_dc = load<float>("recipe_dc.png");
  Image<float> HL_output(HL_input.width(), HL_input.height(), 3);
  client_model.reconstruct_by_Halide(HL_input, HL_ac, HL_dc, meta, &HL_output)*/;

}
 
