#include <limits>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-W#warnings"

#include <qapplication.h>

#pragma clang diagnostic pop

#include <ipp.h>

#include "argparser.h"
#include "array_tools.h"
#include "channel_image.h"
#include "chrono.h"
#include "image_file.h"
#include "image_container_traits.h"

#include "image_filter/histogram.h"


using namespace std;

typedef float    real_type;
typedef unsigned size_type;

// typedef RGB_NO_GAMMA_channel_image<real_type> rgb_image_type;
typedef RGB_channel_image<real_type> rgb_image_type;

typedef Array_2D<bool> bool_array_type;

typedef Array_2D<real_type>      real_array_type;
typedef vector<real_array_type>  real_pyramid_type;

typedef Geometry::Vec3<real_type> real3_type;
typedef Array_2D<real3_type>      real3_array_type;

typedef Image_filter::Histogram_1D    histogram_type;
typedef Image_filter::Lookup_table_1D lookup_type;

const real_type epsilon = 1e-5;

const int pyramid_border = 2;

bool masked_input = false;

inline real_type log_function(const real_type x){
//   return log(x + epsilon);
  return x;
}

inline real_type exp_function(const real_type x){
//   return exp(x) - epsilon;
  return x;
}



inline void expand(const real_array_type& input,
		   const size_type         target_width,
		   const size_type         target_height,
		   real_array_type* const output){
  
  output->resize(target_width,target_height);
    
  IppiSize input_size = {input.height() - 2 * pyramid_border,
			 input.width() - 2 * pyramid_border};
  
  int buffer_size;
  ippiPyrUpGetBufSize_Gauss5x5(input.height() - 2 * pyramid_border,
			       ipp32f,
			       1,
			       &buffer_size);
  Ipp8u* buffer = (Ipp8u*) malloc(buffer_size * sizeof(Ipp8u));

  ippiPyrUp_Gauss5x5_32f_C1R(&(input(pyramid_border,pyramid_border)),
			     input.height() * sizeof(real_type),
			     &((*output)(pyramid_border,pyramid_border)),
			     output->height() * sizeof(real_type),
			     input_size,
			     buffer);


  free(buffer);
}








inline void reduce(const real_array_type& input,
		   real_array_type* const output){
  
  output->assign((input.width() + 1) / 2 + pyramid_border,
		 (input.height() + 1) / 2 + pyramid_border,
		 0.0);

//   real_array_type out_weight((input.width() + 1) / 2 + pyramid_border,
// 			     (input.height() + 1) / 2 + pyramid_border,
// 			     0.0);
  
  IppiSize input_size = {input.height() - 2 * pyramid_border,
			 input.width() - 2 * pyramid_border};

  real_array_type weight(input.width(),input.height(),0.0);

//   ippiSet_32f_C1R(1.0,
// 		  &(weight(pyramid_border,pyramid_border)),
// 		  input.height() * sizeof(real_type),
// 		  input_size);
//   weight(3,3) = 1.0;
  
  int buffer_size;
  ippiPyrDownGetBufSize_Gauss5x5(input.height() - 2 * pyramid_border,
				 ipp32f,
				 1,
				 &buffer_size);
  Ipp8u* buffer = (Ipp8u*) malloc(buffer_size * sizeof(Ipp8u));

  ippiPyrDown_Gauss5x5_32f_C1R(&(input(pyramid_border,pyramid_border)),
			       input.height() * sizeof(real_type),
			       &((*output)(pyramid_border,pyramid_border)),
			       output->height() * sizeof(real_type),
			       input_size,
			       buffer);
  
//   ippiPyrDown_Gauss5x5_32f_C1R(&(weight(pyramid_border,pyramid_border)),
// 			       weight.height() * sizeof(real_type),
// 			       &(out_weight(pyramid_border,pyramid_border)),
// 			       out_weight.height() * sizeof(real_type),
// 			       input_size,
// 			       buffer);

//   cout << "## " << out_weight(0,0) << " " << out_weight(1,1) << " " << out_weight(2,2) << " " << out_weight(3,3) << " " << out_weight(4,4) << " " << out_weight(5,5) << endl;
//   cout << "# " << out_weight(out_weight.width()-1,out_weight.height()-1)
//        << " " << out_weight(out_weight.width()-2,out_weight.height()-2)
//        << " " << out_weight(out_weight.width()-3,out_weight.height()-3) << endl;
//   cout << "# " << (*output)(0,0) << " " << (*output)(1,1) << " " << (*output)(2,2) << endl;

//   Image_file::save("debug.png",out_weight); exit(0);
  
  free(buffer);
}





