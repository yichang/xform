#ifndef TRANSFORM_MODEL_H
#define TRANSFORM_MODEL_H

#include "XImage.h"
#include "Recipe.h"

#include "static_image.h"

namespace xform{

class TransformModel{
public:
    TransformModel();

   // Halide 
   void reconstruct_by_Halide(const Image<float>& input, 
                               const Image<float>& ac,
                               const Image<float>& dc, 
                               const PixelType* meta,
                               Image<float>* output) const;

    void fit_recipe_by_Halide(const Image<float>& input,
                              const Image<float>& target) const;

    // Eigen implemented
    void set_from_recipe(const XImage& input, ImageType_1& ac, 
                               XImage& dc, const PixelType* meta);
    void fit_recipe(const XImage& input, const XImage& target);

    XImage reconstruct(const XImage& input, ImageType_1& ac, 
                               XImage& dc, const PixelType* meta);
    Recipe *recipe;

    bool use_halide;
    int step;
private:
    // Input data
    const XImage *input;
    const XImage *output;

    // Parameters
    int wSize;
    PixelType epsilon;

    int height;
    int width;
    int n_chan_o;
    int n_chan_i;
    int mdl_h;
    int mdl_w;
};

} // namespace xform
#endif // TRANSFORM_MODEL_H
