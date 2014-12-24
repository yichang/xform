#include "TransformModel.h"
#include "Warp.h"

#include <iostream>

using namespace std;

namespace xform{

TransformModel::TransformModel(){
    input  = NULL;
    output = NULL;
    recipe = NULL;

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

void TransformModel::set_images(const XImage &input, const XImage &output) {
    this->input = &input;
    this->output = &output;

    height   = input.rows();
    width    = input.cols();
    n_chan_o = output.channels();
    n_chan_i = input.channels();
    printf("input size %dx%dx%d\n", height, width,n_chan_i);
    
    // Size of the recipe
    mdl_h = ceil(1.0f*height/step);
    mdl_w = ceil(1.0f*width/step);
}

void TransformModel::process() {
    fit_recipe();
    // recipe->write("../output/recipe");
}

void TransformModel::check_fit_io(){
    if(!input || !output){
        printf("No input/output pair, cannot make recipe\n");
    }
};

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

XImage TransformModel::reconstruct() {
    // Lowpass
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
    
    // Reconstruct each patch
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

        printf("\r      - Reconstruction: %.2f%%",  ((100.0*progress)/(mdl_h*mdl_w)));
    }
    printf("\n");

    reconstructed = reconstructed + dc ;
    return reconstructed;
}

} // namespace xform
