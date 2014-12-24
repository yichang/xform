#include <algorithm>
#include <math.h>
#include "XImage.h"
#include "Warp.h"

using namespace xform; 

void Warp::imresize(const XImage& im_in, const float scale, 
                                      XImage* im_out) const{ 
  imresize(im_in, scale, BILINEAR, im_out);
}


void Warp::imresize(const XImage& im_in, const float scale, 
        const InterpType interp_type, XImage* im_out) const{ 

  const int new_h = ceil(static_cast<int>(scale * im_in.rows()));
  const int new_w = ceil(static_cast<int>(scale * im_in.cols()));
  imresize(im_in, new_h, new_w, interp_type,  im_out);
}

void Warp::imresize(const XImage& im_in, const int new_h, 
  const int new_w, const InterpType interp_type, XImage* im_out) const{ 
  
  const int num_channels = im_in.channels();
  (*im_out) = XImage(num_channels);
  for(int i = 0; i < num_channels; i++)
    imresize(im_in.at(i), new_h, new_w, interp_type, &(im_out->at(i)));
}

void Warp::resize_1d(const VecType &in, const InterpType interp_type, 
        double inv_scale, VecType *out) const
{
    // For the anti-aliasing filter
    double scale = 1.0f/inv_scale;
    if(scale>1){
        scale = 1;
    }
    int length = in.rows();

    switch(interp_type){
    case Warp::NEAREST:
        break;
    case Warp::BILINEAR:
    {
        int kernel_width = 1;
        for(double i = 0; i < out->rows(); ++i  )
        {
            double u = i*inv_scale + 0.5f*(-1.0f+inv_scale);
            int l = floor(u-kernel_width/2);
            int r = l+kernel_width;
            double val = 0;
            // if(i == out->rows()-1){
            //     printf("%f,%d,%d\n",u,l,r);
            // }
            double w = 0;
            double weight = 0;
            for(int x = l; x<=r; ++x)
            {
                if(x<0 || x>=length){
                    continue;
                }
                w = kernel_linear(u-x,scale);
                val += w*in(x);
                weight += w;
            }
            if(weight>0)
            {
                val/= weight;
            }
            (*out)(i) = val;
        }
        break;
    }
    case Warp::BICUBIC:
        break;
    }
}

double Warp::kernel_linear(double x, double scale) const {
    x = fabs(x*scale);
    double f = x<1 ? 1-x : 0;
    f *= scale;
    return f;
}

void Warp::imresize(const ImageType_1& im_in, const int new_h, 
    const int new_w, const InterpType interp_type, ImageType_1* im_out) const{ 

    int width  = im_in.cols();
    int height = im_in.rows();

    (*im_out) = ImageType_1(new_h, new_w);

    const double inv_x_scale = (static_cast<double>(width))/
        (static_cast<double>(new_w));
    const double inv_y_scale = (static_cast<double>(height))/ 
        (static_cast<double>(new_h));

  // process_order, kernel width
  if(inv_x_scale < inv_y_scale) {
      ImageType_1 buffer(new_h, width);
      for(int j = 0; j < width; ++j)
      {
          VecType in = im_in.col(j);
          VecType out(new_h);
          resize_1d(in, interp_type, inv_y_scale, &out);
          buffer.col(j) = out;
      }
      for(int i = 0; i < new_h; ++i)
      {
          VecType in = buffer.row(i);
          VecType out(new_w);
          resize_1d(in, interp_type, inv_x_scale, &out);
          im_out->row(i) = out;
      }
  } else {
      ImageType_1 buffer(height, new_w);
      for(int i = 0; i < height; ++i)
      {
          VecType in = im_in.row(i);
          VecType out(new_w);
          resize_1d(in, interp_type, inv_x_scale, &out);
          buffer.row(i) = out;
      }
      for(int j = 0; j < new_w; ++j)
      {
          VecType in = buffer.col(j);
          VecType out(new_h);
          resize_1d(in, interp_type, inv_y_scale, &out);
          im_out->col(j) = out;
      }
  }

  // else if (interp_type == NEAREST)
  // {
  //     for(double i=0; i < (double)new_h; i++){
  //         y = i * inv_y_scale + 0.5f*(1-inv_y_scale); 
  //         for(double j=0; j < (double)new_w; j++){
  //             x = j * inv_x_scale + 0.5f*(1-inv_x_scale); 
  //             (*im_out)(i,j) = im_in(y, x);
  //         }
  //     }
  // }else{ // BICUBIC
  //     //TODO(yichang): bicubic interpolation 
  // }
}


