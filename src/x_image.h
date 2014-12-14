#include <string>
#include "util.h"

#ifndef SRC_X_IMAGE
#define SRC_X_IMAGE

namespace xform{

class XImage{
 public:
  XImage(){};
  XImage(int num_channels);
  XImage(int num_rows, int num_cols, int num_channels);
  ImageType_1& at(int channel);
  const ImageType_1& at(int channel) const;
  int cols() const; 
  int rows() const;
  int channels() const;
  bool setZero();
  bool read(const std::string& filename);
  bool write(const std::string& filename);
 private:
  ImageType_N nd_array; 
};

} // namespace xform

#endif // SRC_X_IMAGE
