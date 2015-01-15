#include <vector>
#include "LocalLaplacian.h"
#include "Pyramid.h" 
#include "Filter.h"
#include "Curve.h" 
#include "util.h"

using namespace xform;

void LocalLaplacian::adjustDetails(const ImageType_1& im_in,  
                                   const PixelType sigma, // Detail range
                                   const PixelType alpha, // Power curve
                                   const int num_levels,
                                   const float interval, // on g 
                                   const float full_range, // 100 for L
                                         ImageType_1* im_out) const{


  // Build Gaussian pyramid
  Pyramid gaussian_pyr(im_in, num_levels, Pyramid::GAUSSIAN, false); 

  // Build Laplacian Pyramids
  const int num_samples = static_cast<int>(full_range/interval) + 1;
  vector<Pyramid> laplacian_pyrs;
  Curve curve;
  for(int i=0; i < num_samples; i++){
    PixelType g0 = interval * i;
    ImageType_1 cur;
    curve.sShape(im_in, sigma, g0, alpha, &cur);
    Pyramid cur_lap_pyr(cur, num_levels, Pyramid::LAPLACIAN, false);
    laplacian_pyrs.push_back(cur_lap_pyr);
  }

  // Construct the output pyramid
  Pyramid out_pyr(num_levels, Pyramid::LAPLACIAN, false);
  for(int k = 0; k < out_pyr.levels(); k++){ 
    const int cur_height = gaussian_pyr.at(k).rows();
    const int cur_width = gaussian_pyr.at(k).cols();
    out_pyr.at(k) = ImageType_1(cur_height, cur_width);
    for(int i=0; i < cur_height; i++){
      for(int j=0; j < cur_width; j++){

        // Linear interp.
        const PixelType g = gaussian_pyr.at(k)(i, j);
        const int lb = static_cast<int>(g/interval);
        if (lb >= num_samples - 1){
          out_pyr.at(k)(i,j) = laplacian_pyrs[lb].at(k)(i, j); 
        } else {
          const int ub = lb + 1;
          const float a = (g - interval * (float)lb)/interval;
          assert(a>=0);
          assert(a<=1);
          out_pyr.at(k)(i,j) = a * laplacian_pyrs[ub].at(k)(i, j) + 
                              (1-a) * laplacian_pyrs[lb].at(k)(i, j);
        }
    }}
  }
  out_pyr.collapse(im_out);
}
