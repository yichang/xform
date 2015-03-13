#include "hist.h"

#include <iostream>

using namespace std;

Histogram::Histogram(float* data, int n_elements){
    float* begin = data;
    float* end = data+n_elements;
    clamp_min = *std::min_element(begin,end);
    clamp_max = *std::max_element(begin,end);

    bin_delta = (clamp_max - clamp_min) / (n_bins - 1);

    first_value = clamp_min - 0.5*bin_delta;
    last_value  = first_value + bin_delta * n_bins;

    n_samples = 0;
    bin.assign(n_bins,0);
    sum.assign(n_bins+1,0);

    for(float* i=begin; i!= end; i++){
      const int index = static_cast<int>((*i - first_value)/bin_delta);
      bin[index]++;
      n_samples++;
    }

    // cumsum
    for(int i=1;i<=n_bins;i++){
      sum[i] = sum[i-1] + bin[i-1];
    }
}

float Histogram::inverse(const float x) const {
    if (x<=0) {
      return normalized_inverse.front();
    }
    
    if (x>=1) {
      return normalized_inverse.back();
    }
   
    const float i        = x/normalized_inverse_delta;
    const int bottom_i = static_cast<int>(i);
    const float alpha    = i - bottom_i;
  
    return (1.0 - alpha) * normalized_inverse[bottom_i] + alpha*normalized_inverse[bottom_i+1];    
}

void Histogram::compute_inverse(){
    normalized_inverse.resize(n_inverse_points);

    normalized_inverse.front() = clamp_min;
    normalized_inverse.back()  = clamp_max;

    int bottom_j = 0;

    for(int i=1;i<n_inverse_points;i++){

        const float y = i * normalized_inverse_delta;

        float bottom_y;
        do{
            bottom_j++;
            bottom_y = normalized_sum_to(first_value + bottom_j*bin_delta);
        }
        while(bottom_y<y);

        bottom_j--;
        const float top_y = bottom_y;
        bottom_y = normalized_sum_to(first_value + bottom_j*bin_delta);

        const float bottom_x = first_value + bottom_j * bin_delta;
        float alpha = (y-bottom_y) / (top_y-bottom_y);

        if (bottom_j==0) {
            alpha = 0.5*alpha + 0.5;
        }
        else if (bottom_j==n_bins-1){
            alpha *= 0.5;
        }

        normalized_inverse[i] = bottom_x + bin_delta * alpha;
    }
}

float Histogram::normalized_sum_to(const float x) const{
    return sum_to(x) / n_samples;
}

float Histogram::sum_to(const float x) const{
    if (x<=clamp_min){
      return 0.0;
    }
    
    if (x>=clamp_max){
      return static_cast<float>(n_samples);
    }

    const float i        = (x - first_value)/bin_delta;
    const int bottom_i = static_cast<int>(i);
    float alpha          = i - bottom_i;

    if (bottom_i==0) {
      alpha = 2*(alpha - 0.5);
    }
    else if (bottom_i==n_bins-1){
      alpha *= 2;
    }

    return (1.0 - alpha) * sum[bottom_i] + alpha*sum[bottom_i+1];
}

void Histogram::build_transfer_lut(const Histogram& input_histogram,
                               const Histogram& model_histogram,
                               vector<float> &lut, float &inv_delta, float &clamp_min)
{
    lut.assign(n_inverse_points,0);
    float min = model_histogram.clamp_min;
    float max = model_histogram.clamp_max;
    float rng = max-min;
    clamp_min = model_histogram.clamp_min;
    for(int i=0;i<n_inverse_points;i++){
        inv_delta = n_inverse_points/(input_histogram.clamp_max-input_histogram.clamp_min);
        float sample_point = input_histogram.clamp_min + i/inv_delta;
        lut[i] = model_histogram.inverse(input_histogram.normalized_sum_to(sample_point));
        // lut[i] *= rng;
        // lut[i] += min;
    }
}
