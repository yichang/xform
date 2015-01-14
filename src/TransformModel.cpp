#include <vector>
#include "TransformModel.h"
#include "Warp.h"

// HALIDE
#include "halide_resize.h"
#include "halide_recon.h"
#include "halide_dequant.h"
#include "static_image.h"

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
void TransformModel::fit_recipe_by_Halide(const Image<float>& input,
                                          const Image<float>& output) const{

  // Lowpass
  Image<float> lp_input(input.width()/wSize, input.height()/wSize, 3),
               lp_output(input.width()/wSize, input.height()/wSize, 3);

  halide_resize(input,  lp_input.height(),  lp_input.width(),  lp_input); 
  halide_resize(output, lp_output.height(), lp_output.width(), lp_output); 
  // High-Pass
  // TODO: to be done
}
void TransformModel::fit_recipe(const XImage& input, const XImage& target, 
  ImageType_1* ac, XImage* dc, vector<PixelType>* meta) const{

    assert(input.cols()==target.cols());
    assert(input.rows()==target.rows());
    const int height = input.rows();
    const int width = input.cols();

    // Highpass
    XImage hp_input(height, width, 3), 
           hp_output(height, width, 3),
           lp_output(height/wSize, width/wSize, 3);

    if (!use_halide){
    // Lowpass
      XImage lp_input; 
      Warp warp;
      warp.imresize(input, height/wSize, width/wSize,Warp::BICUBIC, &lp_input);
      warp.imresize(target, height/wSize, width/wSize,Warp::BICUBIC, &lp_output);

      warp.imresize(lp_input, height,width,Warp::BICUBIC, &hp_input);
      warp.imresize(lp_output, height,width,Warp::BICUBIC, &hp_output);
    } else { //use_halide
      Image<float> HL_lp_input(width/wSize, height/wSize, input.channels()),
                   HL_lp_output(width/wSize, height/wSize, input.channels()),
                   HL_hp_input(width, height, input.channels()),
                   HL_hp_output(width, height, input.channels()),
                   HL_input(width, height, input.channels()),
                   HL_output(width, height, input.channels());

      assert(width==input.cols());
      assert(height==input.rows());

      input.to_Halide(&HL_input);
      target.to_Halide(&HL_output);

      halide_resize(HL_input, HL_lp_input.height(), HL_lp_input.width(), HL_lp_input);
      halide_resize(HL_output, HL_lp_output.height(), HL_lp_output.width(), HL_lp_output);

      halide_resize(HL_lp_input, HL_hp_input.height(), HL_hp_input.width(), HL_hp_input);
      halide_resize(HL_lp_output, HL_hp_output.height(), HL_hp_output.width(), HL_hp_output);

      hp_input.from_Halide(HL_hp_input);
      hp_output.from_Halide(HL_hp_output);
      lp_output.from_Halide(HL_lp_output);
    }

    hp_input = input - hp_input;
    hp_output = target - hp_output;

    const int mdl_h = ceil(1.0f * height/step);
    const int mdl_w = ceil(1.0f * width/step);

    *dc = lp_output;
    *ac = ImageType_1(mdl_h * input.channels(), mdl_w * target.channels()); 

    // Fit affine model per patch
    for (int imin = 0; imin < height; imin += step)
    for (int jmin = 0; jmin < width; jmin += step)
    {
        int mdl_i = imin/step;
        int mdl_j = jmin/step;
        int h = min(imin+wSize,height) - imin;
        int w = min(jmin+wSize,width) - jmin;

        // Extract patches
        MatType p_input(h*w, input.channels());
        MatType p_output(h*w, target.channels());
        for(int c = 0; c < input.channels() ; ++c)
        {
            MatType tmp = hp_input.at(c).block(imin,jmin,h,w);
            tmp.resize(h*w,1);
            p_input.col(c) = tmp;
        }
        for(int c = 0; c < target.channels() ; ++c)
        {
            MatType tmp = hp_output.at(c).block(imin,jmin,h,w);
            tmp.resize(h*w,1);
            p_output.col(c) = tmp;
        }
        // Solve linear system
        MatType lhs = p_input.transpose() * p_input 
          + epsilon*MatType::Identity(input.channels(), input.channels());
        MatType rhs = p_input.transpose() * p_output;
        MatType coef = lhs.ldlt().solve(rhs);
        //recipe->set_coefficients(mdl_i, mdl_j, coef); 
        set_coefficients(coef, mdl_i, mdl_j, ac);
    }
    
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

    const int height = input.rows();
    const int width = input.cols();
    const int n_chan_o = 3;
    const int n_chan_i = 3;

    // Lowpass
    XImage reconstructed(height, width, n_chan_o);
    if (!use_halide){
      
      dequantize(meta, n_chan_i, n_chan_o, &ac);

      XImage lp_input;
      Warp warp;
      warp.imresize(input, height/wSize, width/wSize,Warp::BICUBIC, &lp_input);

      // Highpass
      XImage hp_input;
      warp.imresize(lp_input, height,width,Warp::BICUBIC, &hp_input);
      hp_input = input - hp_input;

      XImage new_dc;
      warp.imresize(dc, height, width, Warp::BICUBIC, &new_dc);

    // Reconstruct each patch
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
              MatType tmp = hp_input.at(c).block(imin,jmin,h,w);
              tmp.resize(h*w,1);
              p_input.col(c) = tmp;
          }

          MatType coef(n_chan_i,n_chan_o);
          get_coefficients(ac, mdl_i, mdl_j, &coef); 
          MatType p_reconstructed = p_input * coef;

          for(int c = 0; c < n_chan_o ; ++c){
              MatType tmp = p_reconstructed.col(c);
              tmp.resize(h,w);
              reconstructed.at(c).block(imin,jmin,h,w) = tmp;
          }
      }
      reconstructed = reconstructed + new_dc ;
    } else { // use_halide
      XImage foo_ac(1);
      foo_ac.at(0) = ac;
      Image<float> HL_input(input.cols(), input.rows(), input.channels()),
                   HL_ac(foo_ac.cols(), foo_ac.rows(), foo_ac.channels()),
                   HL_dc(dc.cols(), dc.rows(), dc.channels()),
                   HL_recon(input.cols(), input.rows(), input.channels());

      input.to_Halide(&HL_input);
      foo_ac.to_Halide(&HL_ac);
      dc.to_Halide(&HL_dc);

      /* Dequantize */
      Image<float> mins(3,3), maxs(3,3);
      for(int y= 0; y < 3; y++){
        for(int x = 0; x < 3; x++){ 
          mins(x,y) = meta[3*y + x];
          maxs(x,y) = meta[3*y + x + n_chan_i * n_chan_o];
        }}
      const int nchannels=3;
      halide_dequant(HL_ac, mins, maxs, nchannels, HL_ac);

      /* Low pass */
      Image<float> HL_ds(width/wSize, height/wSize, 3), 
                   HL_low_pass(width, height, 3),
                   HL_new_dc(width, height, 3);
      halide_resize(HL_input, HL_ds.height(), HL_ds.width(), HL_ds);
      halide_resize(HL_ds, HL_low_pass.height(), HL_low_pass.width(), HL_low_pass);
      halide_resize(HL_dc, HL_new_dc.height(), HL_new_dc.width(), HL_new_dc);

      /* Patch-based */
      halide_recon(HL_input, HL_low_pass, HL_ac, HL_new_dc, step, HL_recon); 
      reconstructed.from_Halide(HL_recon);
    }
  return reconstructed;
}

// Helper functions 
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
            ImageType_1* ac, vector<PixelType>* meta) const{

  const int num_chan = n_chan_i * n_chan_o;
  const int height = ac->rows()/n_chan_i, width = ac->cols()/n_chan_o;
  *meta = vector<PixelType>(2 * num_chan);  

  for(int in_chan = 0; in_chan < n_chan_i; ++in_chan)
    for(int out_chan = 0; out_chan < n_chan_o; ++out_chan)
    {

        PixelType mini = ac->block(in_chan*height, out_chan*width, 
                                                  height, width).minCoeff();
        PixelType maxi = ac->block(in_chan*height, out_chan*width,
                                                  height, width).maxCoeff();
        meta->at(in_chan*n_chan_o + out_chan) = mini;
        meta->at(in_chan*n_chan_o + out_chan + num_chan) = maxi;
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
