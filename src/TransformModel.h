#ifndef TRANSFORM_MODEL_HPP_6HXUVNLU
#define TRANSFORM_MODEL_HPP_6HXUVNLU

#include "XImage.h"
#include "Recipe.h"

namespace xform{

class TransformModel
{
public:
    TransformModel();
    virtual ~TransformModel();

    void set_images(const XImage &input, const XImage &output);

    void process();
    XImage reconstruct();

private:
    // Input data
    const XImage *input;
    const XImage *output;

    Recipe *recipe;

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

    // Helpers
    void check_fit_io();
};

} // namespace xform

#endif /* end of include guard: TRANSFORM_MODEL_HPP_6HXUVNLU */
