#include "util.h"
#include "XImage.h"

#ifndef RECIPE_H
#define RECIPE_H

namespace xform{

class Recipe{
 public:
    Recipe(){};
    Recipe(int num_rows, int num_cols, int n_chan_i, int n_chan_o);

    bool read(const std::string& filename);
    bool write(const std::string& filename);

    void set_dc(XImage& src);
    void set_ac(ImageType_1& src);
    void set_quantize_mins(const PixelType* in, int len);
    void set_quantize_maxs(const PixelType* in, int len);

    XImage& get_dc();
    void set_coefficients(int i, int j, const MatType &coef);
    void get_coefficients(int i, int j, MatType &coef);

    void quantize();
    void dequantize();

    int height,width,n_chan_i,n_chan_o;
    int quantize_levels;
    PixelType* quantize_mins;
    PixelType* quantize_maxs;
    XImage dc;
    ImageType_1 ac; 
private:
};

} // namespace xform
#endif // RECIPE_H
