// All the type definition
// Image class definition (base on Eigen and matrix)
#ifndef UTIL_UTIL_H
#define UTIL_UTIL_H

#include <Eigen/Dense>
using namespace std; 

typedef double PixelType;
typedef Eigen::Matrix<PixelType, Eigen::Dynamic, Eigen::Dynamic> ImageType_1;
typedef Eigen::Matrix<ImageType_1, 3, 1> ImageType_3; 


void readPngToEigen(const string& filename, ImageType_3* image);

#endif //UTIL_UTIL_H 
