// Compile: g++ test_readPngToEigen.cpp ../util/util.cpp  -I./../libs/png++-0.2.5/ -I./../libs/eigen-eigen-1306d75b4a21 -lpng15
#include "../util/util.h"
#include <iostream>
using namespace std;

int main(){

  string filename = "../images/yichang.png";
  ImageType_3 my_image;  
  readPngToEigen(filename, my_image);

  /* print pixel values */
  /*for(int i=0; i< my_image(0).rows(); i++){
    for(int j=0; j < my_image(1).cols(); j++){
      cout << "(" << my_image(0)(i,j) <<" "
        << my_image(2)(i,j) << " " << my_image(2)(i,j) <<")"; 
    }
  }*/

  /* Modify channels */
  //my_image(0).setZero(my_image(0).rows(), my_image(0).cols()); 
  //my_image(1).setZero(my_image(0).rows(), my_image(0).cols()); 

  /* Modify image value */
  //my_image(2) *= 2.0; 

  /* Change size */
  //for(int i=0; i < 3; i++)
  //  my_image(i).resize(200, 200);

  writePngToEigen(my_image, "here.png");

  return 0; 
}
