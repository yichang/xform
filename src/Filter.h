#include <Eigen/Dense>
#include "XImage.h"

#ifndef SRC_FILTER_H
#define SRC_FILTER_H

namespace xform{

class Filter{
 public:

  enum  BoundaryType{
        SYMMETRIC, 
        REPLICATE, 
        CIRCULAR, 
        ZERO_PAD}; 

  Filter(){};

  // Generic convolution
  void convolve(const XImage& im_in, const KernelType_2D& kernel,
        const BoundaryType boundary_type, XImage* im_out) const; 

  void convolve(const ImageType_1& im_in, const KernelType_2D& kernel,
        const BoundaryType boundary_type, ImageType_1* im_out) const; 

  // Filter by separable kernel in two pass
  void sep_kernel(const XImage& im_in, const KernelType_1D& kernel, 
        const BoundaryType boundary_type, XImage* im_out) const; 
  void sep_kernel(const ImageType_1& im_in, const KernelType_1D& kernel, 
        const BoundaryType boundary_type, ImageType_1* im_out) const;     

  //  Box filters by recursive filter
  void box(const XImage& im_in, const int b_width, XImage* im_out) const;
  void box(const XImage& im_in, const int b_width, 
           const BoundaryType boundary_type, XImage* im_out) const;
  void box(const ImageType_1& im_in, const int b_width, 
    const BoundaryType boundary_type, ImageType_1* im_out) const;
  void recursiveBoxFilterZeroPad(const ImageType_1& im_in, const int b_width, 
                                ImageType_1* im_out) const;
  void recursiveBoxFilterNoPad(const ImageType_1& im_in, const int b_width, 
      const BoundaryType boundary_type, ImageType_1* im_out) const;

  // Box filters by summed area table
  void boxBySumArea(const XImage& im_in, const int b_width, const BoundaryType boundary_type, XImage* im_out) const; 

  void boxBySumArea(const ImageType_1& im_in, const int b_width, const BoundaryType boundary_type, ImageType_1* im_out) const; 

  // Approx. Gaussian by iterative box filter 
  void box_iteration(const XImage& im_in, const int b_width, 
                     const int n_iter, XImage* im_out) const;
  void box_iteration(const ImageType_1& im_in, const int b_width, 
                     const int n_iter, ImageType_1* im_out) const;

 private:
  /* Helpers */
  int reflect(const int val, const int width, 
              const BoundaryType boundary_type) const;
};
} // namespace xform
#endif //SRC_FILTER_H
