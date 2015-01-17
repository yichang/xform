//Example: ./extract_recipe [image_path_1] [image_path_2]
#include <string>
#include <fstream>
#include "XImage.h"
#include "Recipe.h"
#include "TransformModel.h"
#include "image_io.h"
#include "static_image.h"

int main(int argc, char** argv){
  
  std::string file_before(argv[1]); 
  std::string file_after(argv[2]); 

  /*xform::XImage before_proc, after_proc;
  before_proc.read(file_before);
  after_proc.read(file_after);
  //Sanity checks
  assert(before_proc.rows()==after_proc.rows());
  assert(before_proc.cols()==after_proc.cols());

  xform::TransformModel server_model;
  server_model.set_images(before_proc, after_proc);
  server_model.fit();*/

  xform::TransformModel server_model;
  const int meta_len = 2 * (3 * server_model.num_affine + 
        server_model.num_scale-1 + server_model.num_bins-1);
  server_model.use_halide=true;
  xform::ImageType_1 ac_lumin_server, ac_chrom_server;
  xform::PixelType* meta_server = new xform::PixelType[meta_len];

  Image<float> HL_input_server = load<float>(file_before),
               HL_output_server = load<float>(file_after);
  const int height = HL_input_server.height(), width = HL_input_server.width();
  const int dc_height = height/std::pow(2,server_model.num_scale-1),
            dc_width = width/std::pow(2,server_model.num_scale-1);
  Image<float> HL_dc_server(dc_width, dc_height, 3);
  
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

  return 0;
}
