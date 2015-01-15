#include <string>
#include "Warp.h"
#include "Filter.h"
#include "Pyramid.h"

using namespace xform; 

Pyramid::Pyramid(FilterType filter_type, bool stack):filter_type(filter_type), stack(stack){
}
Pyramid::Pyramid(const int num_levels, const FilterType filter_type, const bool stack) 
        :Pyramid(filter_type, stack){
  nd_array = ImageType_N(num_levels);
}
Pyramid::Pyramid(const ImageType_1& im_in, const int num_levels, 
const FilterType filter_type, const bool stack) : filter_type(filter_type), stack(stack){ 
  construct(im_in, num_levels);  
}

void Pyramid::construct(const ImageType_1& im_in, const int num_levels){

  nd_array = ImageType_N(num_levels);

  Filter filt; 
  Warp warp; 
  int b_width = 3; 
  const float scale = 0.5; 
  const int n_iter = 3; 

  ImageType_1 current = im_in;  
  for(int i=0; i < num_levels-1; i++){
    ImageType_1 blur, next_, recon;  
    filt.box_iteration(current, b_width, n_iter, &blur);
    if (!stack) // pyramid
      warp.imresize(blur, scale * blur.rows(), 
                        scale * blur.cols(), Warp::BILINEAR, &next_); 
    else{ // stack
      next_ = blur;
      b_width /= scale; 
      b_width += 1;
    }

    if (filter_type == LAPLACIAN) {
      ImageType_1 recon; 
      if (!stack)
        warp.imresize(next_, blur.rows(), blur.cols(), Warp::BILINEAR, &recon); 
      else
        recon = next_;
      this->at(i) =  current - recon; 
    } else { // GAUSSIAN
      this->at(i) = current; 
    }
    current = next_; 
  }
  this->at(this->levels()-1) = current; 
}
void Pyramid::collapse(ImageType_1* im_out) const{ 
  assert(filter_type == LAPLACIAN);

  Warp warp;
  ImageType_1 current = this->at(this->levels()-1); 
  for(int i=this->levels()-1; i > 0; i--){ 
    ImageType_1 next_;   
    const int next_height = this->at(i-1).rows(); 
    const int next_width  = this->at(i-1).cols();
    warp.imresize(current, next_height, next_width, Warp::BILINEAR, &next_);
    current = next_ + this->at(i-1); 
  }
  (*im_out) = current;
}

ImageType_1& Pyramid::at(int level){
  assert(level < this->levels());
  return nd_array(level); 
}
const ImageType_1& Pyramid::at(int level) const{
  assert(level < this->levels());
  return nd_array(level); 
}
int Pyramid::levels() const{
  return nd_array.rows();
}
bool Pyramid::setZero(){
  for(int i=0; i < this->levels(); i++)
    this->at(i).setZero();
}

#ifndef __ANDROID__
bool Pyramid::write(const std::string& prefix){
  // TODO(yichang) write the layers:
  const PixelType offset = 0.5f; 
  for(int i = 0; i < this->levels(); i++){
    ImageType_3 foo;  
    for(int j=0; j < 3; j++){
      foo(j) = this->at(i);
      if ((i < this->levels()-1) && (this->filter_type == LAPLACIAN))
        foo(j).array() += offset;
    }

    std::string post_fix = std::to_string(i); 
    imwrite(foo,  prefix + "_" + post_fix + ".png");
  }
  return true;
}
#endif
