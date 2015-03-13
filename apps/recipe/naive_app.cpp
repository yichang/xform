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
  
  // Write the output
  save(HL_output_server, "naive.jpg");
  return 0;
}