inline void create_Gaussian_pyramid(real_pyramid_type* const output){
    
  real_pyramid_type& pyramid = *output;
	
  for(size_type d = 1 ; d < pyramid.size() ; ++d){
    reduce(pyramid[d-1],&(pyramid[d]));
  }
}


inline void create_Laplacian_pyramid_from_Gaussian_pyramid(const real_pyramid_type& input,
							   real_pyramid_type* const output){


  const size_type depth = input.size();
  
  real_pyramid_type& out = *output;
  out.resize(depth);
  out.back() = input.back();
  
  for(size_type d = 1 ; d < depth ; ++d){
    real_array_type& expanded_level = out[d-1];
    const real_array_type& coarse_input = input[d];
    const real_array_type& fine_input   = input[d-1];
    expand(coarse_input,fine_input.width(),fine_input.height(),&expanded_level);

    IppiSize size = {expanded_level.height(),expanded_level.width()};
    
    ippiSub_32f_C1R(&(expanded_level(0,0)),
		    expanded_level.height() * sizeof(real_type),
		    &(fine_input(0,0)),
		    fine_input.height() * sizeof(real_type),
		    &(expanded_level(0,0)),
		    expanded_level.height() * sizeof(real_type),
		    size);
  }
}

inline void create_Laplacian_pyramid(real_pyramid_type* const input,
				     real_pyramid_type* const output){

  
  create_Gaussian_pyramid(input);
  create_Laplacian_pyramid_from_Gaussian_pyramid(*input,output);
}



inline void collapse_Laplacian_pyramid(const real_pyramid_type& input,
				       real_array_type* const   output){

  real_array_type& out = *output;
  out = input.back();

  real_array_type proxy;
  
  for(size_type d = input.size() - 1 ; d > 0 ; --d){
    const real_array_type& fine_input = input[d-1];
    expand(out,fine_input.width(),fine_input.height(),&proxy);

    IppiSize size = {proxy.height(),proxy.width()};
    out = proxy;
    ippiAdd_32f_C1IR(&(fine_input(0,0)),
		     fine_input.height() * sizeof(real_type),
		     &(out(0,0)),
		     out.height() * sizeof(real_type),
		     size);
  }    
}

  
inline real_type power_curve(const real_type& i,
			      const real_type& g0,
			      const real_type   sigma_range,
			      const real_type   alpha){

  const real_type dis = i - g0;
  const real_type norm = abs(dis);

  if (norm > sigma_range) {
    return i;
  }
  else{
    return g0 + Math_tools::sign(dis) * sigma_range * pow(norm / sigma_range,alpha);
  }
}

vector<real_type> lut;
const size_type samples_per_sigma = 100;
void lut_init(const size_type samples_per_sigma,
	      const size_type range_in_sigmas = 5){
  lut.resize(range_in_sigmas * samples_per_sigma);
  for(size_type s = 0 ; s < lut.size() ; ++s){
    const real_type x = static_cast<real_type>(s) / samples_per_sigma;
    lut[s] = exp(-0.5 * x * x);
  }
  lut.back() = 0;
}

inline real_type Gaussian_derivative(const real_type&  i,
				     const real_type& g0,
				     const real_type  sigma_range,
				     const real_type  alpha,
				     const real_type  slope){

  const real_type dis = i - g0;
  const real_type ndis = dis / sigma_range;
  const real_type andis = abs(ndis);

  if (andis < 1.0) {
    return i + alpha * dis * lut[min<size_type>(lut.size()-1,andis * samples_per_sigma)];
  }
  else{
    const real_type sigma_point = g0 + Math_tools::sign(dis) * sigma_range;
    return sigma_point + slope * (i - sigma_point);
  }
  
}


