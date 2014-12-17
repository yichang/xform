#include <string>
#include "util.h"

#ifndef SRC_PYRAMID
#define SRC_PYRAMID

namespace xform{

class Pyramid{
 public:
  enum FilterType{
    LAPLACIAN,
    GAUSSIAN,
  }
  Pyramid(){};
  Pyramid(const int num_levels);
  Pyramid(const ImageType_1& im_in, const int num_levels, 
          FilterType filter_type);

  // Modifers
  ImageType_1& at(int level);
  const ImageType_1& at(int level) const;

  // Construct the pyramid
  void construct(const ImageType_1& im_in, const int num_levels);

  // Collapse the pyramid,
  void collapse(ImageType_1* im_out) const; 

  int levels() const;
  bool setZero();
  bool write(const std::string& filename);
 private:
  FilterType filter_type;
  ImageType_N nd_array; 
};

} // namespace xform

#endif // SRC_PYRAMID
