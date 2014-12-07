// Make: g++ test_png.cpp -lpng15 
// Test proper installation of png++ and libpng

#include "../libs/png++-0.2.5/png.hpp"
#include <string>

using namespace std;

int main(){

  string filename = "../images/yichang.png";
  png::image<png::rgb_pixel> image(filename);

  int height = image.get_height();
  int width = image.get_width();

  for(int i=0; i < height; i++){
    for(int j=0; j < width ; j++){
      png::rgb_pixel rgbPixel = image.get_pixel(j, i);
      float r = 1.5*(float)rgbPixel.red;   
      float g = 1.5*(float)rgbPixel.green;   
      float b = 1.5*(float)rgbPixel.blue;   

      r = (r>255.0f)? 255.0 : r; 
      g = (g>255.0f)? 255.0 : g; 
      b = (b>255.0f)? 255.0 : b; 
      
      png::rgb_pixel newPixel; 
      newPixel.red = r; 
      newPixel.green = g; 
      newPixel.blue = b; 
      image.set_pixel(j, i, newPixel);
    }
  }

  image.write("output.png");
  return 0; 
}
