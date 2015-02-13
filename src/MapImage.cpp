#include <assert.h>
#include "MapImage.h"
#include "static_image.h"

using namespace xform;

MapImage::MapImage(const Image<float>& h_image){
  for(int i = 0; i < h_image.channels(); i++){
    int offset = i * h_image.height() * h_image.width();
    MapImageType_1 mapper(h_image.data() + offset, h_image.height(), h_image.width());
    nd_array.push_back(mapper);
  }
}
MapImage::MapImage(const Image<float>& h_image, int start, int end){
  assert(start >= 0);
  assert(end <= h_image.channels());

  for(int i = start; i < end; i++){
    int offset = i * h_image.height() * h_image.width();
    MapImageType_1 mapper(h_image.data() + offset, h_image.height(), h_image.width());
    nd_array.push_back(mapper);
  }
}
const MapImageType_1& MapImage::at(int channel) const{
  return nd_array[channel];
}
int MapImage::cols() const{
  assert(this->channels() > 0);
  return nd_array[0].cols(); 
}

int MapImage::rows() const{
  assert(this->channels() > 0);
  return nd_array[0].rows(); 
}

int MapImage::channels() const{
  return nd_array.size(); 
}


