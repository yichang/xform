#include <stdlib.h>
#include "XImage.h"
#include "Filter.h"

using namespace xform;


void Filter::box(const XImage& im_in, const int b_width, XImage* im_out)const{
  box(im_in, b_width, SYMMETRIC, im_out);
}

void Filter::box_iteration(const XImage& im_in, const int b_width, 
                     const int n_iter, XImage* im_out) const{
  assert((b_width%2)==1);
  assert(b_width < std::min(im_in.rows(), im_in.cols()));

  (*im_out) = XImage(im_in.channels());
  for(int i=0; i < im_in.channels(); i++){
    ImageType_1 in = im_in.at(i), out; 
    for(int j = 0; j < n_iter; j++){
      box(in, b_width, REPLICATE, &out);
      in = out/(b_width * b_width); 
    }
    im_out->at(i) = in; 
  }
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
