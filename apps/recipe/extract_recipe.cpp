//Example: ./extract_recipe [image_path_1] [image_path_2]
#include <string>
#include "XImage.h"
#include "Recipe.h"
#include "TransformModel.h"

int main(int argc, char** argv){
  
  std::string file_before(argv[1]); 
  std::string file_after(argv[2]); 

  xform::XImage before_proc, after_proc;
  before_proc.read(file_before);
  after_proc.read(file_after);
  //Sanity checks
  assert(before_proc.rows()==after_proc.rows());
  assert(before_proc.cols()==after_proc.cols());

  xform::TransformModel server_model;
  server_model.set_images(before_proc, after_proc);
  server_model.fit();

  return 0;
}
