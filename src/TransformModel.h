#include "XImage.h"

#include "static_image.h"

#ifndef TRANSFORM_MODEL_H
#define TRANSFORM_MODEL_H

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

  void fit_recipe_by_Halide(const Image<float>& HL_input,
                            const Image<float>& HL_output,
                            ImageType_1* ac, 
                            Image<float>* HL_lp_output, PixelType* meta) const;

  // Eigen-implemented
  void fit_recipe(const XImage& input, const XImage& target, 
  ImageType_1* ac, XImage* dc, PixelType* meta) const;

  XImage reconstruct(const XImage& input, ImageType_1& ac, 
                           XImage& dc, const PixelType* meta) const;

  bool use_halide;
  int step;
  int wSize;
  int quantize_levels;
  PixelType epsilon;

 private:

  void regression_fit(const XImage& input_feat, 
  const XImage& target_feat, ImageType_1* ac) const;

  void regression_predict(const XImage& input_feat, 
  const ImageType_1& ac, const int n_chan_i, const int n_chan_o,  
                                      XImage* reconstructed) const;
  void dequantize(const PixelType* meta, 
   int n_chan_i, int n_chan_o, ImageType_1* ac) const;

  void quantize(int n_chan_i, int n_chan_o, 
            ImageType_1* ac, PixelType* meta) const;

  void get_coefficients(const ImageType_1& ac, 
                              int i, int j, MatType* coef) const;

  void set_coefficients(const MatType& coef, 
                              int i, int j, ImageType_1* ac) const; 
  void two_scale_decomposition(const XImage& input, 
                                    XImage* hp_input, XImage* lp_input) const;
};
} // namespace xform
#endif // TRANSFORM_MODEL_H
