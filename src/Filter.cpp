#include <stdlib.h>
#include "XImage.h"
#include "Filter.h"

using namespace xform;

// Separable filter

void Filter::sep_kernel(const XImage& im_in, const KernelType_1D& kernel, 
    const BoundaryType boundary_type, XImage* im_out) const{

  const int num_channels = im_in.channels(); 
  assert(num_channels > 0);
  (*im_out) = XImage(num_channels);
  for(int i=0; i < num_channels; i++)
    sep_kernel(im_in.at(i), kernel, boundary_type, &(im_out->at(i)));
}
void Filter::sep_kernel(const ImageType_1& im_in, const KernelType_1D& kernel, 
    const BoundaryType boundary_type, ImageType_1* im_out) const{ 
  //TODO(yichang): two pass separable filter
}


// Box filter using recursive implementation 
void Filter::box(const XImage& im_in, const int b_width, XImage* im_out)const{
  box(im_in, b_width, SYMMETRIC, im_out);
}

void Filter::box_iteration(const XImage& im_in, const int b_width, 
                     const int n_iter, XImage* im_out) const{
  assert((b_width%2)==1);
  assert(b_width < std::min(im_in.rows(), im_in.cols()));

  (*im_out) = XImage(im_in.channels());
  for(int i=0; i < im_in.channels(); i++)
    box_iteration(im_in.at(i),b_width, n_iter, &(im_out->at(i))); 
}
void Filter::box_iteration(const ImageType_1& im_in, const int b_width, 
                     const int n_iter, ImageType_1* im_out) const{
  assert((b_width%2)==1);
  assert(b_width < std::min(im_in.rows(), im_in.cols()));

  ImageType_1 in = im_in, out; 
  for(int j = 0; j < n_iter; j++){
    box(in, b_width, SYMMETRIC, &out);
    in = out/(b_width * b_width); 
  }
  (*im_out) = in; 
}
void Filter::box(const XImage& im_in, const int b_width, 
                 const BoundaryType boundary_type, XImage* im_out)const{

  assert((b_width%2)==1);
  assert(b_width < std::min(im_in.rows(), im_in.cols()));

  (*im_out) = XImage(im_in.channels());
  for(int i=0; i < im_in.channels(); i++){
    box(im_in.at(i), b_width, boundary_type, &(im_out->at(i)));
  }

  /* Normalization */
  if (boundary_type == ZERO_PAD){
  ImageType_1 z(im_in.rows(), im_in.cols()), nn;
  z.setOnes();
  recursiveBoxFilterZeroPad(z, b_width, &nn);
  for(int i=0; i < im_in.channels(); i++)
    im_out->at(i).array() = im_out->at(i).array() / nn.array();
  }else{
  for(int i=0; i < im_in.channels(); i++)
    im_out->at(i).array() /= (b_width * b_width);
  }

}

void Filter::box(const ImageType_1& im_in, const int b_width, 
                 const BoundaryType boundary_type, ImageType_1* im_out) const{
  if (boundary_type == ZERO_PAD) 
    recursiveBoxFilterZeroPad(im_in, b_width, im_out);
  else
    recursiveBoxFilterNoPad(im_in, b_width, boundary_type, im_out);
}

