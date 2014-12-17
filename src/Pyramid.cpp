#include "Pyramid.h"

using namespace xform; 

void Pyramid::Pyramid(const int num_levels){
  nd_array = ImageType_N(num_channels);
}

void Pyramid::construct(const ImageType_1& im_in, const int num_levels){
  nd_array = ImageType_N(num_channels);

  if (filter_type == LAPLACIAN){
    for(int i=0; i < num_levels; i++){
      // J <- filter and downsamplei I 
      // push back Difference of updampled J and I  
      // I <- J
    }
  } else { // GAUSSIAN 
  }
}
