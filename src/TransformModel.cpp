#include "TransformModel.h"
#include "ColorSpace.h"
#include "Pyramid.h"
#include "Warp.h"

// HALIDE
#include "halide_resize.h"
#include "halide_recon.h"
#include "halide_recon_separate.h"
#include "halide_dequant.h"
#include "static_image.h"
#include "halide_downsample.h"
#include "halide_highpass.h"
#include "halide_compute_features.h"
#include "halide_compute_cfeatures.h"

#ifndef __ANDROID__
#include "image_io.h"
#endif

using namespace std;

namespace xform{

TransformModel::TransformModel(){
    use_halide=true;
    wSize   = 16;
    step    = wSize;
    epsilon = 1e-2;
    epsilon *= epsilon;
    quantize_levels = 255; 
    num_scale = 5;
    num_affine = 4;
    num_linear = 3;
    num_bins = 4;
}
#ifndef __ANDROID__
void TransformModel::fit_separate_recipe_by_Halide(const Image<float>& HL_input,
                                          const Image<float>& HL_output,
                              ImageType_1* ac_lumin, ImageType_1* ac_chrom, 
                        Image<float>* HL_lp_output, PixelType* meta) const{
  const int lp_width = HL_lp_output->width(), 
            lp_height= HL_lp_output->height(),
            width = HL_input.width(),
            height = HL_input.height(),
            num_lumin_feat = num_affine + num_bins + num_scale - 2,
            num_chrom_feat = num_affine;
  Image<float> HL_lp_input(lp_width, lp_height, 3),
               HL_hp_output(width, height, 3);
  assert(use_halide);

  // Compute target features (HL_hp_output);
  halide_downsample(HL_output, *HL_lp_output);
  halide_highpass(HL_output, *HL_lp_output, HL_hp_output);
  // Lumin and chrom featues
  Image<float> HL_feat_lumin(width, height, num_lumin_feat),
               HL_feat_chrom(width, height, num_chrom_feat);
  halide_compute_features(HL_input, HL_feat_lumin);
  halide_compute_cfeatures(HL_input, HL_feat_chrom);
  // Fitting
  XImage feat_lumin(height, width, num_lumin_feat), 
         feat_chrom(height, width, num_chrom_feat),
         hp_output(height, width, 3), 
         hp_target_y(height, width, 1),
         hp_target_uv(height, width, 2);
  feat_lumin.from_Halide(HL_feat_lumin);
  feat_chrom.from_Halide(HL_feat_chrom);
  hp_output.from_Halide(HL_hp_output);
  
  hp_target_y.at(0)  = hp_output.at(0);
  hp_target_uv.at(0) = hp_output.at(1);
  hp_target_uv.at(1) = hp_output.at(2);
  
  regression_fit(feat_lumin, hp_target_y, ac_lumin);
  regression_fit(feat_chrom, hp_target_uv, ac_chrom);
  
  quantize(feat_lumin.channels(), hp_target_y.channels(),  ac_lumin, meta);
  quantize(feat_chrom.channels(), hp_target_uv.channels(), 
    ac_chrom, meta + 2 * feat_lumin.channels() * hp_target_y.channels());
}
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
  regression_fit(hp_input, hp_output, ac);
  quantize(HL_input.channels(), HL_output.channels(), ac, meta);
}
void TransformModel::fit_separate_recipe(const XImage& input, 
                                         const XImage& target, 
          ImageType_1* ac_lumin, ImageType_1* ac_chrom, 
                            XImage* dc, PixelType* meta) const{
    // Order of meta: lumin_min lumin_max chrom_min chrom_max
    assert(input.cols()==target.cols());
    assert(input.rows()==target.rows());
    assert(!use_halide);

    // RGB2YUV
    XImage input_yuv, target_yuv;
    ColorSpace cs; 
    cs.rgb2yuv(input, &input_yuv);
    cs.rgb2yuv(target, &target_yuv);

   // Target features
    XImage hp_target_yuv, lp_target_yuv;
    two_scale_decomposition(target_yuv, &hp_target_yuv, &lp_target_yuv);
    cs.yuv2rgb(lp_target_yuv, dc);

   //Input features
    XImage feat_chrom, feat_lumin;
    make_chrom_features(input_yuv, &feat_chrom);
    make_lumin_features(input_yuv, &feat_lumin);

    // Build y and uv target feat
    XImage hp_target_y(1), hp_target_uv(2);
    hp_target_y.at(0) = hp_target_yuv.at(0);
    hp_target_uv.at(0) = hp_target_yuv.at(1);
    hp_target_uv.at(1) = hp_target_yuv.at(2);

    // Fit lumin and chrom channels
    regression_fit(feat_lumin, hp_target_y, ac_lumin);
    regression_fit(feat_chrom, hp_target_uv, ac_chrom);
    
    quantize(feat_lumin.channels(), hp_target_y.channels(),  ac_lumin, meta);
    quantize(feat_chrom.channels(), hp_target_uv.channels(), 
      ac_chrom, meta + 2 * feat_lumin.channels() * hp_target_y.channels());
}
void TransformModel::fit_recipe(const XImage& input, const XImage& target, 
  ImageType_1* ac, XImage* dc, PixelType* meta) const{

    assert(input.cols()==target.cols());
    assert(input.rows()==target.rows());
    assert(!use_halide);

    // RGB2YUV
    XImage input_yuv, target_yuv;
    ColorSpace cs; 
    cs.rgb2yuv(input, &input_yuv);
    cs.rgb2yuv(target, &target_yuv);

    // Target features
    XImage hp_target_yuv, lp_target_yuv;
    two_scale_decomposition(target_yuv, &hp_target_yuv, &lp_target_yuv);
    cs.yuv2rgb(lp_target_yuv, dc);

    // Input features
    XImage hp_input_yuv, foo; 
    two_scale_decomposition(input_yuv, &hp_input_yuv, &foo);

    // Fitting
    regression_fit(hp_input_yuv, hp_target_yuv, ac);
    quantize(input.channels(), target.channels(), ac, meta);
}
#endif
void TransformModel::make_chrom_features(const XImage& input, 
                                               XImage* feat) const{
  *feat = XImage(input.rows(), input.cols(), input.channels()+1);     
  feat->setOnes();
  XImage hp_input, foo;
  two_scale_decomposition(input, &hp_input, &foo);
  for(int i=0; i < hp_input.channels(); i++)
    feat->at(i) = hp_input.at(i);
}
void TransformModel::make_lumin_features(const XImage& input, 
                                               XImage* feat) const{
  *feat = XImage(input.rows(), input.cols(), input.channels() + num_scale + num_bins-1);
  feat->setOnes();
  XImage hp_input, foo;
  two_scale_decomposition(input, &hp_input, &foo);
  for(int i = 0; i < hp_input.channels(); i++)
    feat->at(i) = hp_input.at(i);

  if (num_scale > 1){
    Pyramid pyramid_y(input.at(0), num_scale, Pyramid::LAPLACIAN, true);
    for(int i = 0; i < pyramid_y.levels()-1; i++) // ignore lowpass
      feat->at(i + num_affine) = pyramid_y.at(i);
  }

  if (num_bins > 1){
    XImage curve_feat;
    lumin_curve_feature(hp_input.at(0), num_bins, &curve_feat);
    for(int i = 0; i < curve_feat.channels(); i++)
      feat->at(i + num_affine + num_scale-1) = curve_feat.at(i);
  }
}
void TransformModel::lumin_curve_feature(const ImageType_1& input, 
  const int nbins, XImage* feat) const{
  PixelType max = input.maxCoeff(), min = input.minCoeff();
  PixelType range = max - min;
  *feat = XImage(input.rows(), input.cols(), nbins-1);
  feat->setZero();
  for(int c = 0; c < feat->channels(); c++){
    const float thresh = min + static_cast<float>(c+1) * range/static_cast<float>(nbins);
    for(int i = 0; i < feat->rows(); i ++){
      for(int j = 0; j < feat->cols(); j++){
        if(input(i, j) > thresh)
          feat->at(c)(i, j) = input(i, j) - thresh;
      }
    }
  }
}
void TransformModel::reconstruct_separate_by_Halide(const Image<float>& input, 
                               const Image<float>& ac_lumin_raw,
                               const Image<float>& ac_chrom_raw,
                               const Image<float>& dc, 
                               const PixelType* meta,
                               Image<float>* output) const{
  /* Usage: TransformModel(); TransformMode(....); */
  assert(use_halide);
  const int num_lumin_feat = num_affine + num_bins - 1 + num_scale -1;
  const int num_chrom_feat = num_affine;
  Image<float> ac_lumin_mins(1, num_lumin_feat), 
              ac_lumin_maxs(1, num_lumin_feat);
  for(int y= 0; y < ac_lumin_mins.height(); y++){
    for(int x = 0; x < ac_lumin_mins.width(); x++){ 
      ac_lumin_mins(x,y) = meta[y];
      ac_lumin_maxs(x,y) = meta[y + num_lumin_feat];
      //cout << "min "<<x<<","<<y<<"="<<ac_lumin_mins(x,y)<<endl;
      //cout << "max "<<x<<","<<y<<"="<<ac_lumin_maxs(x,y)<<endl;
    }}
  Image<float> ac_chrom_mins(2, num_chrom_feat), 
               ac_chrom_maxs(2, num_chrom_feat);
  for(int y= 0; y < ac_chrom_mins.height(); y++){
    for(int x = 0; x < ac_chrom_mins.width(); x++){ 
      ac_chrom_mins(x,y) = meta[2*y + x + 2 * num_lumin_feat];
      ac_chrom_maxs(x,y) = meta[2*y + x + 2 * num_chrom_feat + 2*num_lumin_feat];
      //cout << "min "<<x<<","<<y<<"="<<ac_chrom_mins(x,y)<<endl;
      //cout << "max "<<x<<","<<y<<"="<<ac_chrom_maxs(x,y)<<endl;
    }}
  //Image<float> output(input.width(), input.height(),input.channels());

  halide_recon_separate(input, 
            ac_lumin_raw, ac_lumin_mins, ac_lumin_maxs,
            ac_chrom_raw, ac_chrom_mins, ac_chrom_maxs,
            dc ,
            *output);
}
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
    const int n_chan_o = num_linear, n_chan_i = num_linear;

    dequantize(meta, n_chan_i, n_chan_o, &ac);

    // RGB2YUV
    ColorSpace cs;
    XImage input_yuv;
    cs.rgb2yuv(input, &input_yuv);

    // Two-scale decomposition
    XImage hp_input_yuv, foo; 
    two_scale_decomposition(input_yuv, &hp_input_yuv,&foo); 

     // Reconstruction
    XImage reconstructed_yuv(height, width, n_chan_o), reconstructed;
    regression_predict(hp_input_yuv, ac, n_chan_i, n_chan_o, &reconstructed_yuv);
    cs.yuv2rgb(reconstructed_yuv, &reconstructed);

    // Add DC back
    XImage new_dc;
    Warp warp;
    warp.imresize(dc, height, width, Warp::BICUBIC, &new_dc);
    reconstructed = reconstructed + new_dc ;
    return reconstructed;
}
XImage TransformModel::reconstruct_separate(const XImage& input, 
                              ImageType_1& ac_lumin, 
                              ImageType_1& ac_chrom,
                              XImage& dc, const PixelType* meta) const{
    assert(!use_halide);
    const int height = input.rows(), width = input.cols();
    const int n_chan_i_lumin = num_affine + num_bins + num_scale - 2;
    const int n_chan_i_chrom = num_affine;

    dequantize(meta,                      n_chan_i_lumin, 1, &ac_lumin);
    dequantize(meta + 2 * n_chan_i_lumin, n_chan_i_chrom, 2, &ac_chrom);

    // RGB2YUV
    ColorSpace cs;
    XImage input_yuv;
    cs.rgb2yuv(input, &input_yuv);

    // Make features 
    XImage feat_lumin, feat_chrom; 
    make_chrom_features(input_yuv, &feat_chrom);
    make_lumin_features(input_yuv, &feat_lumin);

    // Reconstruction
    XImage reconstructed_y(height, width,  1), 
           reconstructed_uv(height, width, 2), 
           reconstructed_yuv(3),
           reconstructed;
    regression_predict(feat_lumin, ac_lumin, n_chan_i_lumin, 1, &reconstructed_y);
    regression_predict(feat_lumin, ac_chrom, n_chan_i_chrom, 2, &reconstructed_uv);
    reconstructed_yuv.at(0) = reconstructed_y.at(0);
    reconstructed_yuv.at(1) = reconstructed_uv.at(0);
    reconstructed_yuv.at(2) = reconstructed_uv.at(1);

    cs.yuv2rgb(reconstructed_yuv, &reconstructed);

    // Add DC back
    XImage new_dc;
    Warp warp;
    warp.imresize(dc, height, width, Warp::BICUBIC, &new_dc);
    reconstructed = reconstructed + new_dc ;
    return reconstructed;
}

// Helper functions 
void TransformModel::two_scale_decomposition(const XImage& input, 
                                    XImage* hp_input, XImage* lp_input) const{

    const int height = input.rows(), width = input.cols();
    Warp warp;
    warp.imresize(input, height/wSize, width/wSize, Warp::BICUBIC, lp_input);
    warp.imresize(*lp_input, height, width,Warp::BICUBIC, hp_input);
    *hp_input = input - *hp_input;
}
void TransformModel::regression_fit(const XImage& input_feat, 
  const XImage& target_feat, ImageType_1* ac) const{

  const int height = input_feat.rows(), width = input_feat.cols(); 
  const int mdl_h = ceil(1.0f * height/step), mdl_w = ceil(1.0f * width/step);
  *ac = ImageType_1(mdl_h * input_feat.channels(), mdl_w * target_feat.channels()); 

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
        //std::cout<<"min " << out_chan << "," << in_chan << "=" << mini <<endl;
        //std::cout<<"max " << out_chan << "," << in_chan << "=" << maxi <<endl;
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