template <typename REMAPPING_FUNCTION>
void local_Laplacian_filter(const real_array_type&    input,
			    const size_type           depth,
			    const size_type           n_samples,
			    const REMAPPING_FUNCTION& remapping_function,
			    real_array_type* const    output){

  real_pyramid_type input_Gaussian_pyramid(depth);
  
  input_Gaussian_pyramid[0].assign(input.width() + 2 * pyramid_border,
				   input.height() + 2 * pyramid_border,
				   0.0);

  Chrono global_chrono;
  global_chrono.start();

  real_type min_i = numeric_limits<real_type>::max();
  real_type max_i = -numeric_limits<real_type>::max(); 
  
  for(size_type x = 0 ; x < input_Gaussian_pyramid[0].width() ; ++x){

    size_type xx = 0;
    if (x > static_cast<size_type>(pyramid_border)) {
      xx = min<size_type>(x - pyramid_border, input.width()-1);
    }
    
    for(size_type y = 0 ; y < input_Gaussian_pyramid[0].height() ; ++y){
      
      size_type yy = 0;
      if (y > static_cast<size_type>(pyramid_border)) {
	yy = min<size_type>(y - pyramid_border, input.height()-1);
      }

      const real_type i = input(xx,yy);
      min_i = min(i,min_i);
      max_i = max(i,max_i);
      input_Gaussian_pyramid[0](x,y) = i;
    }
  }
  
  
    
  clog << "Compute Gaussian pyramid... " << flush;
  
  create_Gaussian_pyramid(&input_Gaussian_pyramid);

  clog << "Done!" << endl;





  
  clog << "Compute intermediate Laplacian pyramids...\n" << flush;

  vector<real_pyramid_type> intermediate_Laplacian_pyramid(n_samples);
  
//   #pragma omp parallel for
  for(size_type s = 0 ; s < n_samples ; ++s){
    
   real_pyramid_type buffer_pyramid(depth);
   
    clog << "  Sample " << s << endl;
    
    const real_type g0 = min_i + (max_i - min_i) * static_cast<real_type>(s) / (n_samples - 1);
    
    buffer_pyramid[0] = input_Gaussian_pyramid[0];

    for(real_array_type::iterator i = buffer_pyramid[0].begin() ; i != buffer_pyramid[0].end() ; ++i){

      *i = remapping_function(*i,g0);
      
    } // END OF for i

    create_Laplacian_pyramid(&buffer_pyramid,&(intermediate_Laplacian_pyramid[s]));
    
  } // END OF for s

  clog << "Done!" << endl;

  

  clog << "Compute output Laplacian pyramid...\n" << flush;

  real_pyramid_type output_Laplacian_pyramid(depth);

  output_Laplacian_pyramid.back() = input_Gaussian_pyramid.back();

  cout << "  residual size: " << input_Gaussian_pyramid.back().width() << " x " << input_Gaussian_pyramid.back().height() << endl;
  
//   #pragma omp parallel for
  for(size_type d = 0 ; d < depth - 1; ++d){

    clog << "  Depth " << d << endl;

    real_array_type& Gaussian_level  = input_Gaussian_pyramid[d];
    real_array_type& Laplacian_level = output_Laplacian_pyramid[d];
    
    Laplacian_level.resize(Gaussian_level.width(),Gaussian_level.height());

    for(size_type x = pyramid_border ; x < Gaussian_level.width() - pyramid_border ; ++x){
      for(size_type y = pyramid_border ; y < Gaussian_level.height() - pyramid_border ; ++y){

	const real_type g = (Gaussian_level(x,y) - min_i) / (max_i - min_i);
	const real_type r = g * (n_samples - 1);
// 	const size_type i = (g < 1.0) ? floor(r) : (n_samples - 2);
// 	const real_type a = r - floor(r);
	size_type i;
	real_type a;
	if (g < 0.0){
	  i = 0;
	  a = 0.0;
	}
	else if (g >= 1.0){
	  i = n_samples - 2;
	  a = 1.0;
	}
	else{
	  i = floor(r);
	  a = r - i;
	}
	
	Laplacian_level(x,y) =
	  (1.0 - a) * intermediate_Laplacian_pyramid[i][d](x,y)
	  + a * intermediate_Laplacian_pyramid[i + 1][d](x,y);

// 	cout << WHERE << endl;
      }
    }
    
  } // END OF for d

  clog << "Done!" << endl;


  real_array_type proxy;

  collapse_Laplacian_pyramid(output_Laplacian_pyramid,&proxy);

  output->resize(input.width(),input.height());
  
  for(size_type x = 0 ; x < input.width() ; ++x){
    for(size_type y = 0 ; y < input.height() ; ++y){
      (*output)(x,y) = proxy(pyramid_border + x, pyramid_border + y);   
    }
  }

  global_chrono.stop();
  clog << global_chrono.report() << endl;
  
}  






