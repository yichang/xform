#include "TransformModel.h"
#include "Warp.h"

// TIMING RELATED
#include <sys/time.h>
#include <iostream>


// HALIDE
#include "halide_resize.h"
#include "halide_recon.h"
#include "static_image.h"

using namespace std;

namespace xform{

TransformModel::TransformModel(){
    input  = NULL;
    output = NULL;
    recipe = NULL;
    use_halide=true;

    // Processing parameters
    this->wSize   = 8;
    this->step    = wSize/2;
    this->epsilon = 1e-2;
    this->epsilon *= this->epsilon;
}

TransformModel::~TransformModel() {
    input  = NULL;
    output = NULL;
    if(recipe){
        delete recipe;
        recipe = NULL;
    }
}
int TransformModel::get_step() const{
  return step;
}

void TransformModel::set_images(const XImage &input, const XImage &output) {
    this->input = &input;
    this->output = &output;

    height   = input.rows();
    width    = input.cols();
    n_chan_o = output.channels();
    n_chan_i = input.channels();
    //printf("input size %dx%dx%d\n", height, width,n_chan_i);
    
    // Size of the recipe
    mdl_h = ceil(1.0f*height/step);
    mdl_w = ceil(1.0f*width/step);
}
void TransformModel::set_recipe(Recipe* saved_recipe){
  recipe = saved_recipe;
}

void TransformModel::set_from_recipe(const XImage& input, ImageType_1& ac, 
                                    XImage& dc, const PixelType* meta){
  
  const int width = ceil(static_cast<float>(input.cols())/static_cast<float>(get_step()));
  const int height = ceil(static_cast<float>(input.rows())/static_cast<float>(get_step()));
  const int n_chan_i = 3; 
  const int n_chan_o = 3; 
  recipe = new Recipe(height, width, n_chan_i, n_chan_o);
  recipe->set_dc(dc);
  recipe->set_ac(ac);

  const int num_chan = n_chan_i * n_chan_o;
  for(int i=0; i < num_chan; i++){
    recipe->quantize_mins[i] = meta[i];
    recipe->quantize_maxs[i] = meta[num_chan+i];
  }
  set_images(input, input); // the second one is dummy

}

#ifndef __ANDROID__
void TransformModel::fit() {
    fit_recipe();
    recipe->quantize();
    recipe->write("recipe");
}
#endif

XImage TransformModel::predict() {
    recipe->dequantize();
    XImage out = reconstruct();
    return out;
}

void TransformModel::check_fit_io(){
    if(!input || !output){
        printf("No input/output pair, cannot make recipe\n");
    }
};

void TransformModel::fit_recipe_by_Halide(const Image<float>& input,
                                          const Image<float>& output) const{

  // Lowpass
  Image<float> lp_input(width/wSize, height/wSize, 3),
               lp_output(width/wSize, height/wSize, 3);

  halide_resize(input,  lp_input.height(),  lp_input.width(),  lp_input); 
  halide_resize(output, lp_output.height(), lp_output.width(), lp_output); 
  // High-Pass
  // TODO: to be done
}

void TransformModel::fit_recipe() {
    // Lowpass
    XImage lp_input, lp_output;
    Warp warp;
    warp.imresize(*input, height/wSize, width/wSize,Warp::BICUBIC, &lp_input);
    warp.imresize(*output, height/wSize, width/wSize,Warp::BICUBIC, &lp_output);
    printf("lp size %dx%d\n", lp_input.rows(),lp_input.cols());

    // Highpass
    XImage hp_input, hp_output;
    warp.imresize(lp_input, height,width,Warp::BICUBIC, &hp_input);
    warp.imresize(lp_output, height,width,Warp::BICUBIC, &hp_output);
    hp_input = *(this->input) - hp_input;
    hp_output = *(this->output) - hp_output;

    if(recipe) {
        delete recipe;
        recipe = NULL;
    }
    recipe = new Recipe(mdl_h,mdl_w,n_chan_i,n_chan_o);
    recipe->set_dc(lp_output);

    // Fit affine model per patch
    int progress = 0;
    for (int imin = 0; imin < height; imin += step)
    for (int jmin = 0; jmin < width; jmin += step)
    {
        progress   += 1;
        int mdl_i = imin/step;
        int mdl_j = jmin/step;
        int h = min(imin+wSize,height) - imin;
        int w = min(jmin+wSize,width) - jmin;

        // Extract patches
        MatType p_input(h*w, n_chan_i);
        MatType p_output(h*w, n_chan_o);
        for(int c = 0; c < n_chan_i ; ++c)
        {
            MatType tmp = hp_input.at(c).block(imin,jmin,h,w);
            tmp.resize(h*w,1);
            p_input.col(c) = tmp;
        }
        for(int c = 0; c < n_chan_o ; ++c)
        {
            MatType tmp = hp_output.at(c).block(imin,jmin,h,w);
            tmp.resize(h*w,1);
            p_output.col(c) = tmp;
        }

        // Solve linear system
        MatType lhs = p_input.transpose() * p_input + epsilon*MatType::Identity(n_chan_i, n_chan_i);
        MatType rhs = p_input.transpose() * p_output;
        MatType coef = lhs.ldlt().solve(rhs);
        recipe->set_coefficients(mdl_i, mdl_j, coef); 

        printf("\r      - Fitting: %.2f%%",  ((100.0*progress)/(mdl_h*mdl_w)));
    }
    printf("\n");
}


void TransformModel::reconstruct_by_Halide(const Image<float>& input, 
                               const Image<float>& ac,
                               const Image<float>& dc, 
                               const PixelType* meta,
                               Image<float>* output) const{
  Image<float> ds(width/wSize, height/wSize, 3);
  Image<float> new_dc(width, height, 3), low_pass(width, height, 3);
  halide_resize(input, ds.height(), ds.width(), ds);                           
  halide_resize(ds, output->height(), output->width(), low_pass);

  halide_resize(dc, output->height(), output->width(), new_dc);

}

XImage TransformModel::reconstruct() {
    // Lowpass
    timeval t0, t_decompose, t_recon;
    gettimeofday(&t0, NULL);

    XImage lp_input;
    Warp warp;
    warp.imresize(*input, height/wSize, width/wSize,Warp::BICUBIC, &lp_input);

    // Highpass
    XImage hp_input;
    warp.imresize(lp_input, height,width,Warp::BICUBIC, &hp_input);
    hp_input = *(this->input) - hp_input;

    // Result
    XImage reconstructed(height, width, n_chan_o);
    XImage dc;
    warp.imresize(recipe->get_dc(), height, width, Warp::BICUBIC, &dc);
    
    gettimeofday(&t_decompose, NULL);
    unsigned int t_dec = (t_decompose.tv_sec - t0.tv_sec) * 1000000 + (t_decompose.tv_usec - t0.tv_usec);
    std::cout<< "t_dec = " <<  t_dec << std::endl;

    // Reconstruct each patch
    if (use_halide){
      int progress = 0;
      for (int imin = 0; imin < height; imin += step)
      for (int jmin = 0; jmin < width; jmin += step)
      {
          progress   += 1;
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
          recipe->get_coefficients(mdl_i, mdl_j, coef); 
          MatType p_reconstructed = p_input * coef;

          for(int c = 0; c < n_chan_o ; ++c)
          {
              MatType tmp = p_reconstructed.col(c);
              tmp.resize(h,w);
              reconstructed.at(c).block(imin,jmin,h,w) = tmp;
          }
      }
    } else { // use_halide
      XImage foo_ac(1);
      foo_ac.at(0) = recipe->ac;
      Image<float> HL_hp_input(hp_input.cols(), hp_input.rows(), hp_input.channels()),
                   HL_ac(foo_ac.cols(), foo_ac.rows(), foo_ac.channels()),
                   HL_recon(input->cols(), input->rows(), input->channels());

      hp_input.to_Halide(&HL_hp_input);
      foo_ac.to_Halide(&HL_ac);
      halide_recon(HL_hp_input, HL_ac, HL_recon); 
      reconstructed.from_Halide(HL_recon);
    }

    reconstructed = reconstructed + dc ;

    gettimeofday(&t_recon, NULL);

    /* timing */
    unsigned int t_rec = (t_recon.tv_sec - t_decompose.tv_sec) * 1000000 + (t_recon.tv_usec - t_decompose.tv_usec);
    std::cout<< "t_recon = " << t_rec << std::endl;
    return reconstructed;
}

} // namespace xform
