// Compile: g++ test_Filter.cpp ../util/util.cpp  ../src/filter.cpp -I./../third_party/png++-0.2.5/ -I./../third_party/eigen-eigen-1306d75b4a21 -I./../ -lpng15; ./a.out
#include "util/util.h"
#include "src/filter.h"

int main(){
  
  string filename = "../images/yichang.png";
  ImageType_3 my_image, im_out;  
  imread(filename, &my_image);

  Filter filter; 
  /* test box filter */
  filter.box(my_image, 33, &im_out); 

  imwrite(im_out, "filter_box.png");

  return 0;
}
