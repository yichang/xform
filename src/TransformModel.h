#ifndef TRANSFORM_MODEL_H
#define TRANSFORM_MODEL_H

#include "XImage.h"
#include "Recipe.h"

#include "static_image.h"

namespace xform{

class TransformModel{
 public:
  TransformModel();

  // Halide-implemented 
  void reconstruct_by_Halide(const Image<float>& input, 
                             const Image<float>& ac,
                             const Image<float>& dc, 
                             const PixelType* meta,
                             Image<float>* output) const;

  void fit_recipe_by_Halide(const Image<float>& input,
                            const Image<float>& target) const;
  // Eigen-implemented
  void fit_recipe(const XImage& input, const XImage& target);

  XImage reconstruct(const XImage& input, ImageType_1& ac, 
                           XImage& dc, const PixelType* meta);
  Recipe *recipe;

  bool use_halide;
  int step;
  int wSize;
  PixelType epsilon;
 private:
  void dequantize(const PixelType* meta, ImageType_1* ac) const;
  void get_coefficients(const ImageType_1& ac, 
                              int i, int j, MatType* coef) const;
};
} // namespace xform
#endif // TRANSFORM_MODEL_H
