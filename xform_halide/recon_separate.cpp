#include <Halide.h>
#include "tools/resize.h"
#include "tools/color_transform.h"
#include "tools/resample.h"

using namespace Halide;

int main(int argc, char **argv){

Var x("x"), y("y"), xi("xi"), xo("xo"), yi("yi"), yo("yo"), c("c"),
    k("k"), ni("ni"), no("no");

  const int J = 5;
  const int nbins = 4;
  const int step = 16;
  const float scaleFactor = float(std::pow(2, J-1));
  bool stack = true;

  ImageParam input(Float(32), 3),
             ac_lumin_raw(Float(32), 2),
             ac_lumin_mins(Float(32), 2),
             ac_lumin_maxs(Float(32), 2),
             ac_chrom_raw(Float(32), 2),
             ac_chrom_mins(Float(32), 2),
             ac_chrom_maxs(Float(32), 2),
             dc(Float(32), 3);

  Param<int>  level;

  Func clamped("clamped");
  clamped(x, y, c) = input(clamp(x, 0, input.width()-1), clamp(y, 0, input.height()-1), c);

  Func my_yuv("yuv");
  my_yuv(x, y, c) = rgb2yuv(clamped)(x, y, c);
  //my_yuv(x, y, c) = clamped(x, y, c); //profiling

  // High-pass features
  Func ds("ds");
  Func ds_x("ds_x");
  ds_x(x, y, c) = resize_x(my_yuv, 1.0/scaleFactor)(x, y, c);
  ds(x, y, c) = resize_y(ds_x, 1.0/scaleFactor)(x, y, c);
  //ds(x,y,c) = downsample_n(my_yuv, J)(x,y,c);

  Func us_ds("us_ds");
  Func us_ds_x("us_ds_x");
  us_ds_x(x, y, c) = resize_x(ds, scaleFactor)(x, y, c);
  us_ds(x, y, c) = resize_y(us_ds_x, scaleFactor)(x, y, c);
  //us_ds(x, y, c) = upsample_n(ds, J)(x, y, c);

  Func hp("hp");
  hp(x, y, c) = my_yuv(x, y, c) - us_ds(x, y, c);

  // Laplacian features 
  Func lumin("lumin");
  lumin(x, y) = my_yuv(x, y, 0);


  Func gdPyramid [J];
  gdPyramid[0](x, y) = lumin(x, y);
  for (int j = 1; j < J; j++) {
      gdPyramid[j](x, y) = downsample(gdPyramid[j-1])(x, y);
  }

  Func laplacian[J-1];
  Func gaussian[J];
  Func lPyramid[J-1];
  if (stack){
    // Gaussian stack
    for(int i = 0; i < J; i++){
      gaussian[i](x, y) = upsample_n(gdPyramid[i], i+1)(x, y); 
    }
    for(int i = 0; i < J-1; i++)
      laplacian[i](x, y) = gaussian[i](x, y) - gaussian[i+1](x, y);
  }else{
    for (int j = J-2; j >= 0; j--) {
        lPyramid[j](x, y) = gdPyramid[j](x,y) - upsample(gdPyramid[j+1])(x, y);
    }
  }

  // Lumin curve features
  Func lumin_hp("lumin_hp");
  lumin_hp(x, y) = hp(x, y, 0);
  Func curve_feat[nbins-1];
  RDom r(0, step, 0, step);
  Func maxi("maxi"), mini("mini");
  maxi(x, y) = maximum(lumin_hp(step * x + r.x, step * y + r.y));
  mini(x, y) = minimum(lumin_hp(step * x + r.x, step * y + r.y));
  Func range("range"); 
  range(x, y) = maxi(x, y) - mini(x, y);
  for(int i = 0; i < nbins - 1; i++){
    Func thresh("thresh"); 
    thresh(x, y) = static_cast<float>(i+1) * range(x, y) / static_cast<float>(nbins) + mini(x, y);
    curve_feat[i](x, y) = max(lumin_hp(x, y) - thresh(x/step, y/step), 0); 
  }

  // De-quantization
  Func ac_lumin_range("ac_lumin_range");
  ac_lumin_range(no, ni) = ac_lumin_maxs(no, ni) - ac_lumin_mins(no, ni);
  Func ac_lumin("ac_lumin");
  const int n_chan_o_lumin = 1, n_chan_i_lumin = 4 + nbins - 1 + J - 1;
  ac_lumin(x, y) = ac_lumin_raw(x, y) *  
    ac_lumin_range((x*n_chan_o_lumin)/ac_lumin_raw.width(), (y*n_chan_i_lumin)/ac_lumin_raw.height()) + 
      ac_lumin_mins((x*n_chan_o_lumin)/ac_lumin_raw.width(), (y*n_chan_i_lumin)/ac_lumin_raw.height());

  Func ac_chrom_range("ac_chrom_range");
  ac_chrom_range(no, ni) = ac_chrom_maxs(no, ni) - ac_chrom_mins(no, ni);
  Func ac_chrom("ac_chrom");
  const int n_chan_o_chrom = 2, n_chan_i_chrom = 4;
  ac_chrom(x, y) = ac_chrom_raw(x, y) * 
    ac_chrom_range((x*n_chan_o_chrom)/ac_chrom_raw.width(), (y*n_chan_i_chrom)/ac_chrom_raw.height()) + 
    ac_chrom_mins((x*n_chan_o_chrom)/ac_chrom_raw.width(), (y*n_chan_i_chrom)/ac_chrom_raw.height());
  
  // Reconstruct each patch (luminance channel)
  Expr offset_y = ac_lumin_raw.height()/(n_chan_i_lumin);
  Expr offset_x = ac_lumin_raw.width();
  
  // Reduce Laplacian coefficients
  Func reduced_laplacian[J-1]; 
  if (stack){ 
    for(int i = 0; i < J - 1; i++)
      reduced_laplacian[i](x, y) = laplacian[i](x, y) * ac_lumin(x/step, y/step + offset_y * (4 + i));

    for(int i = 1; i < J - 1; i++)
      reduced_laplacian[i](x, y) = reduced_laplacian[i](x, y) + reduced_laplacian[i-1](x,y);
  }else{ // Pyramid 

    reduced_laplacian[J-2](x, y) = lPyramid[J-2](x, y) * 
      ac_lumin(clamp(x, 0, offset_x-1), clamp(y + offset_y*(4+J-2), offset_y*(4+J-2), offset_y*(4+J-1)-1));
    for(int i = J-3; i >= 0; i--){
      int scale = std::pow(2, J-1-i);
      reduced_laplacian[i](x, y) = upsample(reduced_laplacian[i+1])(x, y) + 
            lPyramid[i](x, y) * 
            ac_lumin(clamp(x/scale, 0, offset_x-1), clamp(y/scale + offset_y * (4+i), offset_y*(4+i), offset_y*(4+i+1)-1)); 
    }
  }

  // Reduce Histogram Coefficients
  Func reduced_curve_feat[nbins-1]; 
  for(int i = 0; i < nbins - 1; i++)
    reduced_curve_feat[i](x, y) = curve_feat[i](x, y) * ac_lumin(x/step, y/step + offset_y * (4 + J - 1 + i));

  for(int i = 1; i < nbins - 1; i++)
    reduced_curve_feat[i](x, y) = reduced_curve_feat[i](x, y) + reduced_curve_feat[i-1](x,y);

  // Put together
  Func lumin_out("lumin_out");
  RDom z(0, 3);
  lumin_out(x, y) =   hp(x, y, 0 ) * ac_lumin(x/step, y/step + offset_y * 0) + 
                      hp(x, y, 1 ) * ac_lumin(x/step, y/step + offset_y * 1) + 
                      hp(x, y, 2 ) * ac_lumin(x/step, y/step + offset_y * 2) + 
                                     ac_lumin(x/step, y/step + offset_y * 3) +
                                     reduced_laplacian[0](x, y) + 
                                     reduced_curve_feat[nbins-2](x, y); 
  Func u_out("u_out");
  u_out(x, y)  =  sum(hp(x, y, z) * ac_chrom(x/step + offset_x * 0, y/step + offset_y * z)) + 
                            ac_chrom(x/step + offset_x * 0, y/step + offset_y * 3);  

  Func v_out("v_out");
  v_out(x, y) =  sum(hp(x, y, z) * ac_chrom(x/step + offset_x * 1, y/step + offset_y * z)) + 
                            ac_chrom(x/step + offset_x * 1, y/step + offset_y * 3);  
  Expr yy = lumin_out(x, y);
  Expr uu = u_out(x, y);
  Expr vv = v_out(x, y);

  Func yuv_out("yuv_out");
  yuv_out(x,y,c) = select(c == 0, yy, c == 1, uu,  vv);

  // YUV2RGB
  Func rgb_out("rgb_out");
  rgb_out(x, y, c) = yuv2rgb(yuv_out)(x, y, c);
  //rgb_out(x, y, c) = yuv2rgb(clamped)(x, y, c); for profiling

  // Put the upsampled DC back to predicted highpass
  Func clamped_dc("clamped_dc");
  clamped_dc(x, y, c) = dc(clamp(x, 0, dc.width()-1), clamp(y, 0, dc.height()-1), c);

  Func new_dc("new_dc");
  Func new_dc_x("new_dc_x");
  new_dc_x(x, y, c) = resize_x(clamped_dc, scaleFactor)(x, y, c);
  new_dc(x, y, c) = resize_y(new_dc_x, scaleFactor)(x, y, c);
  //new_dc_x(x, y, c) = upsample_n(clamped_dc, 5)(x, y, c);
  //new_dc(x, y, c) = upsample_n(clamped_dc, 5)(x, y, c);

  Func final("final");
  final(x, y, c) = clamp(new_dc(x, y, c)  + rgb_out(x, y, c), 0.0f, 1.0f);
  //final(x, y, c) = my_yuv(x, y, c);
  //final(x, y, c) = hp(x, y, c);
  //final(x, y, c) = reduced_laplacian[0](x, y);
  //final(x, y, c) = reduced_curve_feat[0](x, y);
  //final(x, y, c) = lumin_out(x, y);
  //final(x, y, c) = u_out(x, y);
  //final(x, y, c) = v_out(x, y);
  //final(x, y, c) = yuv_out(x, y, c);
  //final(x, y, c) = rgb_out(x, y, c);
  //final(x, y, c) = new_dc(x, y, c);

  /* Scheduling */
  final.split(y, yo, yi, 32).parallel(yo).vectorize(x, 8);
  new_dc_x.store_at(final, yo).compute_at(new_dc, y).vectorize(x, 8);
  new_dc.compute_at(final, yo);

  // Lumin features
  yuv_out.compute_root();
  yuv_out.split(y, yo, yi, 32).parallel(yo).vectorize(x, 8);
  maxi.compute_at(yuv_out, yo);
  mini.compute_at(yuv_out, yo);
  ac_chrom.compute_root();
  ac_lumin.compute_root();

  //Highpass
  hp.compute_root();
  hp.split(y, yo, yi, 32).parallel(yo).vectorize(x, 8);
  us_ds_x.store_at(hp, yo).compute_at(hp, yi).vectorize(x, 8);
  ds.compute_root();
  ds.split(y, yo, yi, 16).parallel(yo).vectorize(x, 4); 
  ds_x.store_at(ds, yo).compute_at(ds, yi).vectorize(x, 4);

  // Laplacian Coeffificents
  for(int i = 0; i < J; i++){
    gdPyramid[i].compute_root();
    gdPyramid[i].parallel(y, 8).vectorize(x, 8);
  }
  if (!stack){
    for(int i = 0; i < J-1; i++){
      reduced_laplacian[i].compute_root();
      reduced_laplacian[i].parallel(y, 8).vectorize(x, 8);
    }
    for(int i = 0; i < J-1; i++){
      lPyramid[i].compute_root();
      lPyramid[i].parallel(y, 8).vectorize(x, 8);
    }
  }
  std::vector<Argument> args(9);
  args[0] = input;
  args[1] = level;
  args[2] = ac_lumin_raw;
  args[3] = ac_lumin_mins;
  args[4] = ac_lumin_maxs;
  args[5] = ac_chrom_raw;
  args[6] = ac_chrom_mins;
  args[7] = ac_chrom_maxs;
  args[8] = dc;
  final.compile_to_file("halide_recon_separate", args);

  return 0;
}
