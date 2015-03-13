#ifndef HALIDE_style_transfer
#define HALIDE_style_transfer
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
int style_transfer(const int32_t _levels, const float _alpha, const float _beta, buffer_t *_input, buffer_t *_model, buffer_t *_style_transfer) HALIDE_FUNCTION_ATTRS;
#ifdef __cplusplus
extern "C"
#endif
int style_transfer_argv(void **args) HALIDE_FUNCTION_ATTRS;
#endif
