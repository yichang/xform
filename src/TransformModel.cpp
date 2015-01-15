#include "TransformModel.h"
#include "Warp.h"
#include "ColorSpace.h"

// HALIDE
#include "halide_resize.h"
#include "halide_recon.h"
#include "halide_dequant.h"
#include "static_image.h"
#include "image_io.h"

using namespace std;

namespace xform{

TransformModel::TransformModel(){
    use_halide=true;
    wSize   = 8;
    step    = wSize/2;
    epsilon = 1e-2;
    epsilon *= epsilon;
    quantize_levels = 255; 
}

#ifndef __ANDROID__
void TransformModel::fit_recipe_by_Halide(const Image<float>& HL_input,
                                          const Image<float>& HL_output,
          ImageType_1* ac, Image<float>* HL_lp_output, PixelType* meta) const{

  assert(use_halide);

  // Two-scale decomposition
  const int width = HL_input.width(), height = HL_input.height(); 
  Image<float> HL_lp_input(width/wSize, height/wSize, HL_input.channels()),
               HL_hp_input(width, height, HL_input.channels()),
               HL_hp_output(width, height, HL_input.channels());
  *HL_lp_output = Image<float>(width/wSize, height/wSize, HL_input.channels());

  halide_resize(HL_input, HL_lp_input.height(), HL_lp_input.width(), HL_lp_input);
  halide_resize(HL_lp_input, HL_hp_input.height(), HL_hp_input.width(), HL_hp_input);

  halide_resize(HL_output, HL_lp_output->height(), HL_lp_output->width(), *HL_lp_output);
  halide_resize(*HL_lp_output, HL_hp_output.height(), HL_hp_output.width(), HL_hp_output);

  XImage hp_input(height, width, 3), 
         hp_output(height, width, 3),
         lp_output(height/wSize, width/wSize, 3),
         input(height, width, 3),
         output(height, width, 3);
  hp_input.from_Halide(HL_hp_input);
  hp_output.from_Halide(HL_hp_output);
  lp_output.from_Halide(*HL_lp_output);
  input.from_Halide(HL_input);
  output.from_Halide(HL_output);
  hp_input = input - hp_input;
  hp_output = output - hp_output;

  // Regression
  const int mdl_h = ceil(1.0f * height/step);
  const int mdl_w = ceil(1.0f * width/step);
  *ac = ImageType_1(mdl_h * HL_input.channels(), mdl_w * HL_output.channels()); 
  regression_fit(hp_input, hp_output, ac);
  quantize(HL_input.channels(), HL_output.channels(), ac, meta);
}
void TransformModel::fit_recipe(const XImage& input, const XImage& target, 
  ImageType_1* ac, XImage* dc, PixelType* meta) const{

    assert(input.cols()==target.cols());
    assert(input.rows()==target.rows());
    assert(!use_halide);
    const int height = input.rows(), width = input.cols();

    // RGB2YUV
    XImage input_yuv, target_yuv;
    ColorSpace cs; 
    cs.rgb2yuv(input, &input_yuv);
    cs.rgb2yuv(target, &target_yuv);
    input_yuv = input;
    target_yuv = target;

    // Two-scale decomposition
    XImage hp_input(height, width, 3), 
           hp_output(height, width, 3),
           lp_output(height/wSize, width/wSize, 3);
    
    XImage lp_input; 
    Warp warp;
    warp.imresize(input_yuv, height/wSize, width/wSize,Warp::BICUBIC, &lp_input);
    warp.imresize(target_yuv, height/wSize, width/wSize,Warp::BICUBIC, &lp_output);

    warp.imresize(lp_input, height,width,Warp::BICUBIC, &hp_input);
    warp.imresize(lp_output, height,width,Warp::BICUBIC, &hp_output);

    hp_input = input_yuv - hp_input;
    hp_output = target_yuv - hp_output;

    // Fitting
    const int mdl_h = ceil(1.0f * height/step);
    const int mdl_w = ceil(1.0f * width/step);

    *dc = lp_output;
    *ac = ImageType_1(mdl_h * input_yuv.channels(), mdl_w * target.channels()); 
    regression_fit(hp_input, hp_output, ac);
    quantize(input.channels(), target.channels(), ac, meta);
}
#endif
void TransformModel::reconstruct_by_Halide(const Image<float>& HL_input, 
                               const Image<float>& HL_ac,
                               const Image<float>& HL_dc, 
                               const PixelType* meta,
                               Image<float>* HL_output) const{

      /* Usage: TransformModel(); TransformMode(....); */
      /* Dequantize */
      Image<float> mins(3,3), maxs(3,3);
      for(int y= 0; y < 3; y++){
        for(int x = 0; x < 3; x++){ 
          mins(x,y) = meta[3*y + x];
          maxs(x,y) = meta[3*y + x + 9];
        }}
      const int nchannels=3;
      halide_dequant(HL_ac, mins, maxs, nchannels, HL_ac);

      /* Low pass */
      const int width = HL_input.width();
      const int height = HL_input.height();
      Image<float> HL_ds(width/wSize, height/wSize, 3), 
                   HL_low_pass(width, height, 3),
                   HL_new_dc(width, height, 3);
      halide_resize(HL_input, HL_ds.height(), HL_ds.width(), HL_ds);
      halide_resize(HL_ds, HL_low_pass.height(), HL_low_pass.width(), HL_low_pass);
      halide_resize(HL_dc, HL_new_dc.height(), HL_new_dc.width(), HL_new_dc);

      /* Patch-based */
      halide_recon(HL_input, HL_low_pass, HL_ac, HL_new_dc, step, *HL_output); 
}
XImage TransformModel::reconstruct(const XImage& input, ImageType_1& ac, 
                              XImage& dc, const PixelType* meta) const{
    assert(!use_halide);
    const int height = input.rows(), width = input.cols();
    const int n_chan_o = 3, n_chan_i = 3;

    dequantize(meta, n_chan_i, n_chan_o, &ac);

    // Two-scale decomposition
    XImage lp_input, hp_input, new_dc;
    Warp warp;
    warp.imresize(input, height/wSize, width/wSize,Warp::BICUBIC, &lp_input);
    warp.imresize(lp_input, height,width,Warp::BICUBIC, &hp_input);
    hp_input = input - hp_input;

    // Reconstruction
    XImage reconstructed(height, width, n_chan_o), reconstructed_yuv;
    regression_predict(hp_input, ac, n_chan_i, n_chan_o, &reconstructed);

    warp.imresize(dc, height, width, Warp::BICUBIC, &new_dc);
    reconstructed = reconstructed + new_dc ;

    // YUV2RGB
    ColorSpace cs;
    cs.yuv2rgb(reconstructed, &reconstructed_yuv);
    reconstructed_yuv = reconstructed; 

    return reconstructed_yuv;
}
// Helper functions 
void TransformModel::regression_fit(const XImage& input_feat, 
  const XImage& target_feat, ImageType_1* ac) const{
  const int height = input_feat.rows(), width = input_feat.cols(); 
   // Fit affine model per patch
    for (int imin = 0; imin < height; imin += step)
    for (int jmin = 0; jmin < width; jmin += step)
    {
        int mdl_i = imin/step;
        int mdl_j = jmin/step;
        int h = min(imin+wSize,height) - imin;
        int w = min(jmin+wSize,width) - jmin;

        // Extract patches
        MatType p_input(h*w, input_feat.channels());
        MatType p_output(h*w, target_feat.channels());
        for(int c = 0; c < input_feat.channels() ; ++c)
        {
            MatType tmp = input_feat.at(c).block(imin,jmin,h,w);
            tmp.resize(h*w,1);
            p_input.col(c) = tmp;
        }
        for(int c = 0; c < target_feat.channels() ; ++c)
        {
            MatType tmp = target_feat.at(c).block(imin,jmin,h,w);
            tmp.resize(h*w,1);
            p_output.col(c) = tmp;
        }
        // Solve linear system
        MatType lhs = p_input.transpose() * p_input + epsilon*
          MatType::Identity(input_feat.channels(), input_feat.channels());
        MatType rhs = p_input.transpose() * p_output;
        MatType coef = lhs.ldlt().solve(rhs);
        //recipe->set_coefficients(mdl_i, mdl_j, coef); 
        set_coefficients(coef, mdl_i, mdl_j, ac);
    }
}
void TransformModel::regression_predict(const XImage& input_feat, 
  const ImageType_1& ac, const int n_chan_i, const int n_chan_o,  
                                      XImage* reconstructed) const{
    // Reconstruct each patch
    const int height = input_feat.rows(), width = input_feat.cols();

    for (int imin = 0; imin < height; imin += step)
      for (int jmin = 0; jmin < width; jmin += step)
      {
          int mdl_i = imin/step;
          int mdl_j = jmin/step;
          int h = min(imin+step,height) - imin;
          int w = min(jmin+step,width) - jmin;

          // Extract patches
          MatType p_input(h*w, n_chan_i);
          MatType p_recons(h*w, n_chan_o);
          for(int c = 0; c < n_chan_i ; ++c)
          {
              MatType tmp = input_feat.at(c).block(imin,jmin,h,w);
              tmp.resize(h*w,1);
              p_input.col(c) = tmp;
          }

          MatType coef(n_chan_i,n_chan_o);
          get_coefficients(ac, mdl_i, mdl_j, &coef); 
          MatType p_reconstructed = p_input * coef;

          for(int c = 0; c < n_chan_o ; ++c){
              MatType tmp = p_reconstructed.col(c);
              tmp.resize(h,w);
              reconstructed->at(c).block(imin,jmin,h,w) = tmp;
          }
      }
}
void TransformModel::dequantize(const PixelType* meta, 
        int n_chan_i, int n_chan_o, ImageType_1* ac) const{

  const int num_chan = n_chan_i * n_chan_o;
  const int height = ac->rows()/n_chan_i, width = ac->cols()/n_chan_o;

  for(int in_chan = 0; in_chan < n_chan_i; ++in_chan)
  for(int out_chan = 0; out_chan < n_chan_o; ++out_chan)
    {
        PixelType mini = meta[in_chan*n_chan_o + out_chan];
        PixelType maxi = meta[num_chan + in_chan*n_chan_o + out_chan];
        PixelType rng = maxi-mini;
        if(rng == 0){
            rng = 1;
        }
        for(int i = 0; i<height; ++i)
        for(int j = 0; j<width; ++j)
        {
            int ac_map_i = in_chan*height + i;
            int ac_map_j = out_chan*width +j;
            (*ac)(ac_map_i,ac_map_j) *= rng;
            (*ac)(ac_map_i,ac_map_j) += mini;
        }
    }
}
void TransformModel::get_coefficients(const ImageType_1& ac, 
                        int i, int j, MatType* coef) const{

  const int n_chan_i = coef->rows(), n_chan_o = coef->cols();
  const int height = ac.rows()/n_chan_i, width = ac.cols()/n_chan_o;
  for(int in_chan = 0; in_chan < n_chan_i ; ++in_chan)
    for(int out_chan = 0; out_chan < n_chan_o ; ++out_chan)
    {
        int ac_map_i = in_chan*height + i;
        int ac_map_j = out_chan*width +j;
        (*coef)(in_chan,out_chan) = ac(ac_map_i, ac_map_j);
    }
}
void TransformModel::quantize(int n_chan_i, int n_chan_o, 
            ImageType_1* ac, PixelType* meta) const{

  const int num_chan = n_chan_i * n_chan_o;
  const int height = ac->rows()/n_chan_i, width = ac->cols()/n_chan_o;

  for(int in_chan = 0; in_chan < n_chan_i; ++in_chan)
    for(int out_chan = 0; out_chan < n_chan_o; ++out_chan)
    {

        PixelType mini = ac->block(in_chan*height, out_chan*width, 
                                                  height, width).minCoeff();
        PixelType maxi = ac->block(in_chan*height, out_chan*width,
                                                  height, width).maxCoeff();
        meta[in_chan*n_chan_o + out_chan] = mini;
        meta[in_chan*n_chan_o + out_chan + num_chan] = maxi;
        PixelType rng = maxi-mini;
        if(rng == 0){
            rng = 1;
        }
        for(int i = 0; i<height; ++i)
        for(int j = 0; j<width; ++j)
        {
            int ac_map_i = in_chan*height + i;
            int ac_map_j = out_chan*width +j;
            (*ac)(ac_map_i,ac_map_j) -= mini;
            (*ac)(ac_map_i,ac_map_j) /= rng;
            (*ac)(ac_map_i,ac_map_j) = 
                floor((*ac)(ac_map_i,ac_map_j)*quantize_levels)/quantize_levels;
        }
    }
}
void TransformModel::set_coefficients(const MatType& coef, 
                              int i, int j, ImageType_1* ac) const{

  const int n_chan_i = coef.rows(), n_chan_o = coef.cols();
  const int height = ac->rows()/n_chan_i, width = ac->cols()/n_chan_o;

  for(int in_chan = 0; in_chan < n_chan_i; ++in_chan)
    for(int out_chan = 0; out_chan < n_chan_o; ++out_chan)
    {
        int ac_map_i = in_chan*height + i;
        int ac_map_j = out_chan*width +j;
        (*ac)(ac_map_i, ac_map_j) = coef(in_chan,out_chan);
    }
}
} // namespace xform