int Filter::reflect(const int val, const int width, 
                    const BoundaryType boundary_type) const{

  assert((boundary_type == SYMMETRIC)||(boundary_type==REPLICATE)
       ||(boundary_type == CIRCULAR));

  if (boundary_type == SYMMETRIC)
    if (val < 0)
      return std::abs(val);
    else if (val >= width) 
      return 2 * width - val - 1;
    else
      return val; 
  else if (boundary_type == REPLICATE)
      return std::min(std::max(val, 0), width - 1); 
  else{                    //CIRCULAR
    if (val < 0)
      return val + width; 
    else if (val >= width)
      return val - width; 
    else
      return val; 
  }
}
void Filter::recursiveBoxFilterNoPad(const ImageType_1& im_in, 
                                     const int b_width, 
                                     const BoundaryType boundary_type, 
                                     ImageType_1* im_out) const{

  assert((boundary_type == SYMMETRIC)||(boundary_type==REPLICATE)
       ||(boundary_type == CIRCULAR));

  int height = im_in.rows(); 
  int width = im_in.cols();
  int half_width = (b_width-1)/2;

  /* Two pass implementation */
  ImageType_1 h_buf(height, width); //Horizontal pass 
  h_buf.setZero();
  for(int j = -half_width; j <  half_width + 1; j++)
    h_buf.col(0) += im_in.col(reflect(j, width, boundary_type));

  for (int j = 1; j < width; j++)
    h_buf.col(j) = h_buf.col(j-1) 
                   + im_in.col(reflect(j+half_width, width, boundary_type)) 
                   - im_in.col(reflect(j-half_width-1, width, boundary_type));

  (*im_out) =  ImageType_1(height, width); //Vertical pass
  im_out->setZero();
  for(int i = -half_width; i < half_width + 1; i++)
    im_out->row(0) += h_buf.row(reflect(i, height, boundary_type));

  for (int i = 1; i < height; i++)
    im_out->row(i) = im_out->row(i-1) 
                 + h_buf.row(reflect(i + half_width, height, boundary_type)) 
                 - h_buf.row(reflect(i - half_width -1, height, boundary_type));
}
void Filter::recursiveBoxFilterZeroPad(const ImageType_1& im_in, 
                                       const int b_width, 
                                       ImageType_1* im_out) const{
  int height = im_in.rows(); 
  int width = im_in.cols();
  int half_width = (b_width-1)/2;

  /* Zero padding */
  ImageType_1 im_pad(height + b_width - 1, width + b_width - 1);
  im_pad.setZero(); 
  im_pad.block(half_width, half_width, height, width) = im_in;  
  int pad_height = im_pad.rows(); 
  int pad_width = im_pad.cols();

  /* Two pass implementation */
  ImageType_1 h_buf(pad_height, pad_width); //Horizontal pass 
  h_buf.setZero();
  for(int j = half_width; j < 2 * half_width + 1; j++)
    h_buf.col(half_width) += im_pad.col(j);

  for (int j = half_width + 1; j < pad_width - half_width; j++)
    h_buf.col(j) = h_buf.col(j - 1) + im_pad.col(j + half_width) 
                                    - im_pad.col(j - half_width -1);

  ImageType_1 v_buf(pad_height, pad_width); //Vertical pass
  v_buf.setZero();
  for(int i = half_width; i < 2 * half_width + 1; i++)
    v_buf.row(half_width) += h_buf.row(i);

  for (int i = half_width + 1; i < pad_height - half_width; i++)
    v_buf.row(i) = v_buf.row(i - 1) + h_buf.row(i + half_width) 
                                    - h_buf.row(i - half_width -1);

  *im_out = v_buf.block(half_width, half_width, height, width); 
}
void Filter::boxBySumArea(const XImage& im_in, const int b_width, 
        const BoundaryType boundary_type, XImage* im_out) const{
  int num_channels = im_in.channels(); 
  *im_out = XImage(num_channels); 
  for(int i=0; i < num_channels; i++)
    boxBySumArea(im_in.at(i), b_width, boundary_type, &(im_out->at(i)));
}

void Filter::boxBySumArea(const ImageType_1& im_in, const int b_width, 
        const BoundaryType boundary_type, ImageType_1* im_out) const{   
  const int half_width = (b_width-1)/2;
  const int height = im_in.rows();
  const int width = im_in.cols();
  ImageType_1 buf(height + b_width - 1, width + b_width - 1);
  buf.setZero();
  buf.block(half_width, half_width, height, width) = im_in;
  
  assert(boundary_type == REPLICATE);

  if (boundary_type == REPLICATE){
    for(int j = 0; j < half_width; j++)
      buf.block(half_width, j, height, 1) = im_in.col(0);
    for(int j = width + half_width - 1; j < width + b_width - 1; j++)
      buf.block(half_width, j, height, 1) = im_in.col(width-1);
    for(int i = 0; i < half_width; i++)
      buf.block(i, half_width, 1, width) = im_in.row(0);
    for(int i = height + half_width - 1; i < height + b_width - 1; i++)
      buf.block(i, half_width, 1, width) = im_in.row(height-1);

    // Four corners, top left
    for(int i=0; i<half_width; i++)
      for(int j=0; j<half_width; j++)
        buf(i,j) = im_in(0,0);

    // Bottom left 
    for(int i = half_width + height - 1; i< height + b_width - 1; i++)
      for(int j = 0; j < half_width; j++)
        buf(i, j) = im_in(height - 1,0);

    // Top right
    for(int i = 0; i < half_width; i++)
      for(int j = half_width + width - 1; j < width + b_width - 1; j++)
        buf(i,j) = im_in(0, width - 1);

    // Bottom right
    for(int i = half_width + height - 1; i< height + b_width - 1; i++)
      for(int j = half_width + width - 1; j < width + b_width - 1; j++)
        buf(i,j) = im_in(height - 1, width - 1);
  }

  for(int j=1; j < buf.cols(); j++)
    buf.col(j) = buf.col(j) + buf.col(j-1);

  for(int i=1; i < buf.rows(); i++)
    buf.row(i) = buf.row(i) + buf.row(i-1);

  *im_out = buf.block(b_width-1, b_width-1, height, width) 
            + buf.block(0, 0, height, width)
            - buf.block(b_width-1, 0, height, width)
            - buf.block(0, b_width-1, height, width);

  *im_out = (*im_out)/static_cast<PixelType>(b_width * b_width); 
}

