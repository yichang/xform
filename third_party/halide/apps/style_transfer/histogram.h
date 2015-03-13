#ifndef __HISTOGRAM__
#define __HISTOGRAM__

#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>

#ifndef NO_XML
// #include <qdom.h>
#include <QtXml/qdom.h>
#endif

#include "math_tools.h"
#include "msg_stream.h"


namespace Image_filter{


  class Lookup_table_1D;

  /*

  ########################
  #                      #
  #  class Histogram_1D  #
  #                      #
  ########################

  */

  class Histogram_1D{

  public:

    typedef unsigned int size_type;
    typedef float        real_type;
    
    inline Histogram_1D(const size_type number_of_bins = default_number_of_bins,
			const size_type inverse_oversampling = default_inverse_oversampling);

    template<typename Iterator>
    inline Histogram_1D(const size_type number_of_bins,
			const size_type inverse_oversampling,
			Iterator begin,Iterator end);
    
    template<typename Iterator>
    inline void assign(Iterator begin,Iterator end);
    
    template<typename Iterator>
    inline void add(Iterator begin,Iterator end);
    

    inline void set_min_max(const real_type min,
			    const real_type max);
    
    
    inline real_type min_value() const;
    inline real_type max_value() const;

    
    inline size_type sampling() const;
    inline size_type inverse_sampling() const;

    
    inline real_type sum_to(const real_type x) const;
    inline real_type sum_between(const real_type x,
				 const real_type y) const;

    
    inline real_type normalized_sum_to(const real_type x) const;
    inline real_type normalized_sum_between(const real_type x,
					    const real_type y) const;


    inline void compute_inverse();    
    inline real_type inverse(const real_type x) const;
    

    inline void build_equalizing_lookup_table(const real_type hist_min,
					      const real_type hist_max,
					      Lookup_table_1D* const lookup) const;
    
    inline static void build_transfer_lookup_table(const Histogram_1D& input_histogram,
						   const Histogram_1D& model_histogram,
						   Lookup_table_1D* const lookup);

    inline void output_histogram_to_gnuplot(const char* file_name) const;
    inline void output_normalized_histogram_to_gnuplot(const char* file_name) const;
    inline void output_normalized_sum_to_gnuplot(const char* file_name) const;
    inline void output_normalized_inverse_to_gnuplot(const char* file_name) const;
    inline void output_check_to_gnuplot(const char* file_name) const;

#ifndef NO_XML
    inline QDomElement to_DOM_element(const QString& name,
				      QDomDocument& document) const;
    
    inline void from_DOM_element(const QDomElement& element);
#endif

    inline static void set_default_number_of_bins(const size_type s);
    inline static void set_default_inverse_oversampling(const size_type s);
    
  private:
    size_type n_bins;
    real_type bin_delta;
    real_type first_value,last_value;
    real_type clamp_min,clamp_max;
    size_type n_samples;
    std::vector<size_type> bin;
    std::vector<real_type> sum;
    
    size_type n_inverse_points;
    real_type normalized_inverse_delta;
    std::vector<real_type> normalized_inverse;

    static size_type default_number_of_bins;
    static size_type default_inverse_oversampling;
  };


  
  
  /*

  ###########################
  #                         #
  #  class Lookup_table_1D  #
  #                         #
  ###########################

  */


  class Lookup_table_1D{

  public:
    typedef unsigned int size_type;
    typedef float real_type;

    inline Lookup_table_1D(const real_type support_min = 0.0,
			   const real_type support_max = 1.0,
			   const size_type number_of_samples = 2);
    
    inline void assign(const real_type support_min,
		       const real_type support_max,
		       const size_type number_of_samples);

    inline size_type sampling() const;

    inline void resample(const size_type number_of_samples);
    
    inline real_type operator()(const real_type x) const;

    
    inline real_type ith_sample_point(const size_type i) const;
    inline void set_ith_sample_point(const size_type i,const real_type x);

    inline void set_to_identity();

    inline void constrain_variations(const real_type max_ratio,
				     const real_type min_ratio);
    
    inline void output_to_gnuplot(const char* file_name) const;
    
