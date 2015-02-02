#ifndef HALIDE_halide_recon_separate
#define HALIDE_halide_recon_separate
#ifndef BUFFER_T_DEFINED
#define BUFFER_T_DEFINED
#include <stdint.h>
typedef struct buffer_t {
    uint64_t dev;
    uint8_t* host;
    int32_t extent[4];
    int32_t stride[4];
    int32_t min[4];
    int32_t elem_size;
    bool host_dirty;
    bool dev_dirty;
} buffer_t;
#endif
#ifndef HALIDE_FUNCTION_ATTRS
#define HALIDE_FUNCTION_ATTRS
#endif
#ifdef __cplusplus
extern "C"
#endif
int halide_recon_separate(buffer_t *_p0, const int32_t _p8, buffer_t *_p1, buffer_t *_p2, buffer_t *_p3, buffer_t *_p4, buffer_t *_p5, buffer_t *_p6, buffer_t *_p7, buffer_t *_final) HALIDE_FUNCTION_ATTRS;
#endif
