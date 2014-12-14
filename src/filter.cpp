#include "filter.h"
#include "x_image.h"

using namespace xform;

void Filter::box(const XImage& im_in, int b_width, XImage* im_out)const{

  assert((b_width%2)==1);
  (*im_out) = XImage(im_in.channels());
  for(int i=0; i < im_in.channels(); i++){
    box(im_in.at(i), b_width, &(im_out->at(i)));
  }

  /* Normalization */
  ImageType_1 z(im_in.rows(), im_in.cols()), nn;
  z.setOnes();
  recursiveBoxFilter(z, b_width, &nn);
  for(int i=0; i < im_in.channels(); i++)
    im_out->at(i).array() = im_out->at(i).array() / nn.array();
}

void Filter::box(const ImageType_1& im_in, int b_width, 
                                  ImageType_1* im_out) const{
  recursiveBoxFilter(im_in, b_width, im_out);
}

void Filter::recursiveBoxFilter(const ImageType_1& im_in, int b_width, 
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
