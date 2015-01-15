#include <assert.h>
#include "XImage.h" 
#include "static_image.h"

using namespace xform;

XImage::XImage(int num_channels){
  nd_array = ImageType_N(num_channels);
} 

XImage::XImage(int num_rows, int num_cols, int num_channels) {
   nd_array = ImageType_N(num_channels);
  int n_channels = this->channels();
  for(int i = 0; i < n_channels; i++){ 
    this->at(i) = ImageType_1(num_rows, num_cols);
  }
}

// Need to check this carefully
ImageType_1& XImage::at(int channel){
  assert(channel < this->channels());
  return nd_array(channel); 
}

const ImageType_1& XImage::at(int channel) const{
  assert(channel < this->channels());
  return nd_array(channel); 
}

int XImage::cols() const{
  assert(this->channels() > 0);
  return nd_array(0).cols(); 
}

int XImage::rows() const{
  assert(this->channels() > 0);
  return nd_array(0).rows(); 
}

int XImage::channels() const{
  return nd_array.rows(); 
}

bool XImage::setZero(){
  for(int i=0; i < this->channels(); i++)
    this->at(i).setZero();
  return true;
}
bool XImage::setOnes(){
  for(int i=0; i < this->channels(); i++)
    this->at(i).setOnes();
  return true;
}

XImage XImage::operator+(const XImage& rhs) const {
    // TODO: check same size
    XImage out(this->rows(), this->cols(), this->channels());
    for(int i=0; i < this->channels(); i++)
    {
        out.at(i)  = this->at(i) + rhs.at(i);
    }
    return out;
}
XImage XImage::operator-(const XImage& rhs) const {
    // TODO: check same size
    XImage out(this->rows(), this->cols(), this->channels());
    for(int i=0; i < this->channels(); i++)
    {
        out.at(i)  = this->at(i) - rhs.at(i);
    }
    return out;
}


bool XImage::to_Halide(Image<float>* h_image) const{
  assert(cols()==h_image->width());
  assert(rows()==h_image->height());
  assert(channels()==h_image->channels());
  
  for(int c=0 ; c < channels(); c++)
    for(int i=0; i < rows(); i++)
      for(int j=0; j < cols(); j++)
        (*h_image)(j,i,c) = this->at(c)(i,j);
  return true;
}

bool XImage::from_Halide(const Image<float>& h_image){
  assert(cols()==h_image.width());
  assert(rows()==h_image.height());
  assert(channels()==h_image.channels());

  for(int c=0 ; c < channels(); c++)
    for(int i=0; i < rows(); i++)
      for(int j=0; j < cols(); j++)
        this->at(c)(i,j) = h_image(j, i, c);
  return true;
}

#ifndef __ANDROID__
bool XImage::read(const std::string& filename){
  //TODO: change this
  ImageType_3 foo; 
  imread(filename, &foo);
  nd_array = ImageType_N(3);
  for(int i=0; i < 3; i++)
    this->at(i) = foo(i);
  return true;
}
bool XImage::write(const std::string& filename){
  //TODO: change this
  assert(this->channels() == 3);
  ImageType_3 foo; 
  for(int i=0; i < 3; i++)
    foo(i) = this->at(i);
  imwrite(foo, filename);
  return true;
}
#endif
