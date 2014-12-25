#ifndef RECIPE_H_STZIT3SX
#define RECIPE_H_STZIT3SX

#include "util.h"
#include "XImage.h"

namespace xform{

class Recipe
{
public:
    Recipe(){};
    Recipe(int num_rows, int num_cols, int n_chan_i, int n_chan_o);
    virtual ~Recipe();

    bool read(const std::string& filename);
    bool write(const std::string& filename);

    void set_dc(XImage& src);
    XImage& get_dc();
    void set_coefficients(int i, int j, const MatType &coef);
    void get_coefficients(int i, int j, MatType &coef);

    void quantize();
    void dequantize();

private:
    int height,width,n_chan_i,n_chan_o;
    int quantize_levels;
    PixelType* quantize_mins;
    PixelType* quantize_maxs;
    XImage dc;
    ImageType_1 ac; 
};

} // namespace xform


#endif /* end of include guard: RECIPE_H_STZIT3SX */

