#include <iostream>
#include <fstream>
#include <vector>
#include <sys/time.h>

#include "gtest/gtest.h"
#include "XImage.h"
#include "Warp.h"
#include "Curve.h"
#include "ColorSpace.h"
#include "TransformModel.h"
#include "LocalLaplacian.h"
#include "static_image.h"
#include "image_io.h"

TEST(TransformModelTest, recon_from_seperate_recipes_Halide){
  std::string filename = "../images/yichang.png";
  xform::XImage my_image, lab, new_lab(3), out; 
  my_image.read(filename); 
  // Processing by Laplacian filter
  xform::ColorSpace color_space;
  color_space.rgb2lab(my_image, &lab);
  
  xform::Curve curve;
  xform::LocalLaplacian local_laplacian;
  const xform::PixelType sigma = 8, alpha = 4;
  const int num_levels = 7;
  const float interval = 7.0f, l_range = 100.0f;
  local_laplacian.adjustDetails(lab.at(0), sigma, alpha, num_levels, interval, 
                                l_range, &(new_lab.at(0)));
  new_lab.at(1) = lab.at(1); new_lab.at(2) = lab.at(2);
  color_space.lab2rgb(new_lab, &out);
  out.write("TransformTest_recon_by_recipe_gnt.png");

  // Server side :: will make here 
  xform::TransformModel server_model;
  const int meta_len = 2 * (3 * server_model.num_affine + 
        server_model.num_scale-1 + server_model.num_bins-1);
  server_model.use_halide=true;
  xform::ImageType_1 ac_lumin_server, ac_chrom_server;
  xform::XImage dc_server; 
  xform::PixelType* meta_server = new xform::PixelType[meta_len];

  const int height = my_image.rows(), width = my_image.cols();
  const int dc_height = height/std::pow(2,server_model.num_scale-1),
            dc_width = width/std::pow(2,server_model.num_scale-1);
  Image<float> HL_input_server(width, height, 3),
               HL_output_server(width, height, 3),
               HL_dc_server(dc_width, dc_height, 3);
  my_image.to_Halide(&HL_input_server);
  out.to_Halide(&HL_output_server);
  server_model.fit_separate_recipe_by_Halide(HL_input_server, HL_output_server, 
    &ac_lumin_server, &ac_chrom_server, &HL_dc_server, meta_server);
  save(HL_dc_server, "recipe_dc.png");
  xform::imwrite(ac_lumin_server, "recipe_ac_lumin.png");
  xform::imwrite(ac_chrom_server, "recipe_ac_chrom.png");
  std::ofstream out_file;                                                   
  out_file.open("quant.meta");                                                
  for(int i=0; i < meta_len; i++) // TODO: don't hard code this! 
    out_file<<meta_server[i]<<" ";                              
  out_file.close();                                   

  // Client side
  std::ifstream in_file ; // Quant data
  in_file.open("quant.meta");
  xform::PixelType* meta = new xform::PixelType[meta_len];
  for(int i = 0; i < meta_len; i++)
    in_file >> meta[i];

  Image<float> ac_lumin =load<float>("recipe_ac_lumin.png"), 
               ac_chrom =load<float>("recipe_ac_chrom.png"),
               dc = load<float>("recipe_dc.png"),
               client_image = load<float>(filename),
               recon(client_image.width(), client_image.height(), client_image.channels());
  xform::TransformModel client_model;
  client_model.use_halide=true;
  timeval t0, t_recon;
  gettimeofday(&t0, NULL);
  client_model.reconstruct_separate_by_Halide(
    client_image, ac_lumin, ac_chrom, dc, meta, &recon);
  gettimeofday(&t_recon, NULL);
  unsigned int t_rec = (t_recon.tv_sec - t0.tv_sec) * 1000000 + (t_recon.tv_usec - t0.tv_usec);
  std::cout<< "t_recon = " << t_rec << std::endl;
  save(recon, "TransformTest_recon_by_separate_recipe_halide.png");
}
/*TEST(TransformModelTest, fit_recipe){
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

  xform::ImageType_1 ac;
  xform::XImage dc; 
  xform::PixelType* meta = new xform::PixelType[2*3*3];
  server_model.fit_recipe(my_image, out, &ac, &dc, meta);

  dc.write("recipe_dc.png");
  xform::imwrite(ac, "recipe_ac.png");
  std::ofstream out_file;                                                   
  out_file.open("quant.meta");                                                
  for(int i=0; i < 2*3*3; i++)                         
    out_file<<meta[i]<<" ";                              
  out_file.close();                                   

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
  Image<float> HL_input_server(my_image.cols(), my_image.rows(), my_image.channels()),
               HL_output_server(out.cols(), out.rows(), out.channels()),
               HL_dc_server;
  my_image.to_Halide(&HL_input_server);
  out.to_Halide(&HL_output_server);
  xform::TransformModel server_model;
  xform::ImageType_1 ac_server;
  xform::PixelType* meta_server = new xform::PixelType[2*3*3];
  server_model.fit_recipe_by_Halide(HL_input_server, HL_output_server, &ac_server, &HL_dc_server, meta_server);

  save(HL_dc_server, "recipe_dc.png");
  xform::imwrite(ac_server, "recipe_ac.png");
  std::ofstream out_file;                                                   
  out_file.open("quant.meta");                                                
  for(int i=0; i < 2*3*3; i++)                         
    out_file<<meta_server[i]<<" ";                              
  out_file.close();                                   

  // Client side
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

  gettimeofday(&t_recon, NULL);
  unsigned int t_rec = (t_recon.tv_sec - t0.tv_sec) * 1000000 + (t_recon.tv_usec - t0.tv_usec);
  std::cout<< "t_recon = " << t_rec << std::endl;

  save(HL_output, "TransformTest_recon_by_halide.png");
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
  xform::ImageType_1 ac_server;
  xform::XImage dc_server; 
  xform::PixelType* meta_server = new xform::PixelType[2*3*3];
  server_model.fit_recipe(my_image, out, &ac_server, &dc_server, meta_server);
  dc_server.write("recipe_dc.png");
  xform::imwrite(ac_server, "recipe_ac.png");
  std::ofstream out_file;                                                   
  out_file.open("quant.meta");                                                
  for(int i=0; i < 2*3*3; i++)                         
    out_file<<meta_server[i]<<" ";                              
  out_file.close();                                   

  // Client side
  std::ifstream in_file ; // Quant data
  in_file.open("quant.meta");
  xform::PixelType* meta = new xform::PixelType[2*3*3];
  for(int i = 0; i < 2*3*3; i++)
    in_file >> meta[i];

  xform::XImage ac, dc, client_image;
  ac.read("recipe_ac.png");
  dc.read("recipe_dc.png");
  client_image.read(filename);

  xform::TransformModel client_model;
  client_model.use_halide=false;

  timeval t0, t_recon;
  gettimeofday(&t0, NULL);
  xform::XImage reconstructed = client_model.
      reconstruct(client_image, ac.at(0), dc, meta);
  gettimeofday(&t_recon, NULL);
  unsigned int t_rec = (t_recon.tv_sec - t0.tv_sec) * 1000000 + (t_recon.tv_usec - t0.tv_usec);
  std::cout<< "t_recon = " << t_rec << std::endl;

  reconstructed.write("TransformTest_recon_by_recipe.png");
}
*/
TEST(TransformModelTest, recon_from_seperate_recipes){
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
  const int meta_len = 2 * (3 * server_model.num_affine + 
        server_model.num_scale-1 + server_model.num_bins-1);
  server_model.use_halide=false;
  xform::ImageType_1 ac_lumin_server, ac_chrom_server;
  xform::XImage dc_server; 
  xform::PixelType* meta_server = new xform::PixelType[meta_len];
  server_model.fit_separate_recipe(my_image, out, &ac_lumin_server, 
    &ac_chrom_server, &dc_server, meta_server);
  dc_server.write("recipe_dc.png");
  xform::imwrite(ac_lumin_server, "recipe_ac_lumin.png");
  xform::imwrite(ac_chrom_server, "recipe_ac_chrom.png");
  std::ofstream out_file;                                                   
  out_file.open("quant.meta");                                                
  for(int i=0; i < meta_len; i++) // TODO: don't hard code this! 
    out_file<<meta_server[i]<<" ";                              
  out_file.close();                                   

  // Client side
  std::ifstream in_file ; // Quant data
  in_file.open("quant.meta");
  xform::PixelType* meta = new xform::PixelType[meta_len];
  for(int i = 0; i < meta_len; i++)
    in_file >> meta[i];

  xform::XImage ac_lumin, ac_chrom, dc, client_image;
  ac_lumin.read("recipe_ac_lumin.png");
  ac_chrom.read("recipe_ac_chrom.png");
  dc.read("recipe_dc.png");
  client_image.read(filename);

  xform::TransformModel client_model;
  client_model.use_halide=false;

  timeval t0, t_recon;
  gettimeofday(&t0, NULL);
  xform::XImage reconstructed = client_model.
    reconstruct_separate(client_image, ac_lumin.at(0), ac_chrom.at(0), dc, meta);
  gettimeofday(&t_recon, NULL);
  unsigned int t_rec = (t_recon.tv_sec - t0.tv_sec) * 1000000 + (t_recon.tv_usec - t0.tv_usec);
  std::cout<< "t_recon = " << t_rec << std::endl;

  reconstructed.write("TransformTest_recon_by_separate_recipe.png");
}