struct Gradient_transfer_function{

  const lookup_type& lookup;

  Gradient_transfer_function(const lookup_type& l)
    :lookup(l){}

   real_type operator()(const real_type i,
		       const real_type g0) const{

     const real_type d = i - g0;
     return g0 + Math_tools::sign(d) * lookup(abs(d));
   }
 
};

struct Tone_mapping_function{

  const real_type thr;
  
  Tone_mapping_function(const real_type threshold)
    :thr(threshold){}

  real_type operator()(const real_type i,
		       const real_type g0) const{

    const real_type d = i - g0;
    return g0 + Math_tools::sign(d) * min(thr,abs(d));
  }
};


void compute_intensity_histogram(const real_array_type& image,
				 const bool_array_type &mask,
				 histogram_type* const  histogram){
  
//   histogram->assign(image.begin(),image.end());
  vector<real_type> value;
  
  for(size_type x = 1 ; x < image.width() ; ++x){
    for(size_type y = 1 ; y < image.height() ; ++y){

      if (mask(x,y)){
	  value.push_back(image(x,y));
      }
    }
  }

  histogram->assign(value.begin(),value.end());
}


void compute_gradient_histogram(const real_array_type& image,
				const bool_array_type& mask,
				histogram_type* const  histogram){

  vector<real_type> value;
  
  for(size_type x = 1 ; x < image.width() ; ++x){
    for(size_type y = 1 ; y < image.height() ; ++y){

      if (mask(x,y)){
	  const real_type dx = image(x,y) - image(x-1,y);
	  const real_type dy = image(x,y) - image(x,y-1);
	  value.push_back(sqrt(dx * dx + dy * dy));
	}
    }
  }

  histogram->assign(value.begin(),value.end());
}


void remove_zeros(real3_array_type* const image,
		  const real_type almost_zero = 1.0 / 512.0){

  real_type min_non_zero = numeric_limits<real_type>::max();
  
  for(real3_array_type::iterator i = image->begin() ; i != image->end() ; ++i){
    for(size_type c = 0 ; c < 3 ; ++c){
      const real_type v = (*i)[c];
      if (v > 0){
	min_non_zero = min(min_non_zero,v);
      }
    }
  }

  
  for(real3_array_type::iterator i = image->begin() ; i != image->end() ; ++i){

    for(size_type c = 0 ; c < 3 ; ++c){

      real_type& r = (*i)[c];

//       r = max(r,almost_zero);
      r = max<real_type>(r,0.9*min_non_zero);
    }
  }
  
}





void transfer_toning(const real_array_type&  model_intensity,
		     const real3_array_type& rgb_model_ratio,
		     const real_array_type&  output_intensity,
		     real3_array_type* const rgb_output_ratio){

  const size_type lut_size = 10;
  vector<real3_type> ratio(lut_size);
  vector<size_type>  count(lut_size,0);

  rgb_output_ratio->resize(output_intensity.width(),
			   output_intensity.height());

  real_array_type::const_iterator mi = model_intensity.begin();
  for(real3_array_type::const_iterator mr = rgb_model_ratio.begin() ;
      mr != rgb_model_ratio.end() ; ++mi, ++mr){

    const size_type index = (*mi >= 1.0) ? (lut_size - 1) : floor(*mi * (lut_size - 1) + 0.5);
    
    for(size_type c = 0 ; c < 3 ; ++c){
      ratio[index][c] += log((*mr)[c]);
    }
    ++count[index];
  }

  for(size_type n = 0 ; n < lut_size ; ++n){

    if (count[n] == 0){
      size_type n_neg = n;
      while ((n_neg > 0) && (count[n_neg] == 0)){
	--n_neg;
      }

      size_type n_pos = n;

      while ((n_pos < lut_size - 1) && (count[n_pos] == 0)){
	++n_pos;
      }

      ratio[n] = (n_pos - n + 1) * ratio[n_neg] + (n - n_neg + 1) * ratio[n_pos];
      count[n] = (n_pos - n + 1) * count[n_neg] + (n - n_neg + 1) * count[n_pos];
    }

    
    for(size_type c = 0 ; c < 3 ; ++c){
      ratio[n][c] = exp(ratio[n][c] / count[n]);
    }

//     cout << ratio[n] << endl;
  }

  real_array_type::const_iterator out_i = output_intensity.begin();
  for(real3_array_type::iterator out_r = rgb_output_ratio->begin() ;
      out_r != rgb_output_ratio->end() ; ++out_i, ++out_r){

    if (*out_i >= 1.0){
      *out_r = ratio.back();
    }
    else if (*out_i <= 0.0){
      *out_r = ratio.front();
    }
    else{
      const real_type proxy = *out_i * (lut_size - 1);
      const size_type index = floor(proxy);
      const real_type alpha = proxy - index;
      *out_r = (1.0 - alpha) * ratio[index] + alpha * ratio[index + 1];
    }
    
  }
  
}