  private:
    real_type min_x,max_x;
    size_type n_samples;
    real_type inv_delta;
    std::vector<real_type> sample;
    
  };

  
  
/*
  
  #############################################
  #############################################
  #############################################
  ######                                 ######
  ######   I M P L E M E N T A T I O N   ######
  ######                                 ######
  #############################################
  #############################################
  #############################################
  
*/



  
  /*

  ########################
  #                      #
  #  class Histogram_1D  #
  #                      #
  ########################

  */



  Histogram_1D::Histogram_1D(const size_type number_of_bins,
			     const size_type inverse_oversampling):
    n_bins(number_of_bins),
    n_samples(0),
    bin(n_bins,0),
    sum(n_bins+1,0),
    n_inverse_points(n_bins*inverse_oversampling+1),
    normalized_inverse_delta(1.0 / (n_inverse_points-1)){

    if (number_of_bins == 0){
      Message::error<<"Histogram_1D: number_of_bins == 0\n"
		    <<"You may use set_default_number_of_bins()."<<Message::done;
    }
    
    if (inverse_oversampling == 0){
      Message::error<<"Histogram_1D: inverse_oversampling == 0\n"
		    <<"You may use set_default_inverse_oversampling()."<<Message::done;
    }
  }
  

  template<typename Iterator>
  Histogram_1D::Histogram_1D(const size_type number_of_bins,
			     const size_type inverse_oversampling,
			     Iterator begin,Iterator end):
  n_bins(number_of_bins),
  n_samples(0),
  bin(n_bins,0),
  sum(n_bins+1,0),
  n_inverse_points(n_bins*inverse_oversampling+1),
  normalized_inverse_delta(1.0 / (n_inverse_points-1)){

    clamp_min = *std::min_element(begin,end);
    clamp_max = *std::max_element(begin,end);

    bin_delta = (clamp_max - clamp_min) / (n_bins - 1);

    first_value = clamp_min - 0.5*bin_delta;
    last_value  = first_value + bin_delta * n_bins;

    for(Iterator i=begin;i!=end;i++){
      const size_type index = static_cast<size_type>((*i - first_value)/bin_delta);
      bin[index]++;
      n_samples++;
    }

    for(size_type i=1;i<=n_bins;i++){
      sum[i] = sum[i-1] + bin[i-1];
    }
  }

  template<typename Iterator>
  void Histogram_1D::assign(Iterator begin,Iterator end){
    
//     clamp_min = std::min(2.0,*std::min_element(begin,end));
//     clamp_max = std::max(75.0,*std::max_element(begin,end));
    clamp_min = *std::min_element(begin,end);
    clamp_max = *std::max_element(begin,end);
 
    bin_delta = (clamp_max - clamp_min) / (n_bins - 1);

    first_value = clamp_min - 0.5*bin_delta;
    last_value  = first_value + bin_delta * n_bins;

    n_samples = 0;
    bin.assign(n_bins,0);

    for(Iterator i=begin;i!=end;i++){
      const size_type index = static_cast<size_type>((*i - first_value)/bin_delta);
      bin[index]++;
      n_samples++;
    }
    
    for(size_type i=1;i<=n_bins;i++){
      sum[i] = sum[i-1] + bin[i-1];
    }
    
  }


  

  template<typename Iterator>
  void Histogram_1D::add(Iterator begin,Iterator end){
    
    for(Iterator i=begin;i!=end;i++){
      const size_type index = static_cast<size_type>((*i - first_value)/bin_delta);
      bin[index]++;
      n_samples++;
    }
    
    for(size_type i=1;i<=n_bins;i++){
      sum[i] = sum[i-1] + bin[i-1];
    }
    
  }


  //! Reset everything
  void Histogram_1D::set_min_max(const real_type min,
				 const real_type max){
    
    clamp_min = min;
    clamp_max = max;
    
    bin_delta = (clamp_max - clamp_min) / (n_bins - 1);

    first_value = clamp_min - 0.5*bin_delta;
    last_value  = first_value + bin_delta * n_bins;

    n_samples = 0;
    bin.assign(n_bins,0);
  }
    
  
  
  
  Histogram_1D::real_type Histogram_1D::min_value() const{

    return clamp_min;
  }

  Histogram_1D::real_type Histogram_1D::max_value() const{

    return clamp_max;
  }

  

