#include <vector>
#include "util.h"
#include "static_image.h"

#ifndef SRC_MAP_IMAGE_H
#define SRC_MAP_IMAGE_H

namespace xform{


class MapImage{
 public: 

  MapImage(){};
  MapImage(const Image<float>& h_image);
  MapImage(const Image<float>& h_image, int start, int end);

  int cols() const; 
  int rows() const;
  int channels() const;
  const MapImageType_1& at(int channel) const;

  //Eigen::Matrix<MapImageType_1, Eigen::Dynamic, 1> nd_array;
  std::vector<MapImageType_1> nd_array;
};

} // namespace xform

#endif // SRC_MAP_IMAGE