real_type compute_average_saturation(const real3_array_type& input){

  if (input.empty()) return 0;
  
  real_type average_saturation = 0;
  real_type w_sum = 0;
  
  for(real3_array_type::const_iterator i = input.begin() ; i != input.end() ; ++i){

    const real3_type& c = *i;
    
    const real_type mean = (c[0] + c[1] + c[2]) / 3.0;

    const real_type std_dev = sqrt((c - real3_type(mean,mean,mean)).square_norm() / 3.0);

    const real_type w = 1.0 - 4.0 * (mean - 0.5) * (mean - 0.5);
    
    average_saturation += std_dev * w;
    w_sum += w;
  }

  return average_saturation / w_sum;
}



void transfer_saturation(const real3_array_type& model,
			 real3_array_type* const output){

  const real_type model_saturation = compute_average_saturation(model);
  const real_type input_saturation = compute_average_saturation(*output);
  const real_type ratio = (input_saturation > 0) ? (model_saturation / input_saturation) : 1;

  cout << "ratio = " << ratio << endl;
  
  for(real3_array_type::iterator o = output->begin() ; o != output->end() ; ++o){

     const real3_type& c = *o;
    
    const real_type mean = (c[0] + c[1] + c[2]) / 3.0;
    const real3_type m(mean,mean,mean);
    
    *o = (c - m) * ratio + m;
  }
}



void transfer_dynamic_range(const real_array_type& model_intensity,
			    real_array_type* const output_intensity){  
  
  const real_type epsilon = 0.1;
  
  const real_type m_min = Math_tools::median(model_intensity.begin(), model_intensity.end(), epsilon);
  const real_type m_max = Math_tools::median(model_intensity.begin(), model_intensity.end(), 1.0 - epsilon);
  
  const real_type i_min = Math_tools::median(output_intensity->begin(), output_intensity->end(), epsilon);
  const real_type i_max = Math_tools::median(output_intensity->begin(), output_intensity->end(), 1.0 - epsilon);

  const real_type ratio = (m_max - m_min) / (i_max - i_min);

  for(real_array_type::iterator o = output_intensity->begin() ; o != output_intensity->end() ; ++o){
    *o = (*o - i_min) * ratio + m_min;
  }

  
}

void transfer_dynamic_range(const real_array_type& model_intensity,
			    const bool_array_type& model_mask,
			    real_array_type* const output_intensity,
			    const bool_array_type& output_mask){

  vector<real_type> model_value, output_value;

  bool_array_type::const_iterator m = model_mask.begin();
  for(real_array_type::const_iterator i = model_intensity.begin(); i != model_intensity.end() ; ++i, ++m){

    if (*m) model_value.push_back(*i);
  }

  m = output_mask.begin();

  for(real_array_type::const_iterator i = output_intensity->begin(); i != output_intensity->end() ; ++i, ++m){

    if (*m) output_value.push_back(*i);
  }
  
  
  const real_type epsilon = 0.1;
  
  const real_type m_min = Math_tools::median(model_value.begin(), model_value.end(), epsilon);
  const real_type m_max = Math_tools::median(model_value.begin(), model_value.end(), 1.0 - epsilon);
  
  const real_type i_min = Math_tools::median(output_value.begin(), output_value.end(), epsilon);
  const real_type i_max = Math_tools::median(output_value.begin(), output_value.end(), 1.0 - epsilon);

  const real_type ratio = (m_max - m_min) / (i_max - i_min);

  for(real_array_type::iterator o = output_intensity->begin() ; o != output_intensity->end() ; ++o){
    *o = (*o - i_min) * ratio + m_min;
  }
  
}



