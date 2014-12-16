#include <algorithm>
#include <math.h>
#include "XImage.h"
#include "Warp.h"

using namespace xform; 

void Warp::imresize(const XImage& im_in, const float scale, 
                                      XImage* im_out) const{ 
  imresize(im_in, scale, BILINEAR, im_out);
}


void Warp::imresize(const XImage& im_in, const float scale, 
        const InterpType interp_type, XImage* im_out) const{ 

  const int new_h = static_cast<int>(scale * im_in.rows());
  const int new_w = static_cast<int>(scale * im_in.cols());
  imresize(im_in, new_h, new_w, interp_type,  im_out);
}

void Warp::imresize(const XImage& im_in, const int new_h, 
  const int new_w, const InterpType interp_type, XImage* im_out) const{ 
  
  const int num_channels = im_in.channels();
  (*im_out) = XImage(num_channels);
  for(int i = 0; i < num_channels; i++)
    imresize(im_in.at(i), new_h, new_w, interp_type, &(im_out->at(i)));
}

void Warp::imresize(const ImageType_1& im_in, const int new_h, 
  const int new_w, const InterpType interp_type, ImageType_1* im_out) const{ 

  (*im_out) = ImageType_1(new_h, new_w);

  const double x_scale = (static_cast<double>(new_w)-1.0) / 
                         (static_cast<double>(im_in.cols())-1.0);
  const double y_scale = (static_cast<double>(new_h)-1.0) / 
                         (static_cast<double>(im_in.rows())-1.0);

  double l, r, u, b, x, y; 
  if  (interp_type == BILINEAR)
    for(double i=0; i < (double)new_h; i++){
      y = i / y_scale; 
      u = floor(y);  
      b = std::min(u + 1, (double)(im_in.rows()-1)); 
      for(double j=0; j < (double)new_w; j++){
        x = j / x_scale; 
        l = floor(x);  
        r = std::min(l + 1, (double)(im_in.cols()-1));
        (*im_out)(i, j) = (y - u) * (x - l) * im_in(b, r) +  
                          (y - u) * (r - x) * im_in(b, l) +  
                          (b - y) * (x - l) * im_in(u, r) +  
                          (b - y) * (r - x) * im_in(u, l) ; 
      }
    }
  else if (interp_type == NEAREST){
    for(double i=0; i < (double)new_h; i++){
      y = i / y_scale; 
      for(double j=0; j < (double)new_w; j++){
        x = j / x_scale; 
        (*im_out)(i,j) = im_in(y, x);
      }}
  }else{ // BICUBIC
    //TODO(yichang): bicubic 
  }
}


