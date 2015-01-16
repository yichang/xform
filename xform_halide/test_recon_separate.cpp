#include <fstream>
#include "halide_recon_separate.h"
#include "static_image.h"
#include "image_io.h"
#include <iostream>
#include <sys/time.h>
using namespace std;

int main(int argc, char** argv){
  
  Image<float> input = load<float>(argv[1]);
  Image<float> ac_lumin_raw = load<float>(argv[2]);
  Image<float> ac_chrom_raw = load<float>(argv[3]);
  Image<float> dc = load<float>(argv[4]);

  const int num_lumin_feat = 11;
  const int num_chrom_feat = 4;
  const int meta_len = 2 * (num_lumin_feat + 2 * num_chrom_feat);
  std::ofstream out_file;                                                   
  float* meta = new float[meta_len];
  out_file.open(argv[5]);                                                
  for(int i=0; i < meta_len; i++)
    out_file<<meta[i]<<" ";                              
  out_file.close();                                   

  Image<float> ac_lumin_mins(1, num_lumin_feat), 
              ac_lumin_maxs(1, num_lumin_feat);
  int offset=0;
  for(int y= 0; y < ac_lumin_mins.height(); y++){
    for(int x = 0; x < ac_lumin_mins.width(); x++){ 
      ac_lumin_mins(x,y) = meta[4*y + x];
      ac_lumin_maxs(x,y) = meta[4*y + x + num_lumin_feat];
      offset++;
    }}

  Image<float> ac_chrom_mins(2, num_chrom_feat), 
               ac_chrom_maxs(2, num_chrom_feat);
  for(int y= 0; y < ac_chrom_mins.height(); y++){
    for(int x = 0; x < ac_chrom_mins.width(); x++){ 
      ac_chrom_mins(x,y) = meta[4*y + x + offset];
      ac_chrom_maxs(x,y) = meta[4*y + x + 2 * num_chrom_feat + offset];
    }}

  const int level = 3;
  Image<float> output(input.width(), input.height(),input.channels());

  halide_recon_separate(input, level, 
            ac_lumin_raw, ac_lumin_mins, ac_lumin_maxs,
            ac_chrom_raw, ac_chrom_mins, ac_chrom_maxs,
            dc ,
            output);

  save(output, argv[6]);
}
