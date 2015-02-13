#include <vector>
#include "util.h"
#include "static_image.h"

#ifndef SRC_MAP_IMAGE
#define SRC_MAP_IMAGE

namespace xform{


class MapImage{
 public: 

  MapImage(){};
  MapImage(const Image<float>& h_image);

  int cols() const; 
  int rows() const;
  int channels() const;
  const MapImageType_1& at(int channel) const;

  //Eigen::Matrix<MapImageType_1, Eigen::Dynamic, 1> nd_array;
  std::vector<MapImageType_1> nd_array;
};

} // namespace xform

#endif // SRC_MAP_IMAGE