  Histogram_1D::size_type Histogram_1D::sampling() const{
    return n_bins;
  }
  
  Histogram_1D::size_type Histogram_1D::inverse_sampling() const{
    return n_inverse_points;
  }


  
  Histogram_1D::real_type Histogram_1D::sum_to(const real_type x) const{

    if (x<=clamp_min){
      return 0.0;
    }
    
    if (x>=clamp_max){
      return static_cast<real_type>(n_samples);
    }

    const real_type i        = (x - first_value)/bin_delta;
    const size_type bottom_i = static_cast<size_type>(i);
    real_type alpha          = i - bottom_i;

    if (bottom_i==0) {
      alpha = 2*(alpha - 0.5);
    }
    else if (bottom_i==n_bins-1){
      alpha *= 2;
    }

    return (1.0 - alpha) * sum[bottom_i] + alpha*sum[bottom_i+1];
  }


  Histogram_1D::real_type Histogram_1D::sum_between(const real_type x,
						    const real_type y) const{
    
    return sum_to(y) - sum_to(x);
  }

  
  Histogram_1D::real_type Histogram_1D::normalized_sum_to(const real_type x) const{
    
    return sum_to(x) / n_samples;
  }
  
  Histogram_1D::real_type Histogram_1D::normalized_sum_between(const real_type x,
							       const real_type y) const{
    
    return sum_between(x,y) / n_samples;
  }


  void Histogram_1D::compute_inverse(){
    
    normalized_inverse.resize(n_inverse_points);
    
    normalized_inverse.front() = clamp_min;
    normalized_inverse.back()  = clamp_max;

    size_type bottom_j = 0;
    
    for(size_type i=1;i<n_inverse_points;i++){
      
      const real_type y = i * normalized_inverse_delta;

      real_type bottom_y;
      do{
	bottom_j++;
	bottom_y = normalized_sum_to(first_value + bottom_j*bin_delta);
      }
      while(bottom_y<y);

      bottom_j--;
      const real_type top_y = bottom_y;
      bottom_y = normalized_sum_to(first_value + bottom_j*bin_delta);

      const real_type bottom_x = first_value + bottom_j * bin_delta;
      real_type alpha = (y-bottom_y) / (top_y-bottom_y);

      if (bottom_j==0) {
	alpha = 0.5*alpha + 0.5;
      }
      else if (bottom_j==n_bins-1){
	alpha *= 0.5;
      }

      normalized_inverse[i] = bottom_x + bin_delta * alpha;
    }
  }


  
  Histogram_1D::real_type Histogram_1D::inverse(const real_type x) const{

    using namespace std;

    if (x<=0) {
      return normalized_inverse.front();
    }
    
    if (x>=1) {
      return normalized_inverse.back();
    }
   
    const real_type i        = x/normalized_inverse_delta;
    const size_type bottom_i = static_cast<size_type>(i);
    const real_type alpha    = i - bottom_i;
  
    return (1.0 - alpha) * normalized_inverse[bottom_i] + alpha*normalized_inverse[bottom_i+1];    
  }


  

  void Histogram_1D::build_equalizing_lookup_table(const real_type hist_min,
						   const real_type hist_max,
						   Lookup_table_1D* const lookup) const{


    lookup->assign(clamp_min,clamp_max,n_bins+1);

    const real_type delta = (hist_max - hist_min) / n_samples;
    
    for(size_type i=0;i<=n_bins;i++){
      lookup->set_ith_sample_point(i,hist_min + delta*sum[i]);
    }
  }

  
  
  void Histogram_1D::build_transfer_lookup_table(const Histogram_1D& input_histogram,
						 const Histogram_1D& model_histogram,
						 Lookup_table_1D* const lookup){
    
    lookup->assign(input_histogram.min_value(),
		   input_histogram.max_value(),
		   model_histogram.inverse_sampling());

    for(size_type i=0,i_max=lookup->sampling();i<i_max;i++){
      lookup->set_ith_sample_point(i,model_histogram.inverse(input_histogram.normalized_sum_to(lookup->ith_sample_point(i))));
    }
  }

  

