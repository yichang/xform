#ifndef STYLE_TRANSFER_WRAPPER_H_NXOCJ6DN
#define STYLE_TRANSFER_WRAPPER_H_NXOCJ6DN

#include <stdio.h>
#include <vector>
#include "style_transfer.h"
#include "gradient_norm.h"
#include "static_image.h"
#include "image_io.h"
#include <iostream>
#include <sys/time.h>

#include "const.h"
#include "hist.h"

using namespace std;


int style_transfer_wrapper(Image<float> &input, Image<float> &model,int levels, Image<float> &output);

#endif /* end of include guard: STYLE_TRANSFER_WRAPPER_H_NXOCJ6DN */

