#include <Halide.h>
using namespace Halide;

Var x("x"), y("y"), xi("xi"), xo("xo"), yi("yi"), yo("yo"), c("c"),
    k("k"), ni("ni"), no("no");

enum InterpolationType {
    BOX, LINEAR, CUBIC, LANCZOS
};

Expr kernel_box(Expr x) {
    Expr xx = abs(x);
    return select(xx <= 0.5f, 1.0f, 0.0f);
}

Expr kernel_linear(Expr x) {
    Expr xx = abs(x);
    return select(xx < 1.0f, 1.0f - xx, 0.0f);
}

Expr kernel_cubic(Expr x) {
    Expr xx = abs(x);
    Expr xx2 = xx * xx;
    Expr xx3 = xx2 * xx;
    float a = -0.5f;

    return select(xx < 1.0f, (a + 2.0f) * xx3 - (a + 3.0f) * xx2 + 1,
                  select (xx < 2.0f, a * xx3 - 5 * a * xx2 + 8 * a * xx - 4.0f * a,
                          0.0f));
}

Expr sinc(Expr x) {
    return sin(float(M_PI) * x) / x;
}

Expr kernel_lanczos(Expr x) {
    Expr value = sinc(x) * sinc(x/3);
    value = select(x == 0.0f, 1.0f, value); // Take care of singularity at zero
    value = select(x > 3 || x < -3, 0.0f, value); // Clamp to zero out of bounds
    return value;
}

struct KernelInfo {
    const char *name;
    float size;
    Expr (*kernel)(Expr);
};

static KernelInfo kernelInfo[] = {
    { "box", 0.5f, kernel_box },
    { "linear", 1.0f, kernel_linear },
    { "cubic", 2.0f, kernel_cubic },
    { "lanczos", 3.0f, kernel_lanczos }
};
InterpolationType interpolationType = LINEAR;

Func resize_x(Func f, float scaleFactor){

  float kernelScaling = std::min(scaleFactor, 1.0f);
  float kernelSize = kernelInfo[interpolationType].size / kernelScaling;
  Expr sourcex = (x + 0.5f) / scaleFactor;
  Func kernelx("kernelx"); 
  Expr beginx = cast<int>(sourcex - kernelSize + 0.5f);
  RDom domx(0, static_cast<int>(2.0f * kernelSize) + 1, "domx");
  {
        const KernelInfo &info = kernelInfo[interpolationType];
        Func kx; 
        kx(x, k) = info.kernel((k + beginx - sourcex) * kernelScaling);
        kernelx(x, k) = kx(x, k) / sum(kx(x, domx));
  }
  Func resized_x("resized_x");
  
  kernelx.compute_root();
  resized_x(x, y, c) = sum(kernelx(x, domx) * cast<float>(f(domx + beginx, y, c)));
  return resized_x;
}
Func resize_y(Func f, float scaleFactor){

    float kernelScaling = std::min(scaleFactor, 1.0f);
    float kernelSize = kernelInfo[interpolationType].size / kernelScaling;
    Expr sourcey = (y + 0.5f) / scaleFactor;
    Func kernely("kernely");
    Expr beginy = cast<int>(sourcey - kernelSize + 0.5f);
    RDom domy(0, static_cast<int>(2.0f * kernelSize) + 1, "domy");
    {
        const KernelInfo &info = kernelInfo[interpolationType];
        Func ky;
        ky(y, k) = info.kernel((k + beginy - sourcey) * kernelScaling);
        kernely(y, k) = ky(y, k) / sum(ky(y, domy));
    }
    Func resized_y("resized_y");
    resized_y(x, y, c) = sum(kernely(y, domy) * f(x, domy + beginy, c));

    kernely.compute_root();
    return resized_y;
}











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
  const int step = 16;
  const float scaleFactor = float(std::pow(2, J-1));


  ImageParam input(Float(32), 3), 
             lp_input(Float(32), 3);
  Param<int>  level;

  Func clamped("clamped");
  clamped(x, y, c) = input(clamp(x, 0, input.width()-1), clamp(y, 0, input.height()-1), c);

  Func my_yuv("yuv");
  my_yuv(x, y, c) = rgb2yuv(clamped)(x, y, c);

  Func clamped_lp("clamped_lp");
  clamped_lp(x, y, c) = lp_input(clamp(x, 0, lp_input.width()-1), clamp(y, 0, lp_input.height()-1), c);

  Func lp_yuv("yuv");
  lp_yuv(x, y, c) = rgb2yuv(clamped_lp)(x, y, c);

  Func us_ds("us_ds");
  Func us_ds_x("us_ds_x");
  us_ds_x(x, y, c) = resize_x(lp_yuv, scaleFactor)(x, y, c);
  us_ds(x, y, c) = resize_y(us_ds_x, scaleFactor)(x, y, c);

  Func hp("hp");
  hp(x, y, c) = my_yuv(x, y, c) - us_ds(x, y, c);

  /* Scheduling */
  hp.split(y, yo, yi, 16).parallel(yo).vectorize(x, 8);
  us_ds_x.store_at(hp, yo).compute_at(hp, yi);
  
  std::vector<Argument> args(2);
  args[0] = input;
  args[1] = lp_input;
  hp.compile_to_file("halide_highpass", args);

  return 0;
}
