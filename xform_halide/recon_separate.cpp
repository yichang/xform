#include <Halide.h>
using namespace Halide;

Var x, y, yi, yo, c;
// Downsample with a 1 3 3 1 filter
Func downsample(Func f) {
    Func downx, downy;

    downx(x, y, _) = (f(2*x-1, y, _) + 3.0f * (f(2*x, y, _) + f(2*x+1, y, _)) + f(2*x+2, y, _)) / 8.0f;
    downy(x, y, _) = (downx(x, 2*y-1, _) + 3.0f * (downx(x, 2*y, _) + downx(x, 2*y+1, _)) + downx(x, 2*y+2, _)) / 8.0f;

    return downy;
}
// Upsample using bilinear interpolation
Func upsample(Func f) {
    Func upx, upy;

    upx(x, y, _) = 0.25f * f((x/2) - 1 + 2*(x % 2), y, _) + 0.75f * f(x/2, y, _);
    upy(x, y, _) = 0.25f * upx(x, (y/2) - 1 + 2*(y % 2), _) + 0.75f * upx(x, y/2, _);

    return upy;
}
Func downsample_n(Func f, const int J){
  Func* gdPyramid = new Func[J];
  gdPyramid[0](x, y, _) = f(x, y, _);
  for (int j = 1; j < J; j++) {
      gdPyramid[j](x, y, _) = downsample(gdPyramid[j-1])(x, y, _);
  }
  for(int i = 0; i < J; i++){
    gdPyramid[i].compute_root();
    gdPyramid[i].split(y, yo, yi, 16).parallel(yo).vectorize(x, 8);
  }
  return gdPyramid[J-1];
}
Func upsample_n(Func f, const int J){
  Func* guPyramid = new Func[J];
  guPyramid[J-1](x, y, _) = f(x, y, _);
  for (int j = J-1; j > 0; j--) {
      guPyramid[j-1](x, y, _) = upsample(guPyramid[j])(x, y, _);
  }
  for(int i = 0; i < J; i++){
    guPyramid[i].compute_root();
    guPyramid[i].split(y, yo, yi, 16).parallel(yo).vectorize(x, 8);
  }
  return guPyramid[0];
}
Func gaussian_stack(Func f, const int j){
  Func ds;
  ds(x, y, _) = downsample_n(f, j)(x, y, _);
  Func us;
  us(x, y, _) = upsample_n(ds, j)(x, y, _);
  return us;
}
Func yuv2rgb(Func yuv){
  Func rgb("rgb");
  rgb(x, y, c) = yuv(x, y, c);
  rgb(x, y, 0) = yuv(x, y, 0)                            + 1.140 * yuv(x, y, 2);
  rgb(x, y, 1) = yuv(x, y, 0) - 0.395 * yuv(x, y, 1) - 0.581 * yuv(x, y, 2); 
  rgb(x, y, 2) = yuv(x, y, 0) + 2.032 * yuv(x, y, 1); 
  return rgb;
}
Func rgb2yuv(Func rgb_){
  Func yuv_("yuv_");
  yuv_(x, y, c) = rgb_(x, y, c);
  yuv_(x, y, 0) =  cast<float>(0.299) * rgb_(x, y, 0) + 0.587 * rgb_(x, y, 1) + 0.144 * rgb_(x, y, 2); 
  yuv_(x, y, 1) = cast<float>(-0.147) * rgb_(x, y, 0) - 0.289 * rgb_(x, y, 1) + 0.436 * rgb_(x, y, 2); 
  yuv_(x, y, 2) =  cast<float>(0.615) * rgb_(x, y, 0) - 0.515 * rgb_(x, y, 1) - 0.100 * rgb_(x, y, 2); 
  return yuv_;
}
int main(int argc, char **argv){

  const int J = 5;
  const int nbins = 4;
  const int step = 16;

  Var x, y, c, yi, yo, xi, xo, ni, no;

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

  // High-pass features
  Func ds("ds");
  ds(x,y,c) = downsample_n(my_yuv, J)(x,y,c);

  Func hp("hp");
  hp(x, y, c) = my_yuv(x, y, c) - upsample_n(ds, J)(x, y, c);

  // Laplacian features 
  Func lumin("lumin");
  lumin(x, y) = my_yuv(x, y, 0);

  Func gaussian[J];
  for(int i = 0; i < J; i++)
    gaussian[i](x, y) = gaussian_stack(lumin, i+1)(x, y);

  Func laplacian[J-1];
  for(int i = 0; i < J-1; i++)
    laplacian[i](x, y) = gaussian[i](x, y) - gaussian[i+1](x, y);

  // Lumin curve features
  Func lumin_hp("lumin_hp");
  lumin_hp(x, y) = hp(x, y, 0);
  Func curve_feat[nbins-1];
  RDom r(0, input.width(), 0, input.height());
  Func maxi("maxi"), mini("mini");
  maxi() = maximum(lumin_hp(r.x, r.y));
  mini() = minimum(lumin_hp(r.x, r.y));
  Func range("range"); 
  range() = maxi() - mini();
  for(int i = 0; i < nbins - 1; i++){
    Func thresh("thresh"); 
    thresh() = static_cast<float>(i+1) * range() / static_cast<float>(nbins) + mini();
    curve_feat[i](x, y) = max(lumin_hp(x, y) - thresh(), 0); 
  }

  // De-quantization
  Func ac_lumin_range("ac_lumin_range");
  ac_lumin_range(no, ni) = ac_lumin_maxs(no, ni) - ac_lumin_mins(no, ni);
  Func ac_lumin("ac_lumin");
  const int n_chan_o_lumin = 1, n_chan_i_lumin = 4 + nbins - 1 + J - 1;
  ac_lumin(x, y) = ac_lumin_raw(x, y) +  
    ac_lumin_range((x*n_chan_o_lumin)/ac_lumin_raw.width(), (y*n_chan_i_lumin)/ac_lumin_raw.height()) + 
      ac_lumin_mins((x*n_chan_o_lumin)/ac_lumin_raw.width(), (y*n_chan_i_lumin)/ac_lumin_raw.height());

  Func ac_chrom_range("ac_chrom_range");
  ac_chrom_range(no, ni) = ac_chrom_maxs(no, ni) - ac_chrom_mins(no, ni);
  Func ac_chrom("ac_chrom");
  const int n_chan_o_chrom = 1, n_chan_i_chrom = 4;
  ac_chrom(x, y) = ac_chrom_raw(x, y) * 
    ac_chrom_range((x*n_chan_o_chrom)/ac_chrom_raw.width(), (y*n_chan_i_chrom)/ac_chrom_raw.height()) + 
    ac_chrom_mins((x*n_chan_o_chrom)/ac_chrom_raw.width(), (y*n_chan_i_chrom)/ac_chrom_raw.height());
  
  // Reconstruct each patch (luminance channel)
  Expr offset_y = ac_lumin_raw.height()/(n_chan_i_lumin);
  Expr offset_x = ac_lumin_raw.width();
  
  // Reduce Laplacian coefficients
  Func reduced_laplacian[J-1]; 
  for(int i = 0; i < J - 1; i++)
    reduced_laplacian[i](x, y) = laplacian[i](x, y) * ac_lumin(x/step, y/step + offset_y * (4 + i));

  for(int i = 1; i < J - 1; i++)
    reduced_laplacian[i](x, y) = reduced_laplacian[i](x, y) + reduced_laplacian[i-1](x,y);

  // Reduce Histogram Coefficients
  Func reduced_curve_feat[nbins-1]; 
  for(int i = 0; i < nbins - 1; i++)
    reduced_curve_feat[i](x, y) = curve_feat[i](x, y) * ac_lumin(x/step, y/step + offset_y * (4 + J - 1 + i));

  for(int i = 1; i < nbins - 1; i++)
    reduced_curve_feat[i](x, y) = reduced_curve_feat[i](x, y) + reduced_curve_feat[i-1](x,y);

  // Put together
  Func lumin_out("lumin_out");
  RDom z(0, 3);
  lumin_out(x, y) = sum(hp(x, y, z) *  ac_lumin(x/step, y/step + offset_y * z)) + 
                                       ac_lumin(x/step, y/step + offset_y * 3) +
                                       reduced_laplacian[J-2](x, y) + 
                                       reduced_curve_feat[nbins-2](x, y); 
  // Now for chrominance channel
  Func chrom_out("chrom_out");
  chrom_out(x, y, c) =  sum(hp(x, y, z) * ac_chrom(x/step + offset_x * c, y/step + offset_y * z)) + 
                                          ac_chrom(x/step + offset_x * c, y/step + offset_y * 3);  
  // Combine Y and UV
  Func yuv_out("yuv_out");
  yuv_out(x, y, c) = my_yuv(x, y, c);
  yuv_out(x, y, 0) = lumin_out(x, y);
  yuv_out(x, y, 1) = chrom_out(x, y, 0);
  yuv_out(x, y, 2) = chrom_out(x, y, 1);

  // YUV2RGB
  Func rgb_out("rgb_out");
  rgb_out(x, y, c) = yuv2rgb(yuv_out)(x, y, c);

  // Put the upsampled DC back to predicted highpass
  Func clamped_dc("clamped_dc");
  clamped_dc(x, y, c) = dc(clamp(x, 0, dc.width()-1), clamp(y, 0, dc.height()-1), c);

  Func new_dc("new_dc");
  new_dc(x, y, c) = upsample_n(clamped_dc, 5)(x, y, c);

  Func final("final");
  final(x, y, c) = clamp(new_dc(x, y, c)  + rgb_out(x, y, c), 0.0f, 1.0f);

  /* Scheduling */
  //final.tile(x, y, xo, xi, yo, yi, 16, 32).parallel(yo).vectorize(xi,8);
  maxi.compute_root();
  mini.compute_root();
  range.compute_root();
  
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