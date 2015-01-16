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
int main(int argc, char **argv){

  Var x, y, c, yi, yo, xi, xo, ni, no;
  const int J = 5;
  const int nbins = 4;

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

  // High-pass features
  Func ds("ds");
  ds(x,y,c) = downsample_n(clamped, J)(x,y,c);

  Func hp("hp");
  hp(x, y, c) = clamped(x, y, c) - upsample_n(ds, J)(x, y, c);

  // Laplacian features 
  Func lumin("lumin");
  lumin(x, y) = clamped(x, y, 0);

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
  //RDom r(0, input.width(), 0, input.height());
  RDom r(0, 5, 0, 5);
  Expr maxi = maximum(lumin_hp(r.x, r.y));
  Expr mini = minimum(lumin_hp(r.x, r.y));
  Expr range = maxi - mini;
  for(int i = 0; i < nbins - 1; i++){
    Expr thresh = 
      static_cast<float>(i+1) * range / static_cast<float>(nbins) + mini;
    curve_feat[i](x, y) = max(lumin_hp(x, y) - thresh, 0); 
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
  const int n_chan_o_chrom = 1, n_chan_i_chrom = 4;
  ac_chrom(x, y) = ac_chrom_raw(x, y) * 
    ac_chrom_range((x*n_chan_o_chrom)/ac_chrom_raw.width(), (y*n_chan_i_chrom)/ac_chrom_raw.height()) + 
    ac_chrom_mins((x*n_chan_o_chrom)/ac_chrom_raw.width(), (y*n_chan_i_chrom)/ac_chrom_raw.height());
  
  // Reconstruct each patch (luminance channel)
  const int step = 32;
  Expr offset = ac_lumin_raw.height()/(n_chan_i_lumin);
  
  // Reduce Laplacian coefficients
  Func reduced_laplacian[J-1]; 
  for(int i = 0; i < J - 1; i++)
    reduced_laplacian[i](x, y) = laplacian[i](x, y) * ac_lumin(x/step, y/step + offset * (4 + i));

  for(int i = 1; i < J - 1; i++)
    reduced_laplacian[i](x, y) = reduced_laplacian[i](x, y) + reduced_laplacian[i-1](x,y);

  // Reduce Histogram Coefficients
  Func reduced_curve_feat[nbins-1]; 
  for(int i = 0; i < nbins - 1; i++)
    reduced_curve_feat[i](x, y) = curve_feat[i](x, y) * ac_lumin(x/step, y/step + offset * (4 + J - 1 + i));

  for(int i = 1; i < nbins - 1; i++)
    reduced_curve_feat[i](x, y) = reduced_curve_feat[i](x, y) + reduced_curve_feat[i-1](x,y);

  Func lumin_out("lumin_out");
  RDom z(0, 3);
  lumin_out(x, y) = sum(hp(x, y, z) * ac_lumin(x/step, y/step + offset * z)) + 
                                      ac_lumin(x/step, y/step + offset * 3)  + // affine term 
                                      reduced_laplacian[J-2](x, y) + 
                                      reduced_curve_feat[nbins-2](x, y); 

  // Contribution of high-pass features
  // Contribution of Laplacian features
  // Contribution of curve features
  

  // Reconstruct each patch (chrominance channel)
  // Contribution of high-pass features
  

  Func final("final");
  //final(x, y) = 0.5 + curve_feat[2](x, y);
  final(x, y) = lumin_out(x, y) + 0.5;
  //final(x, y) = reduced_laplacian[0](x, y);

  /* Scheduling */
  //final.tile(x, y, xo, xi, yo, yi, 16, 32).parallel(yo).vectorize(xi,8);
  
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
