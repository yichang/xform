// Compile: g++ test_readPngToEigen.cpp ../util/util.cpp  -I./../libs/png++-0.2.5/ -I./../libs/eigen-eigen-1306d75b4a21 -lpng15
#include "../util/util.h"

int main(){

  string filename = "../images/yichang.png";
  ImageType_3 my_image;  
  readPngToEigen(filename, &my_image);

  return 0; 
}
