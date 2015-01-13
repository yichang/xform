#ifndef TRANSFORM_MODEL_HPP_6HXUVNLU
#define TRANSFORM_MODEL_HPP_6HXUVNLU

#include "XImage.h"
#include "Recipe.h"

#include "static_image.h"

namespace xform{

class TransformModel
{
public:
    TransformModel();
    virtual ~TransformModel();

   // Halide 
   void reconstruct_by_Halide(const Image<float>& input, 
                               const Image<float>& ac,
                               const Image<float>& dc, 
                               const PixelType* meta,
                               Image<float>* output) const;

  void fit_recipe_by_Halide(const Image<float>& input,
                              const Image<float>& target) const;

    // Eigen implemented
    void set_images(const XImage &input, const XImage &output);
    void set_recipe(Recipe* recipe);
    void set_from_recipe(const XImage& input, ImageType_1& ac, 
                               XImage& dc, const PixelType* meta);
    void fit();
    XImage predict();
    Recipe *recipe;

    int get_step() const;
    bool use_halide;
private:
    // Input data
    const XImage *input;
    const XImage *output;


    // Parameters
    int wSize;
    int step;
    PixelType epsilon;

    int height;
    int width;
    int n_chan_o;
    int n_chan_i;
    int mdl_h;
    int mdl_w;
    
    // Pipeline operations
    void fit_recipe();
    XImage reconstruct();

    // Helpers
    void check_fit_io();
};

} // namespace xform

#endif /* end of include guard: TRANSFORM_MODEL_HPP_6HXUVNLU */
