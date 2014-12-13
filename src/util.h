// All the type definition
// Image class definition (base on Eigen and matrix)
#ifndef UTIL_UTIL_H
#define UTIL_UTIL_H

#include <Eigen/Dense>
using namespace std; 

typedef double PixelType;
typedef Eigen::Matrix<PixelType, Eigen::Dynamic, Eigen::Dynamic> ImageType_1;
typedef Eigen::Matrix<ImageType_1, 3, 1> ImageType_3; 
typedef Eigen::Matrix<ImageType_1, Eigen::Dynamic, 1> ImageType_N; 
typedef Eigen::Matrix<PixelType, 3, 3> ColorMatType;  

// Image in range of [0,1]
bool imread(const string& filename, ImageType_3* image);

// Input image in range of [0, 1], otherwise truncate the image
bool imwrite(const ImageType_3& image, const string& filename);

#endif //UTIL_UTIL_H 