  void Histogram_1D::output_histogram_to_gnuplot(const char* file_name) const{

    std::ofstream out(file_name);
    
    for(size_type i=0;i<n_bins;i++){
      const real_type x = first_value + (i+0.5)*bin_delta;
      out<<x<<'\t'<<bin[i]<<'\n';
    }

    out.flush();
    out.close();
  }

  
  void Histogram_1D::output_normalized_histogram_to_gnuplot(const char* file_name) const{

    std::ofstream out(file_name);

    real_type sum = 0;
    
    for(size_type i=0;i<n_bins;i++){
      const real_type x = first_value + (i+0.5)*bin_delta;
      out<<x<<'\t'<<(static_cast<real_type>(bin[i])/n_samples)<<'\n';

      sum += static_cast<real_type>(bin[i])/n_samples;
    }

    std::cout<<sum<<std::endl;
    
    out.flush();
    out.close();
  }

  
  void Histogram_1D::output_normalized_sum_to_gnuplot(const char* file_name) const{
    
    std::ofstream out(file_name);
    
    for(size_type i=0;i<=10*n_bins;i++){
      const real_type x = first_value + i*bin_delta/10;
      out<<x<<'\t'<<normalized_sum_to(x)<<'\n';
    }

    out.flush();
    out.close();    
  }


  void Histogram_1D::output_normalized_inverse_to_gnuplot(const char* file_name) const{
    
    std::ofstream out(file_name);
    
    for(size_type i=0;i<normalized_inverse.size();i++){
      const real_type x = (i+0.5)*normalized_inverse_delta;
      out<<x<<'\t'<<normalized_inverse[i]<<'\n';
    }

    out.flush();
    out.close();    
  }

  void Histogram_1D::output_check_to_gnuplot(const char* file_name) const{

    std::ofstream out(file_name);
    
    for(size_type i=0;i<n_bins;i++){

      if (bin[i]>0){
	const real_type x = first_value + (i+0.5)*bin_delta;
	out<<x<<'\t'<<inverse(normalized_sum_to(x))<<'\n';
      }
    }

    out.flush();
    out.close();
  }


#ifndef NO_XML
  
  QDomElement Histogram_1D::to_DOM_element(const QString& name,
					   QDomDocument& document) const{

    QDomElement main_element = document.createElement(name);

    main_element.setAttribute("nBins",QString::number(n_bins));
    main_element.setAttribute("clampMin",QString::number(clamp_min));
    main_element.setAttribute("clampMax",QString::number(clamp_max));
    main_element.setAttribute("nInversePoints",QString::number(n_inverse_points));

    std::ostringstream out;

    for(size_type i=0;i<n_bins;i++){
      out<<bin[i]<<' ';
    }

    main_element.appendChild(document.createTextNode(QString::fromStdString(out.str())));

    return main_element;
  }

  

  void Histogram_1D::from_DOM_element(const QDomElement& element){

    QDomElement& elt = const_cast<QDomElement&>(element);
    
    n_bins = elt.attributeNode("nBins").value().toUInt();
    clamp_min = elt.attributeNode("clampMin").value().toDouble();
    clamp_max = elt.attributeNode("clampMax").value().toDouble();
    n_inverse_points = elt.attributeNode("nInversePoints").value().toUInt();
      
    bin_delta = (clamp_max - clamp_min) / (n_bins - 1);
    normalized_inverse_delta = 1.0 / (n_inverse_points-1);
    
    first_value = clamp_min - 0.5*bin_delta;
    last_value  = first_value + bin_delta * n_bins;
    
    bin.resize(n_bins);
    n_samples = 0;
    
    std::istringstream in(elt.text().toStdString());
    
    for(size_type i=0;i<n_bins;i++){
      in>>bin[i];
      n_samples += bin[i];
    }
    
    sum.assign(n_bins+1,0);
    for(size_type i=1;i<=n_bins;i++){
      sum[i] = sum[i-1] + bin[i-1];
    }
    
  }

  
#endif
    

  void Histogram_1D::set_default_number_of_bins(const size_type s){
    default_number_of_bins = s;
  }

  
  void Histogram_1D::set_default_inverse_oversampling(const size_type s){
    default_inverse_oversampling = s;
  }
 

  
  
  /*

  ###########################
  #                         #
  #  class Lookup_table_1D  #
  #                         #
  ###########################

  */



