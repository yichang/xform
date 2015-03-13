//Example: ./extract_recipe [image_path_1] [image_path_2]
#include <string>
#include <fstream>
#include <sys/time.h>
#include "XImage.h"
#include "Recipe.h"
#include "TransformModel.h"
#include "image_io.h"
#include "static_image.h"
#include "local_laplacian.h"
#include "style_transfer_wrapper.h"

int main(int argc, char** argv){
  
  std::string input_file(argv[1]); 
  Image<float> HL_input_server = load<float>(input_file),
               HL_output_server(HL_input_server.width(), HL_input_server.height(), 3);

  // Processing
  int levels = atoi(argv[2]);
  float alpha = 1;
  float beta = 1;
  int mode = atoi(argv[3]);

  if(mode==0) {
      // Detail enhance
      local_laplacian(levels, alpha/(levels-1), beta, HL_input_server, HL_output_server);
  }else{
      // Style transfer
      std::string model_file(argv[4]); 
      Image<float> HL_model_server = load<float>(model_file);
      style_transfer_wrapper(HL_input_server,HL_model_server,levels,HL_output_server);
  }
  
  // Fitting
  timeval t1, t2;
  gettimeofday(&t1, NULL);
  xform::TransformModel server_model;
  const int meta_len = 2 * (3 * server_model.num_affine + 
        server_model.num_scale-1 + server_model.num_bins-1);
  server_model.use_halide=true;
  xform::ImageType_1 ac_lumin_server, ac_chrom_server;
  xform::PixelType* meta_server = new xform::PixelType[meta_len];

  const int height = HL_input_server.height(), width = HL_input_server.width();
  const int dc_height = height/std::pow(2,server_model.num_scale-1),
            dc_width = width/std::pow(2,server_model.num_scale-1);
  Image<float> HL_dc_server(dc_width, dc_height, 3);
  
  server_model.fit_separate_recipe_by_Halide(HL_input_server, HL_output_server, 
    &ac_lumin_server, &ac_chrom_server, &HL_dc_server, meta_server);

  gettimeofday(&t2, NULL);
  unsigned int t = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
  printf("%u\n", t);

  // Write the output
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
