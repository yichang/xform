#include <algorithm>
#include "util.h"
#include "png.hpp"
#include "jpeglib.h"
#include <string>
#include <regex>

using namespace std;

namespace xform{

inline PixelType clamp(PixelType val){
    return std::max(std::min(val, PIX_UPPER_BOUND), PIX_LOWER_BOUND);
}

/* Implment through png++ */
bool imread(const string& filename, ImageType_3* image){
    std::regex jpg_regex(".*jpg");
    std::regex png_regex(".*png");
    if(regex_match(filename, jpg_regex)){
        read_jpeg(filename, image);
    }else if(regex_match(filename, png_regex)){
        //TODO(yichang): replace reference with pointer
        // TODO(yichang): error handling when sizes are not equal.
        png::image<png::rgb_pixel> buf_image(filename);
        int height = buf_image.get_height();
        int width = buf_image.get_width();

        for(int i=0; i<image->rows(); i++){
            (*image)(i) = ImageType_1(height, width);
        }

        png::rgb_pixel pix; 
        for(int i=0; i<height; i++){
            for(int j=0; j<width; j++){
                pix = buf_image.get_pixel(j, i);
                (*image)(0)(i,j) = static_cast<PixelType>(pix.red)/PNG_RANGE;
                (*image)(1)(i,j) = static_cast<PixelType>(pix.green)/PNG_RANGE;
                (*image)(2)(i,j) = static_cast<PixelType>(pix.blue)/PNG_RANGE;
            }
        }
    }

    return true;
}


bool write_jpeg(const ImageType_3& image, const string& filename, int quality) {
    printf("write jpg\n");
    FILE* pFile = fopen(filename.c_str(), "wb");
    if (!pFile)
        return false;

    jpeg_compress_struct cinfo;
    jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, pFile);

    int height = image(0).rows();
    int width  = image(0).cols();
    int n_chan = 3;
    cinfo.image_width      = width;
    cinfo.image_height     = height;
    cinfo.input_components = n_chan;
    cinfo.in_color_space   = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo,quality, true);

    JSAMPROW row_ptr;

    // Copy float image to uint8 buffer
    uint8_t *buffer = new uint8_t[width*height*n_chan];
    memset(buffer,0,width*height*n_chan*sizeof(uint8_t));
    uint8_t *curr_pixel = buffer;
    for (size_t y = 0; y < height; ++y)
    for (size_t x = 0; x < width; ++x)
    {
        // red
        *curr_pixel = PNG_RANGE * clamp(image(0)(y, x)); 
        curr_pixel++;
        // green
        *curr_pixel = PNG_RANGE * clamp(image(1)(y, x)); 
        curr_pixel++;
        // blue
        *curr_pixel = PNG_RANGE * clamp(image(2)(y, x)); 
        curr_pixel++;
    }

    jpeg_start_compress(&cinfo, true);
    while(cinfo.next_scanline < height)
    {
        row_ptr = (JSAMPROW) buffer + cinfo.next_scanline*n_chan*width;
        jpeg_write_scanlines(&cinfo, &row_ptr, 1);
    }
    jpeg_finish_compress(&cinfo);
    
    delete buffer;
    buffer = NULL;

    fclose(pFile);
    return true;
}

bool read_jpeg(const string& filename, ImageType_3* image) {
    // FILE* pFile = fopen(filename.c_str(), "rb");
    // if (!pFile)
    //     return false;
    //
    // jpeg_decompress_struct cinfo;
    // jpeg_error_mgr jerr;
    //
    // cinfo.err = jpeg_std_error(&jerr);
    // jmp_buf jumpBuffer;
    //
    //
    // jpeg_create_decompress(&cinfo);
    // jpeg_stdio_src(&cinfo, pFile);
    // jpeg_read_header(&cinfo, TRUE);
    // jpeg_start_decompress(&cinfo);

    // int width = cinfo.image_width;
    // int height = cinfo.image_height;
    // int n_chan = cinfo.num_components;
    // if(n_chan != image->rows()){
    //     printf("jpeg load error, non matching channels");
    // }
    // for(int i=0; i<image->rows(); i++){
    //     (*image)(i) = ImageType_1(height, width);
    // }
      
    // uint8_t *pData = new uint8_t[height*width*n_chan];
    
    // while(cinfo.output_scanline < height)
    // {
    // 	uint8_t* p = pData + cinfo.output_scanline*width*n_chan;
    // 	jpeg_read_scanlines(&cinfo, &p, 1);
    // }
    //
    // jpeg_finish_decompress(&cinfo);
    // jpeg_destroy_decompress(&cinfo);
    // fclose(pFile);

    return true;
}

bool imwrite(const ImageType_3& image, const string& filename){
    std::regex jpg_regex(".*jpg");
    std::regex png_regex(".*png");
    int height = image(0).rows();
    int width  = image(0).cols();
    if(regex_match(filename, jpg_regex)){
        write_jpeg(image, filename);
    }else if(regex_match(filename, png_regex)){
        int r, g, b;
        png::image<png::rgb_pixel> buf_image(width, height);
        for (size_t y = 0; y < height; ++y){
            for (size_t x = 0; x < width; ++x){
                r = PNG_RANGE * clamp(image(0)(y, x)); 
                g = PNG_RANGE * clamp(image(1)(y, x)); 
                b = PNG_RANGE * clamp(image(2)(y, x)); 
                buf_image[y][x] = png::rgb_pixel(r, g, b);
                // non-checking equivalent of image.set_pixel(x, y, ...);
            }
        }
        buf_image.write(filename);
    }
    return true;
}
bool imwrite(const ImageType_1& image, const string& filename){
    int height = image.rows();
    int width = image.cols();
    int val;
    png::image<png::gray_pixel> buf_image(width, height);
    for (size_t y = 0; y < height; ++y){
        for (size_t x = 0; x < width; ++x){
            val = PNG_RANGE * clamp(image(y, x)); 
            buf_image[y][x] = png::gray_pixel(val);
            // non-checking equivalent of image.set_pixel(x, y, ...);
        }
    }
    buf_image.write(filename);
    return true;
}
}// namespace xform
