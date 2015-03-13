#ifndef HIST_H_EECZYFI7
#define HIST_H_EECZYFI7

#include "const.h"
#include <vector>
#include <algorithm>

using namespace std;

class Histogram
{
public:
    Histogram(float* data, int n_elements);
    void compute_inverse();
    float sum_to(const float x ) const;
    float normalized_sum_to(const float x ) const;
    // virtual ~Histogram();
    //
    static void build_transfer_lut(const Histogram& input_histogram,
                                   const Histogram& model_histogram,
                                   vector<float> &lut,
                                   float &inv_delta, float &clam_min);
    float inverse(const float x) const;

    int n_samples;
    float clamp_min;
    float clamp_max;
    float bin_delta;
    float first_value;
    float last_value;
    vector<float> normalized_inverse;
private:
    vector<float> bin;
    vector<float> sum;
    /* data */
};

#endif /* end of include guard: HIST_H_EECZYFI7 */

