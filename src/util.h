// All the type definition
// Image class definition (base on Eigen and matrix)
#ifndef UTIL_UTIL_H
#define UTIL_UTIL_H

#include <Eigen/Dense>
using namespace std; 

namespace xform{

//typedef double PixelType;
typedef float PixelType;
typedef Eigen::Matrix<PixelType,Eigen::Dynamic, Eigen::Dynamic> MatType;
typedef Eigen::Matrix<PixelType,Eigen::Dynamic, 1> VecType;
typedef Eigen::Matrix<PixelType, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> ImageType_1;
typedef Eigen::Matrix<ImageType_1, 3, 1> ImageType_3; 
typedef Eigen::Matrix<ImageType_1, Eigen::Dynamic, 1> ImageType_N; 
typedef Eigen::Matrix<PixelType, 3, 3> ColorMatType;  
typedef Eigen::Matrix<PixelType, Eigen::Dynamic, 1> KernelType_1D; 
typedef Eigen::Matrix<PixelType, Eigen::Dynamic, Eigen::Dynamic> KernelType_2D; 
typedef Eigen::Map<ImageType_1> MapImageType_1;


const PixelType PIX_UPPER_BOUND = 1.0f;
const PixelType PIX_LOWER_BOUND = 0.0f;
const PixelType PNG_RANGE = 255.0f;


// Image in range of [0,1]
bool imread(const string& filename, ImageType_3* image);
bool imread(const string& filename, ImageType_1* image);

// Input image in range of [0, 1], otherwise truncate the image
bool imwrite(const ImageType_3& image, const string& filename);
bool imwrite(const ImageType_1& image, const string& filename);

bool write_jpeg(const ImageType_3& image, const string& filename, int quality = 75);
bool read_jpeg(const string& filename, ImageType_3* image);

} // namespace xform

#endif //UTIL_UTIL_H 