int main(int argc, char** argv)
{
  // Read command lines arguments.
  QApplication application(argc,argv);

  histogram_type::set_default_number_of_bins(200);
  histogram_type::set_default_inverse_oversampling(10);

  string input_name;
  string input_mask_name;
  string model_name;
  string model_mask_name;
  string output_name("output.png");
  size_type n_samples = 20;
  size_type depth = 0;
  size_type n_iterations = 2;
  real_type alpha = 0.5;
    
  parse(argc,argv)
    .forParameter('i',"input",input_name,"input image")
    .forParameter('j',"inputmask",input_mask_name,"input mask",false)
    .forParameter('m',"model",model_name,"model image")
    .forParameter('l',"modelmask",model_mask_name,"model mask",false)
    .forParameter('o',"output",output_name,"output image",false)
    .forParameter('n',"n_iterations",n_iterations,"number of iterations",false)
    .forParameter('d',"depth",depth,"depth",false)
    .forParameter('s',"n_samples",n_samples,"number of samples along the intensity axis",false)
    .forParameter('a',"alpha",alpha,"alpha",false)
    .forErrors(std::cerr);
  
  ippSetNumThreads(1);

  lut_init(samples_per_sigma);
  
  // I
  rgb_image_type input_image;  
  Image_file::load(input_name.c_str(),&input_image);

  // M
  rgb_image_type model_image;  
  Image_file::load(model_name.c_str(),&model_image);

  bool_array_type input_mask;
  if (!input_mask_name.empty()){
    Image_file::load(input_mask_name.c_str(),&input_mask);
    masked_input = true;
  }
  else{
    input_mask.assign(input_image.width(),input_image.height(),true);
  }

  bool_array_type model_mask;
  if (!model_mask_name.empty()){
    Image_file::load(model_mask_name.c_str(),&model_mask);
  }
  else{
    model_mask.assign(model_image.width(),model_image.height(),true);
  }

  size_type min_d = max(input_image.width(),input_image.height());

  if (depth == 0){
    depth = 1;
    while (min_d > 1){
      ++depth;
      min_d = (min_d + 1) / 2;
    }
  }

  clog << "depth: " << depth << endl;
  
  //XXX: step 1, gaussian pyramid of input
  real_pyramid_type input_Gaussian_pyramid(depth);
  real3_array_type rgb_input;
  input_image.to_vector_array(&rgb_input);
  
  remove_zeros(&rgb_input);
  
  real3_array_type rgb_model;
  model_image.to_vector_array(&rgb_model);
  remove_zeros(&rgb_model);
  
  real_array_type input_intensity(rgb_input.width(),rgb_input.height());
  real_array_type output_intensity(rgb_input.width(),rgb_input.height());
  real_array_type model_intensity(rgb_model.width(),rgb_model.height());
 
  real3_array_type& rgb_input_ratio = rgb_input;
  real3_array_type& rgb_output = rgb_input;

  // XXX: RGB_to_gray
  for(size_type x = 0 ; x < rgb_input.width() ; ++x){
    for(size_type y = 0 ; y < rgb_input.height() ; ++y){
  
      const real3_type& i = rgb_input(x,y);
      
      const real_type intensity = (20.0 * i[0] + 40.0 * i[1] + i[2]) / 61.0;
      
      input_intensity(x,y) = intensity;
      rgb_input_ratio(x,y) /= intensity;
    }
  }
  
  // transform(input_intensity.begin(),input_intensity.end(),input_intensity.begin(),log_function);
  
  real3_array_type rgb_model_ratio = rgb_model;
  
  for(size_type x = 0 ; x < rgb_model.width() ; ++x){
    for(size_type y = 0 ; y < rgb_model.height() ; ++y){
  
      const real3_type& i = rgb_model(x,y);
      
      const real_type intensity = (40.0 * i[0] + 20.0 * i[1] + i[2]) / 61.0;
      
      model_intensity(x,y) = intensity;
      rgb_model_ratio(x,y) /= intensity;
    }
  }

  // XXX: computed M histograms
  histogram_type model_intensity_histogram;
  compute_intensity_histogram(model_intensity,
			      model_mask,
			      &model_intensity_histogram);
  model_intensity_histogram.compute_inverse();
  
  histogram_type model_gradient_histogram;
  compute_gradient_histogram(model_intensity,
			     model_mask,
			     &model_gradient_histogram);
  model_gradient_histogram.compute_inverse();

  output_intensity = input_intensity;

  real_array_type after_gradient_transfer;
  
  // XXX: iterating gradient transfer
  for(size_type i = 0 ; i < n_iterations ; ++i){

    if (i == 0){
      Tone_mapping_function remapping_function(0.5);
      local_Laplacian_filter(output_intensity,depth,n_samples*2,remapping_function,&output_intensity);
    }
    else{
      // XXX: computed I histograms
      histogram_type input_gradient_histogram;
      compute_gradient_histogram(output_intensity,
				 input_mask,
				 &input_gradient_histogram);
      
      /// XXX: LUT of the transfer function
      lookup_type gradient_lookup;
      histogram_type::build_transfer_lookup_table(input_gradient_histogram,
						  model_gradient_histogram,
						  &gradient_lookup);

    
      Gradient_transfer_function remapping_function(gradient_lookup);
      
      gradient_lookup.output_to_gnuplot("gradient_lookup.txt");
      
      /// XXX: apply the transfer using local laplacian
      local_Laplacian_filter(output_intensity,depth,n_samples * ((i == 1) ? 3 : 1),remapping_function,&output_intensity);
    }
    
    after_gradient_transfer = output_intensity;

    cout << "masked input: " << (masked_input ? "true" : "false") << endl;
    
    // XXX: intensity histogram matching
    if (i > 0){
    
      histogram_type input_intensity_histogram;
      compute_intensity_histogram(output_intensity,
				  input_mask,
				  &input_intensity_histogram);
      
      lookup_type intensity_lookup;
      histogram_type::build_transfer_lookup_table(input_intensity_histogram,
						  model_intensity_histogram,
						  &intensity_lookup);

      transform(output_intensity.begin(),output_intensity.end(),output_intensity.begin(),intensity_lookup);

    }

  }

  for(real_array_type::iterator
	agt = after_gradient_transfer.begin(),
	oi  = output_intensity.begin() ;
      agt != after_gradient_transfer.end() ;
      ++agt, ++oi){

    const real_type o = 0.5 + 0.5 * tanh(*oi * 2.0 - 1.0);
    const real_type a = 0.5 + 0.5 * tanh(*agt * 2.0 - 1.0);
    *oi = alpha * o + (1.0 - alpha) * a;
  }


  transfer_dynamic_range(model_intensity, model_mask, &output_intensity, input_mask);
  cout << "Transfer toning... " << flush;
  real3_array_type rgb_output_ratio = rgb_input_ratio;
  transfer_toning(model_intensity,rgb_model_ratio,output_intensity,&rgb_output_ratio);

  
  // XXX: add back the coloes
  for(size_type x = 0 ; x < rgb_input.width() ; ++x){
    for(size_type y = 0 ; y < rgb_input.height() ; ++y){
      rgb_output(x,y) = rgb_output_ratio(x,y) * output_intensity(x,y);   
    }
  }
  
  for(real3_array_type::iterator o = rgb_output.begin() ; o != rgb_output.end() ; ++o){
 
    (*o)[0] = max<real_type>(0.0,(*o)[0]);
    (*o)[1] = max<real_type>(0.0,(*o)[1]);
    (*o)[2] = max<real_type>(0.0,(*o)[2]);

    (*o)[0] = min<real_type>(1.0,(*o)[0]);
    (*o)[1] = min<real_type>(1.0,(*o)[1]);
    (*o)[2] = min<real_type>(1.0,(*o)[2]);

  }
  
  rgb_image_type output_image;
  output_image.from_vector_array(rgb_output);

  
  Image_file::save(output_name.c_str(),output_image);
}
