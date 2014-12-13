// Compile: g++ test_readWritePngToEigen.cpp ../util/util.cpp  -I./../libs/png++-0.2.5/ -I./../libs/eigen-eigen-1306d75b4a21 -I./../ -lpng15
#include <iostream>
#include "util/util.h"
using namespace std;

int main(){

  string filename = "../images/yichang.png";
  ImageType_3 my_image;  
  imread(filename, &my_image);

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
  my_image(1).array() = my_image(1).array() + 0.4; 

  /* Change size */
  //for(int i=0; i < 3; i++)
  //  my_image(i).resize(200, 200);

  imwrite(my_image, "here.png");

  return 0; 
}