  Lookup_table_1D::Lookup_table_1D(const real_type support_min,
				   const real_type support_max,
				   const size_type number_of_samples):
  min_x(support_min),
  max_x(support_max),
  n_samples(number_of_samples),
  inv_delta((n_samples-1.0)/(max_x-min_x)),
  sample(n_samples){}

  
  void Lookup_table_1D::assign(const real_type support_min,
			       const real_type support_max,
			       const size_type number_of_samples){
    
    min_x     = support_min;
    max_x     = support_max;
    n_samples = number_of_samples;
    inv_delta = (n_samples-1.0)/(max_x-min_x);
    sample.resize(n_samples);
  }

  
  Lookup_table_1D::size_type Lookup_table_1D::sampling() const{

    return n_samples;
  }

  

  void Lookup_table_1D::resample(const size_type number_of_samples){

    const real_type new_inv_delta = (number_of_samples - 1.0) / (max_x - min_x);

    std::vector<real_type> new_sample(number_of_samples);

    for(size_type i=1,i_end=number_of_samples-1;i<i_end;i++){

      new_sample[i] = (*this)(min_x + i / new_inv_delta);
    }
    
    new_sample.front() = sample.front();
    new_sample.back()  = sample.back();

    std::swap(new_sample,sample);
    inv_delta = new_inv_delta;
    n_samples = number_of_samples;
  }

  

  Lookup_table_1D::real_type Lookup_table_1D::operator()(const real_type x) const{

    if (x<=min_x){
      return sample.front();
    }
    if (x>=max_x){
      return sample.back();
    }

    const real_type i = (x-min_x) * inv_delta;
    const size_type bottom_i = static_cast<size_type>(i);
    const real_type alpha = i - bottom_i;
    
    return (1.0-alpha)*sample[bottom_i] + alpha*sample[bottom_i+1];
    
  }

  
  Lookup_table_1D::real_type Lookup_table_1D::ith_sample_point(const size_type i) const{

    return min_x + i / inv_delta;
    
  }

  
  void Lookup_table_1D::set_ith_sample_point(const size_type i,const real_type x){

    sample[i] = x;
  }


  
  void Lookup_table_1D::constrain_variations(const real_type max_ratio,
					     const real_type min_ratio){

    typedef std::vector<real_type> value_array_type;
    
    const real_type average_variation =
      (sample.back() - sample.front()) / (sample.size() - 1.0);

    real_type max_var = max_ratio * average_variation;
    real_type min_var = min_ratio * average_variation;
//     real_type max_var = max_ratio / inv_delta;
//     real_type min_var = min_ratio / inv_delta;

    if (max_var < min_var){
      std::swap(max_var,min_var);
    }
    
    value_array_type truncated_variation(sample.size()-1,0);

    for(size_type i=0,i_end=truncated_variation.size();i<i_end;i++){

      truncated_variation[i] = Math_tools::clamp(min_var,max_var,sample[i+1]-sample[i]);
    }

    
    value_array_type target_variation(sample.size()-1,0);

    for(size_type i=1,i_end=target_variation.size();i<i_end;i++){

      target_variation[i] = truncated_variation[i] - truncated_variation[i-1];
    }


    const size_type n_loops = sample.size();

    value_array_type buffer(sample);
    
    for(size_type loop = 0;loop < n_loops;loop++){

     for(size_type i=1,i_end=target_variation.size();i<i_end;i++){

       sample[i] = buffer[i] + (buffer[i+1] + buffer[i-1] - 2*buffer[i] - target_variation[i]) * 0.5;
     } // END OF for i
     std::swap(buffer,sample);
    } // END OF for loop

  }

  

  void Lookup_table_1D::output_to_gnuplot(const char* file_name) const{
    
    std::ofstream out(file_name);
    
    for(size_type i=0;i<n_samples;i++){

      const real_type x = min_x + i/inv_delta;
      out<<x<<'\t'<<sample[i]<<'\n';
    }

    out.flush();
    out.close();
    
  }


  
  void Lookup_table_1D::set_to_identity(){
    
    for(size_type i=0;i<n_samples;i++){

      const real_type x = min_x + i/inv_delta;

      sample[i] = x;
    }
  }

}


#endif
