// tvm target: c -keys=arm_cpu,cpu -device=arm_cpu -march=armv7e-m -mcpu=cortex-m7 -model=stm32f746xx
#define TVM_EXPORTS
#include "tvm/runtime/c_backend_api.h"
#include "tvm/runtime/c_runtime_api.h"
#include <math.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_batch_flatten(void *args, int32_t *arg_type_ids, int32_t num_args,
                                                  void *out_ret_value, int32_t *out_ret_tcode, void *resource_handle);
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_conv2d_add_nn_relu(void *args, int32_t *arg_type_ids, int32_t num_args,
                                                       void *out_ret_value, int32_t *out_ret_tcode,
                                                       void *resource_handle);
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_conv2d_add_nn_relu_1(void *args, int32_t *arg_type_ids, int32_t num_args,
                                                         void *out_ret_value, int32_t *out_ret_tcode,
                                                         void *resource_handle);
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_dense_add(void *args, int32_t *arg_type_ids, int32_t num_args, void *out_ret_value,
                                              int32_t *out_ret_tcode, void *resource_handle);
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_dense_add_1(void *args, int32_t *arg_type_ids, int32_t num_args,
                                                void *out_ret_value, int32_t *out_ret_tcode, void *resource_handle);
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_dense_add_nn_relu_nn_relu(void *args, int32_t *arg_type_ids, int32_t num_args,
                                                              void *out_ret_value, int32_t *out_ret_tcode,
                                                              void *resource_handle);
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_max_pool2d(void *args, int32_t *arg_type_ids, int32_t num_args, void *out_ret_value,
                                               int32_t *out_ret_tcode, void *resource_handle);
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_max_pool2d_1(void *args, int32_t *arg_type_ids, int32_t num_args,
                                                 void *out_ret_value, int32_t *out_ret_tcode, void *resource_handle);
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_pad(void *args, int32_t *arg_type_ids, int32_t num_args, void *out_ret_value,
                                        int32_t *out_ret_tcode, void *resource_handle);
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_pad_1(void *args, int32_t *arg_type_ids, int32_t num_args, void *out_ret_value,
                                          int32_t *out_ret_tcode, void *resource_handle);
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_batch_flatten(void *args, int32_t *arg_type_ids, int32_t num_args,
                                                  void *out_ret_value, int32_t *out_ret_tcode, void *resource_handle)
{
    int32_t p0_code = arg_type_ids[0];
    int32_t tensor_code = arg_type_ids[1];
    void *p0 = (((TVMValue *)args)[0].v_handle);
    void *tensor = (((TVMValue *)args)[1].v_handle);
    void *p0_1 = (((DLTensor *)p0)[0].data);
    void *tvmgen_fused_nn_batch_flatten_p0_shape = (((DLTensor *)p0)[0].shape);
    void *tvmgen_fused_nn_batch_flatten_p0_strides = (((DLTensor *)p0)[0].strides);
    int32_t dev_id = (((DLTensor *)p0)[0].device.device_id);
    void *tensor_1 = (((DLTensor *)tensor)[0].data);
    void *tvmgen_fused_nn_batch_flatten_tensor_shape = (((DLTensor *)tensor)[0].shape);
    void *tvmgen_fused_nn_batch_flatten_tensor_strides = (((DLTensor *)tensor)[0].strides);
    if (!(tvmgen_fused_nn_batch_flatten_p0_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_batch_flatten_tensor_strides == NULL))
    {
    }
    for (int32_t ax1_outer = 0; ax1_outer < 56; ++ax1_outer)
    {
        for (int32_t ax1_inner = 0; ax1_inner < 4; ++ax1_inner)
        {
            int32_t cse_var_1 = ((ax1_outer * 4) + ax1_inner);
            ((float *)tensor_1)[cse_var_1] = ((float *)p0_1)[cse_var_1];
        }
    }
    return 0;
}

#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_conv2d_add_nn_relu(void *args, int32_t *arg_type_ids, int32_t num_args,
                                                       void *out_ret_value, int32_t *out_ret_tcode,
                                                       void *resource_handle)
{
    int32_t p0_code = arg_type_ids[0];
    int32_t p1_code = arg_type_ids[1];
    int32_t p2_code = arg_type_ids[2];
    int32_t T_relu_code = arg_type_ids[3];
    void *p0 = (((TVMValue *)args)[0].v_handle);
    void *p1 = (((TVMValue *)args)[1].v_handle);
    void *p2 = (((TVMValue *)args)[2].v_handle);
    void *T_relu = (((TVMValue *)args)[3].v_handle);
    void *p0_1 = (((DLTensor *)p0)[0].data);
    void *tvmgen_fused_nn_conv2d_add_nn_relu_p0_shape = (((DLTensor *)p0)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_nn_relu_p0_strides = (((DLTensor *)p0)[0].strides);
    int32_t dev_id = (((DLTensor *)p0)[0].device.device_id);
    void *p1_1 = (((DLTensor *)p1)[0].data);
    void *tvmgen_fused_nn_conv2d_add_nn_relu_p1_shape = (((DLTensor *)p1)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_nn_relu_p1_strides = (((DLTensor *)p1)[0].strides);
    void *p2_1 = (((DLTensor *)p2)[0].data);
    void *tvmgen_fused_nn_conv2d_add_nn_relu_p2_shape = (((DLTensor *)p2)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_nn_relu_p2_strides = (((DLTensor *)p2)[0].strides);
    void *T_relu_1 = (((DLTensor *)T_relu)[0].data);
    void *tvmgen_fused_nn_conv2d_add_nn_relu_T_relu_shape = (((DLTensor *)T_relu)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_nn_relu_T_relu_strides = (((DLTensor *)T_relu)[0].strides);
    if (!(tvmgen_fused_nn_conv2d_add_nn_relu_p0_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_conv2d_add_nn_relu_p1_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_conv2d_add_nn_relu_p2_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_conv2d_add_nn_relu_T_relu_strides == NULL))
    {
    }
    void *data_vec = TVMBackendAllocWorkspace(1, dev_id, (uint64_t)10240, 2, 32);
    if (data_vec == NULL)
    {
        return -1;
    }
    float kernel_vec[96];
    for (int32_t h = 0; h < 128; ++h)
    {
        for (int32_t vh = 0; vh < 4; ++vh)
        {
            for (int32_t vw = 0; vw < 5; ++vw)
            {
                int32_t cse_var_1 = (vh * 5);
                ((float *)data_vec)[(((h * 20) + cse_var_1) + vw)] = ((float *)p0_1)[(((h * 5) + cse_var_1) + vw)];
            }
        }
    }
    for (int32_t co = 0; co < 2; ++co)
    {
        for (int32_t kh = 0; kh < 4; ++kh)
        {
            for (int32_t kw = 0; kw < 3; ++kw)
            {
                for (int32_t vc = 0; vc < 4; ++vc)
                {
                    int32_t cse_var_2 = (co * 48);
                    kernel_vec[(((cse_var_2 + (kh * 12)) + (kw * 4)) + vc)] =
                        ((float *)p1_1)[(((cse_var_2 + (vc * 12)) + (kh * 3)) + kw)];
                }
            }
        }
    }
    for (int32_t ax1_outer = 0; ax1_outer < 2; ++ax1_outer)
    {
        float conv[12];
        for (int32_t ax2_outer = 0; ax2_outer < 128; ++ax2_outer)
        {
            for (int32_t vw_init = 0; vw_init < 3; ++vw_init)
            {
                for (int32_t vc_init = 0; vc_init < 4; ++vc_init)
                {
                    conv[((vw_init * 4) + vc_init)] = 0.000000e+00f;
                }
            }
            for (int32_t vw_1 = 0; vw_1 < 3; ++vw_1)
            {
                for (int32_t vc_1 = 0; vc_1 < 4; ++vc_1)
                {
                    int32_t cse_var_3 = ((vw_1 * 4) + vc_1);
                    conv[cse_var_3] = (conv[cse_var_3] + (((float *)data_vec)[((ax2_outer * 20) + vw_1)] *
                                                          kernel_vec[((ax1_outer * 48) + vc_1)]));
                }
            }
            for (int32_t vw_2 = 0; vw_2 < 3; ++vw_2)
            {
                for (int32_t vc_2 = 0; vc_2 < 4; ++vc_2)
                {
                    int32_t cse_var_4 = ((vw_2 * 4) + vc_2);
                    conv[cse_var_4] = (conv[cse_var_4] + (((float *)data_vec)[(((ax2_outer * 20) + vw_2) + 1)] *
                                                          kernel_vec[(((ax1_outer * 48) + vc_2) + 4)]));
                }
            }
            for (int32_t vw_3 = 0; vw_3 < 3; ++vw_3)
            {
                for (int32_t vc_3 = 0; vc_3 < 4; ++vc_3)
                {
                    int32_t cse_var_5 = ((vw_3 * 4) + vc_3);
                    conv[cse_var_5] = (conv[cse_var_5] + (((float *)data_vec)[(((ax2_outer * 20) + vw_3) + 2)] *
                                                          kernel_vec[(((ax1_outer * 48) + vc_3) + 8)]));
                }
            }
            for (int32_t vw_4 = 0; vw_4 < 3; ++vw_4)
            {
                for (int32_t vc_4 = 0; vc_4 < 4; ++vc_4)
                {
                    int32_t cse_var_6 = ((vw_4 * 4) + vc_4);
                    conv[cse_var_6] = (conv[cse_var_6] + (((float *)data_vec)[(((ax2_outer * 20) + vw_4) + 5)] *
                                                          kernel_vec[(((ax1_outer * 48) + vc_4) + 12)]));
                }
            }
            for (int32_t vw_5 = 0; vw_5 < 3; ++vw_5)
            {
                for (int32_t vc_5 = 0; vc_5 < 4; ++vc_5)
                {
                    int32_t cse_var_7 = ((vw_5 * 4) + vc_5);
                    conv[cse_var_7] = (conv[cse_var_7] + (((float *)data_vec)[(((ax2_outer * 20) + vw_5) + 6)] *
                                                          kernel_vec[(((ax1_outer * 48) + vc_5) + 16)]));
                }
            }
            for (int32_t vw_6 = 0; vw_6 < 3; ++vw_6)
            {
                for (int32_t vc_6 = 0; vc_6 < 4; ++vc_6)
                {
                    int32_t cse_var_8 = ((vw_6 * 4) + vc_6);
                    conv[cse_var_8] = (conv[cse_var_8] + (((float *)data_vec)[(((ax2_outer * 20) + vw_6) + 7)] *
                                                          kernel_vec[(((ax1_outer * 48) + vc_6) + 20)]));
                }
            }
            for (int32_t vw_7 = 0; vw_7 < 3; ++vw_7)
            {
                for (int32_t vc_7 = 0; vc_7 < 4; ++vc_7)
                {
                    int32_t cse_var_9 = ((vw_7 * 4) + vc_7);
                    conv[cse_var_9] = (conv[cse_var_9] + (((float *)data_vec)[(((ax2_outer * 20) + vw_7) + 10)] *
                                                          kernel_vec[(((ax1_outer * 48) + vc_7) + 24)]));
                }
            }
            for (int32_t vw_8 = 0; vw_8 < 3; ++vw_8)
            {
                for (int32_t vc_8 = 0; vc_8 < 4; ++vc_8)
                {
                    int32_t cse_var_10 = ((vw_8 * 4) + vc_8);
                    conv[cse_var_10] = (conv[cse_var_10] + (((float *)data_vec)[(((ax2_outer * 20) + vw_8) + 11)] *
                                                            kernel_vec[(((ax1_outer * 48) + vc_8) + 28)]));
                }
            }
            for (int32_t vw_9 = 0; vw_9 < 3; ++vw_9)
            {
                for (int32_t vc_9 = 0; vc_9 < 4; ++vc_9)
                {
                    int32_t cse_var_11 = ((vw_9 * 4) + vc_9);
                    conv[cse_var_11] = (conv[cse_var_11] + (((float *)data_vec)[(((ax2_outer * 20) + vw_9) + 12)] *
                                                            kernel_vec[(((ax1_outer * 48) + vc_9) + 32)]));
                }
            }
            for (int32_t vw_10 = 0; vw_10 < 3; ++vw_10)
            {
                for (int32_t vc_10 = 0; vc_10 < 4; ++vc_10)
                {
                    int32_t cse_var_12 = ((vw_10 * 4) + vc_10);
                    conv[cse_var_12] = (conv[cse_var_12] + (((float *)data_vec)[(((ax2_outer * 20) + vw_10) + 15)] *
                                                            kernel_vec[(((ax1_outer * 48) + vc_10) + 36)]));
                }
            }
            for (int32_t vw_11 = 0; vw_11 < 3; ++vw_11)
            {
                for (int32_t vc_11 = 0; vc_11 < 4; ++vc_11)
                {
                    int32_t cse_var_13 = ((vw_11 * 4) + vc_11);
                    conv[cse_var_13] = (conv[cse_var_13] + (((float *)data_vec)[(((ax2_outer * 20) + vw_11) + 16)] *
                                                            kernel_vec[(((ax1_outer * 48) + vc_11) + 40)]));
                }
            }
            for (int32_t vw_12 = 0; vw_12 < 3; ++vw_12)
            {
                for (int32_t vc_12 = 0; vc_12 < 4; ++vc_12)
                {
                    int32_t cse_var_14 = ((vw_12 * 4) + vc_12);
                    conv[cse_var_14] = (conv[cse_var_14] + (((float *)data_vec)[(((ax2_outer * 20) + vw_12) + 17)] *
                                                            kernel_vec[(((ax1_outer * 48) + vc_12) + 44)]));
                }
            }
            for (int32_t ax3_inner = 0; ax3_inner < 3; ++ax3_inner)
            {
                for (int32_t ax1_inner = 0; ax1_inner < 4; ++ax1_inner)
                {
                    float v_ = conv[((ax3_inner * 4) + ax1_inner)] + ((float *)p2_1)[((ax1_outer * 4) + ax1_inner)];
                    ((float *)T_relu_1)[((((ax1_outer * 1536) + (ax1_inner * 384)) + (ax2_outer * 3)) + ax3_inner)] =
                        ((v_) > (0.000000e+00f) ? (v_) : (0.000000e+00f));
                }
            }
        }
    }
    if (TVMBackendFreeWorkspace(1, dev_id, data_vec) != 0)
    {
        return -1;
    }
    return 0;
}

#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_conv2d_add_nn_relu_1(void *args, int32_t *arg_type_ids, int32_t num_args,
                                                         void *out_ret_value, int32_t *out_ret_tcode,
                                                         void *resource_handle)
{
    int32_t p0_code = arg_type_ids[0];
    int32_t p1_code = arg_type_ids[1];
    int32_t p2_code = arg_type_ids[2];
    int32_t T_relu_code = arg_type_ids[3];
    void *p0 = (((TVMValue *)args)[0].v_handle);
    void *p1 = (((TVMValue *)args)[1].v_handle);
    void *p2 = (((TVMValue *)args)[2].v_handle);
    void *T_relu = (((TVMValue *)args)[3].v_handle);
    void *p0_1 = (((DLTensor *)p0)[0].data);
    void *tvmgen_fused_nn_conv2d_add_nn_relu_1_p0_shape = (((DLTensor *)p0)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_nn_relu_1_p0_strides = (((DLTensor *)p0)[0].strides);
    int32_t dev_id = (((DLTensor *)p0)[0].device.device_id);
    void *p1_1 = (((DLTensor *)p1)[0].data);
    void *tvmgen_fused_nn_conv2d_add_nn_relu_1_p1_shape = (((DLTensor *)p1)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_nn_relu_1_p1_strides = (((DLTensor *)p1)[0].strides);
    void *p2_1 = (((DLTensor *)p2)[0].data);
    void *tvmgen_fused_nn_conv2d_add_nn_relu_1_p2_shape = (((DLTensor *)p2)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_nn_relu_1_p2_strides = (((DLTensor *)p2)[0].strides);
    void *T_relu_1 = (((DLTensor *)T_relu)[0].data);
    void *tvmgen_fused_nn_conv2d_add_nn_relu_1_T_relu_shape = (((DLTensor *)T_relu)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_nn_relu_1_T_relu_strides = (((DLTensor *)T_relu)[0].strides);
    if (!(tvmgen_fused_nn_conv2d_add_nn_relu_1_p0_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_conv2d_add_nn_relu_1_p1_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_conv2d_add_nn_relu_1_p2_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_conv2d_add_nn_relu_1_T_relu_strides == NULL))
    {
    }
    void *data_vec = TVMBackendAllocWorkspace(1, dev_id, (uint64_t)3360, 2, 32);
    if (data_vec == NULL)
    {
        return -1;
    }
    void *kernel_vec = TVMBackendAllocWorkspace(1, dev_id, (uint64_t)2048, 2, 32);
    if (kernel_vec == NULL)
    {
        return -1;
    }
    for (int32_t h = 0; h < 21; ++h)
    {
        for (int32_t ci = 0; ci < 8; ++ci)
        {
            for (int32_t vh = 0; vh < 5; ++vh)
            {
                ((float *)data_vec)[(((h * 40) + (ci * 5)) + vh)] = ((float *)p0_1)[(((ci * 45) + (h * 2)) + vh)];
            }
        }
    }
    for (int32_t co = 0; co < 4; ++co)
    {
        for (int32_t ci_1 = 0; ci_1 < 8; ++ci_1)
        {
            for (int32_t kh = 0; kh < 4; ++kh)
            {
                for (int32_t vc = 0; vc < 4; ++vc)
                {
                    int32_t cse_var_1 = (co * 128);
                    ((float *)kernel_vec)[(((cse_var_1 + (ci_1 * 16)) + (kh * 4)) + vc)] =
                        ((float *)p1_1)[(((cse_var_1 + (vc * 32)) + (ci_1 * 4)) + kh)];
                }
            }
        }
    }
    for (int32_t ax1_outer = 0; ax1_outer < 4; ++ax1_outer)
    {
        float conv[8];
        for (int32_t ax2_outer = 0; ax2_outer < 21; ++ax2_outer)
        {
            for (int32_t vc_init = 0; vc_init < 4; ++vc_init)
            {
                conv[vc_init] = 0.000000e+00f;
                conv[(vc_init + 4)] = 0.000000e+00f;
            }
            for (int32_t ci_2 = 0; ci_2 < 8; ++ci_2)
            {
                for (int32_t vc_1 = 0; vc_1 < 4; ++vc_1)
                {
                    int32_t cse_var_4 = (vc_1 + 4);
                    int32_t cse_var_3 = ((ax2_outer * 40) + (ci_2 * 5));
                    int32_t cse_var_2 = (((ax1_outer * 128) + (ci_2 * 16)) + vc_1);
                    conv[vc_1] = (conv[vc_1] + (((float *)data_vec)[cse_var_3] * ((float *)kernel_vec)[cse_var_2]));
                    conv[cse_var_4] =
                        (conv[cse_var_4] + (((float *)data_vec)[(cse_var_3 + 1)] * ((float *)kernel_vec)[cse_var_2]));
                }
                for (int32_t vc_2 = 0; vc_2 < 4; ++vc_2)
                {
                    int32_t cse_var_7 = (vc_2 + 4);
                    int32_t cse_var_6 = ((ax2_outer * 40) + (ci_2 * 5));
                    int32_t cse_var_5 = ((((ax1_outer * 128) + (ci_2 * 16)) + vc_2) + 4);
                    conv[vc_2] =
                        (conv[vc_2] + (((float *)data_vec)[(cse_var_6 + 1)] * ((float *)kernel_vec)[cse_var_5]));
                    conv[cse_var_7] =
                        (conv[cse_var_7] + (((float *)data_vec)[(cse_var_6 + 2)] * ((float *)kernel_vec)[cse_var_5]));
                }
                for (int32_t vc_3 = 0; vc_3 < 4; ++vc_3)
                {
                    int32_t cse_var_10 = (vc_3 + 4);
                    int32_t cse_var_9 = ((ax2_outer * 40) + (ci_2 * 5));
                    int32_t cse_var_8 = ((((ax1_outer * 128) + (ci_2 * 16)) + vc_3) + 8);
                    conv[vc_3] =
                        (conv[vc_3] + (((float *)data_vec)[(cse_var_9 + 2)] * ((float *)kernel_vec)[cse_var_8]));
                    conv[cse_var_10] =
                        (conv[cse_var_10] + (((float *)data_vec)[(cse_var_9 + 3)] * ((float *)kernel_vec)[cse_var_8]));
                }
                for (int32_t vc_4 = 0; vc_4 < 4; ++vc_4)
                {
                    int32_t cse_var_13 = (vc_4 + 4);
                    int32_t cse_var_12 = ((ax2_outer * 40) + (ci_2 * 5));
                    int32_t cse_var_11 = ((((ax1_outer * 128) + (ci_2 * 16)) + vc_4) + 12);
                    conv[vc_4] =
                        (conv[vc_4] + (((float *)data_vec)[(cse_var_12 + 3)] * ((float *)kernel_vec)[cse_var_11]));
                    conv[cse_var_13] = (conv[cse_var_13] +
                                        (((float *)data_vec)[(cse_var_12 + 4)] * ((float *)kernel_vec)[cse_var_11]));
                }
            }
            for (int32_t ax1_inner = 0; ax1_inner < 4; ++ax1_inner)
            {
                float v_ = conv[ax1_inner] + ((float *)p2_1)[((ax1_outer * 4) + ax1_inner)];
                ((float *)T_relu_1)[(((ax1_outer * 168) + (ax1_inner * 42)) + (ax2_outer * 2))] =
                    ((v_) > (0.000000e+00f) ? (v_) : (0.000000e+00f));
            }
            for (int32_t ax1_inner_1 = 0; ax1_inner_1 < 4; ++ax1_inner_1)
            {
                float v__1 = conv[(ax1_inner_1 + 4)] + ((float *)p2_1)[((ax1_outer * 4) + ax1_inner_1)];
                ((float *)T_relu_1)[((((ax1_outer * 168) + (ax1_inner_1 * 42)) + (ax2_outer * 2)) + 1)] =
                    ((v__1) > (0.000000e+00f) ? (v__1) : (0.000000e+00f));
            }
        }
    }
    if (TVMBackendFreeWorkspace(1, dev_id, kernel_vec) != 0)
    {
        return -1;
    }
    if (TVMBackendFreeWorkspace(1, dev_id, data_vec) != 0)
    {
        return -1;
    }
    return 0;
}

#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_dense_add(void *args, int32_t *arg_type_ids, int32_t num_args, void *out_ret_value,
                                              int32_t *out_ret_tcode, void *resource_handle)
{
    int32_t p0_code = arg_type_ids[0];
    int32_t p1_code = arg_type_ids[1];
    int32_t p2_code = arg_type_ids[2];
    int32_t T_add_code = arg_type_ids[3];
    void *p0 = (((TVMValue *)args)[0].v_handle);
    void *p1 = (((TVMValue *)args)[1].v_handle);
    void *p2 = (((TVMValue *)args)[2].v_handle);
    void *T_add = (((TVMValue *)args)[3].v_handle);
    void *p0_1 = (((DLTensor *)p0)[0].data);
    void *tvmgen_fused_nn_dense_add_p0_shape = (((DLTensor *)p0)[0].shape);
    void *tvmgen_fused_nn_dense_add_p0_strides = (((DLTensor *)p0)[0].strides);
    int32_t dev_id = (((DLTensor *)p0)[0].device.device_id);
    void *p1_1 = (((DLTensor *)p1)[0].data);
    void *tvmgen_fused_nn_dense_add_p1_shape = (((DLTensor *)p1)[0].shape);
    void *tvmgen_fused_nn_dense_add_p1_strides = (((DLTensor *)p1)[0].strides);
    void *p2_1 = (((DLTensor *)p2)[0].data);
    void *tvmgen_fused_nn_dense_add_p2_shape = (((DLTensor *)p2)[0].shape);
    void *tvmgen_fused_nn_dense_add_p2_strides = (((DLTensor *)p2)[0].strides);
    void *T_add_1 = (((DLTensor *)T_add)[0].data);
    void *tvmgen_fused_nn_dense_add_T_add_shape = (((DLTensor *)T_add)[0].shape);
    void *tvmgen_fused_nn_dense_add_T_add_strides = (((DLTensor *)T_add)[0].strides);
    if (!(tvmgen_fused_nn_dense_add_p0_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_dense_add_p1_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_dense_add_p2_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_dense_add_T_add_strides == NULL))
    {
    }
    float packed_weight[64];
    float compute_global[4];
    for (int32_t y = 0; y < 16; ++y)
    {
        for (int32_t x = 0; x < 4; ++x)
        {
            packed_weight[((y * 4) + x)] = ((float *)p1_1)[((x * 16) + y)];
        }
    }
    for (int32_t x_c_init = 0; x_c_init < 4; ++x_c_init)
    {
        compute_global[x_c_init] = 0.000000e+00f;
    }
    for (int32_t k_outer = 0; k_outer < 16; ++k_outer)
    {
        for (int32_t x_c = 0; x_c < 4; ++x_c)
        {
            compute_global[x_c] =
                (compute_global[x_c] + (((float *)p0_1)[k_outer] * packed_weight[((k_outer * 4) + x_c)]));
        }
    }
    for (int32_t ax1_inner_inner = 0; ax1_inner_inner < 4; ++ax1_inner_inner)
    {
        ((float *)T_add_1)[ax1_inner_inner] = (compute_global[ax1_inner_inner] + ((float *)p2_1)[ax1_inner_inner]);
    }
    return 0;
}

#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_dense_add_1(void *args, int32_t *arg_type_ids, int32_t num_args,
                                                void *out_ret_value, int32_t *out_ret_tcode, void *resource_handle)
{
    int32_t p0_code = arg_type_ids[0];
    int32_t p1_code = arg_type_ids[1];
    int32_t p2_code = arg_type_ids[2];
    int32_t T_add_code = arg_type_ids[3];
    void *p0 = (((TVMValue *)args)[0].v_handle);
    void *p1 = (((TVMValue *)args)[1].v_handle);
    void *p2 = (((TVMValue *)args)[2].v_handle);
    void *T_add = (((TVMValue *)args)[3].v_handle);
    void *p0_1 = (((DLTensor *)p0)[0].data);
    void *tvmgen_fused_nn_dense_add_1_p0_shape = (((DLTensor *)p0)[0].shape);
    void *tvmgen_fused_nn_dense_add_1_p0_strides = (((DLTensor *)p0)[0].strides);
    int32_t dev_id = (((DLTensor *)p0)[0].device.device_id);
    void *p1_1 = (((DLTensor *)p1)[0].data);
    void *tvmgen_fused_nn_dense_add_1_p1_shape = (((DLTensor *)p1)[0].shape);
    void *tvmgen_fused_nn_dense_add_1_p1_strides = (((DLTensor *)p1)[0].strides);
    void *p2_1 = (((DLTensor *)p2)[0].data);
    void *tvmgen_fused_nn_dense_add_1_p2_shape = (((DLTensor *)p2)[0].shape);
    void *tvmgen_fused_nn_dense_add_1_p2_strides = (((DLTensor *)p2)[0].strides);
    void *T_add_1 = (((DLTensor *)T_add)[0].data);
    void *tvmgen_fused_nn_dense_add_1_T_add_shape = (((DLTensor *)T_add)[0].shape);
    void *tvmgen_fused_nn_dense_add_1_T_add_strides = (((DLTensor *)T_add)[0].strides);
    if (!(tvmgen_fused_nn_dense_add_1_p0_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_dense_add_1_p1_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_dense_add_1_p2_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_dense_add_1_T_add_strides == NULL))
    {
    }
    float packed_weight[16];
    float compute_global[4];
    for (int32_t y = 0; y < 4; ++y)
    {
        for (int32_t x = 0; x < 4; ++x)
        {
            packed_weight[((y * 4) + x)] = ((float *)p1_1)[((x * 4) + y)];
        }
    }
    for (int32_t x_c_init = 0; x_c_init < 4; ++x_c_init)
    {
        compute_global[x_c_init] = 0.000000e+00f;
    }
    for (int32_t k_outer = 0; k_outer < 4; ++k_outer)
    {
        for (int32_t x_c = 0; x_c < 4; ++x_c)
        {
            compute_global[x_c] =
                (compute_global[x_c] + (((float *)p0_1)[k_outer] * packed_weight[((k_outer * 4) + x_c)]));
        }
    }
    for (int32_t ax1_inner_inner = 0; ax1_inner_inner < 4; ++ax1_inner_inner)
    {
        ((float *)T_add_1)[ax1_inner_inner] = (compute_global[ax1_inner_inner] + ((float *)p2_1)[ax1_inner_inner]);
    }
    return 0;
}

#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_dense_add_nn_relu_nn_relu(void *args, int32_t *arg_type_ids, int32_t num_args,
                                                              void *out_ret_value, int32_t *out_ret_tcode,
                                                              void *resource_handle)
{
    int32_t p0_code = arg_type_ids[0];
    int32_t p1_code = arg_type_ids[1];
    int32_t p2_code = arg_type_ids[2];
    int32_t T_relu_code = arg_type_ids[3];
    void *p0 = (((TVMValue *)args)[0].v_handle);
    void *p1 = (((TVMValue *)args)[1].v_handle);
    void *p2 = (((TVMValue *)args)[2].v_handle);
    void *T_relu = (((TVMValue *)args)[3].v_handle);
    void *p0_1 = (((DLTensor *)p0)[0].data);
    void *tvmgen_fused_nn_dense_add_nn_relu_nn_relu_p0_shape = (((DLTensor *)p0)[0].shape);
    void *tvmgen_fused_nn_dense_add_nn_relu_nn_relu_p0_strides = (((DLTensor *)p0)[0].strides);
    int32_t dev_id = (((DLTensor *)p0)[0].device.device_id);
    void *p1_1 = (((DLTensor *)p1)[0].data);
    void *tvmgen_fused_nn_dense_add_nn_relu_nn_relu_p1_shape = (((DLTensor *)p1)[0].shape);
    void *tvmgen_fused_nn_dense_add_nn_relu_nn_relu_p1_strides = (((DLTensor *)p1)[0].strides);
    void *p2_1 = (((DLTensor *)p2)[0].data);
    void *tvmgen_fused_nn_dense_add_nn_relu_nn_relu_p2_shape = (((DLTensor *)p2)[0].shape);
    void *tvmgen_fused_nn_dense_add_nn_relu_nn_relu_p2_strides = (((DLTensor *)p2)[0].strides);
    void *T_relu_1 = (((DLTensor *)T_relu)[0].data);
    void *tvmgen_fused_nn_dense_add_nn_relu_nn_relu_T_relu_shape = (((DLTensor *)T_relu)[0].shape);
    void *tvmgen_fused_nn_dense_add_nn_relu_nn_relu_T_relu_strides = (((DLTensor *)T_relu)[0].strides);
    if (!(tvmgen_fused_nn_dense_add_nn_relu_nn_relu_p0_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_dense_add_nn_relu_nn_relu_p1_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_dense_add_nn_relu_nn_relu_p2_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_dense_add_nn_relu_nn_relu_T_relu_strides == NULL))
    {
    }
    void *packed_weight = TVMBackendAllocWorkspace(1, dev_id, (uint64_t)14336, 2, 32);
    if (packed_weight == NULL)
    {
        return -1;
    }
    for (int32_t z = 0; z < 2; ++z)
    {
        for (int32_t y = 0; y < 224; ++y)
        {
            for (int32_t x = 0; x < 8; ++x)
            {
                int32_t cse_var_1 = (z * 1792);
                ((float *)packed_weight)[((cse_var_1 + (y * 8)) + x)] = ((float *)p1_1)[((cse_var_1 + (x * 224)) + y)];
            }
        }
    }
    for (int32_t ax1_outer_ax0_outer_fused = 0; ax1_outer_ax0_outer_fused < 2; ++ax1_outer_ax0_outer_fused)
    {
        float compute_global[8];
        for (int32_t x_c_init = 0; x_c_init < 8; ++x_c_init)
        {
            compute_global[x_c_init] = 0.000000e+00f;
        }
        for (int32_t k_outer = 0; k_outer < 224; ++k_outer)
        {
            for (int32_t x_c = 0; x_c < 8; ++x_c)
            {
                compute_global[x_c] =
                    (compute_global[x_c] +
                     (((float *)p0_1)[k_outer] *
                      ((float *)packed_weight)[(((ax1_outer_ax0_outer_fused * 1792) + (k_outer * 8)) + x_c)]));
            }
        }
        for (int32_t ax1_inner_inner = 0; ax1_inner_inner < 8; ++ax1_inner_inner)
        {
            int32_t cse_var_2 = ((ax1_outer_ax0_outer_fused * 8) + ax1_inner_inner);
            float v_ = compute_global[ax1_inner_inner] + ((float *)p2_1)[cse_var_2];
            float v__1 = (v_) > (0.000000e+00f) ? (v_) : (0.000000e+00f);
            ((float *)T_relu_1)[cse_var_2] = ((v__1) > (0.000000e+00f) ? (v__1) : (0.000000e+00f));
        }
    }
    if (TVMBackendFreeWorkspace(1, dev_id, packed_weight) != 0)
    {
        return -1;
    }
    return 0;
}

#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_max_pool2d(void *args, int32_t *arg_type_ids, int32_t num_args, void *out_ret_value,
                                               int32_t *out_ret_tcode, void *resource_handle)
{
    int32_t p0_code = arg_type_ids[0];
    int32_t pool_max_code = arg_type_ids[1];
    void *p0 = (((TVMValue *)args)[0].v_handle);
    void *pool_max = (((TVMValue *)args)[1].v_handle);
    void *p0_1 = (((DLTensor *)p0)[0].data);
    void *tvmgen_fused_nn_max_pool2d_p0_shape = (((DLTensor *)p0)[0].shape);
    void *tvmgen_fused_nn_max_pool2d_p0_strides = (((DLTensor *)p0)[0].strides);
    int32_t dev_id = (((DLTensor *)p0)[0].device.device_id);
    void *pool_max_1 = (((DLTensor *)pool_max)[0].data);
    void *tvmgen_fused_nn_max_pool2d_pool_max_shape = (((DLTensor *)pool_max)[0].shape);
    void *tvmgen_fused_nn_max_pool2d_pool_max_strides = (((DLTensor *)pool_max)[0].strides);
    if (!(tvmgen_fused_nn_max_pool2d_p0_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_max_pool2d_pool_max_strides == NULL))
    {
    }
    for (int32_t ax0_ax1_fused = 0; ax0_ax1_fused < 8; ++ax0_ax1_fused)
    {
        for (int32_t ax2 = 0; ax2 < 42; ++ax2)
        {
            ((float *)pool_max_1)[((ax0_ax1_fused * 42) + ax2)] = -3.402823e+38f;
            for (int32_t rv0 = 0; rv0 < 3; ++rv0)
            {
                for (int32_t rv1 = 0; rv1 < 3; ++rv1)
                {
                    int32_t cse_var_1 = ((ax0_ax1_fused * 42) + ax2);
                    float v_ = ((float *)pool_max_1)[cse_var_1];
                    float v__1 = ((float *)p0_1)[((((ax0_ax1_fused * 384) + (ax2 * 9)) + (rv0 * 3)) + rv1)];
                    ((float *)pool_max_1)[cse_var_1] = ((v_) > (v__1) ? (v_) : (v__1));
                }
            }
        }
    }
    return 0;
}

#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_max_pool2d_1(void *args, int32_t *arg_type_ids, int32_t num_args,
                                                 void *out_ret_value, int32_t *out_ret_tcode, void *resource_handle)
{
    int32_t p0_code = arg_type_ids[0];
    int32_t pool_max_code = arg_type_ids[1];
    void *p0 = (((TVMValue *)args)[0].v_handle);
    void *pool_max = (((TVMValue *)args)[1].v_handle);
    void *p0_1 = (((DLTensor *)p0)[0].data);
    void *tvmgen_fused_nn_max_pool2d_1_p0_shape = (((DLTensor *)p0)[0].shape);
    void *tvmgen_fused_nn_max_pool2d_1_p0_strides = (((DLTensor *)p0)[0].strides);
    int32_t dev_id = (((DLTensor *)p0)[0].device.device_id);
    void *pool_max_1 = (((DLTensor *)pool_max)[0].data);
    void *tvmgen_fused_nn_max_pool2d_1_pool_max_shape = (((DLTensor *)pool_max)[0].shape);
    void *tvmgen_fused_nn_max_pool2d_1_pool_max_strides = (((DLTensor *)pool_max)[0].strides);
    if (!(tvmgen_fused_nn_max_pool2d_1_p0_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_max_pool2d_1_pool_max_strides == NULL))
    {
    }
    for (int32_t ax0_ax1_fused = 0; ax0_ax1_fused < 16; ++ax0_ax1_fused)
    {
        for (int32_t ax2 = 0; ax2 < 14; ++ax2)
        {
            ((float *)pool_max_1)[((ax0_ax1_fused * 14) + ax2)] = -3.402823e+38f;
            for (int32_t rv0 = 0; rv0 < 3; ++rv0)
            {
                int32_t cse_var_1 = ((ax0_ax1_fused * 14) + ax2);
                float v_ = ((float *)pool_max_1)[cse_var_1];
                float v__1 = ((float *)p0_1)[(((ax0_ax1_fused * 42) + (ax2 * 3)) + rv0)];
                ((float *)pool_max_1)[cse_var_1] = ((v_) > (v__1) ? (v_) : (v__1));
            }
        }
    }
    return 0;
}

#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_pad(void *args, int32_t *arg_type_ids, int32_t num_args, void *out_ret_value,
                                        int32_t *out_ret_tcode, void *resource_handle)
{
    int32_t p0_code = arg_type_ids[0];
    int32_t T_pad_code = arg_type_ids[1];
    void *p0 = (((TVMValue *)args)[0].v_handle);
    void *T_pad = (((TVMValue *)args)[1].v_handle);
    void *p0_1 = (((DLTensor *)p0)[0].data);
    void *tvmgen_fused_nn_pad_p0_shape = (((DLTensor *)p0)[0].shape);
    void *tvmgen_fused_nn_pad_p0_strides = (((DLTensor *)p0)[0].strides);
    int32_t dev_id = (((DLTensor *)p0)[0].device.device_id);
    void *T_pad_1 = (((DLTensor *)T_pad)[0].data);
    void *tvmgen_fused_nn_pad_T_pad_shape = (((DLTensor *)T_pad)[0].shape);
    void *tvmgen_fused_nn_pad_T_pad_strides = (((DLTensor *)T_pad)[0].strides);
    if (!(tvmgen_fused_nn_pad_p0_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_pad_T_pad_strides == NULL))
    {
    }
    for (int32_t ax0_ax1_fused_ax2_fused = 0; ax0_ax1_fused_ax2_fused < 131; ++ax0_ax1_fused_ax2_fused)
    {
        for (int32_t ax3_outer = 0; ax3_outer < 2; ++ax3_outer)
        {
            for (int32_t ax3_inner = 0; ax3_inner < 4; ++ax3_inner)
            {
                if (((ax3_outer * 4) + ax3_inner) < 5)
                {
                    int32_t cse_var_1 = (ax3_outer * 4);
                    ((float *)T_pad_1)[(((ax0_ax1_fused_ax2_fused * 5) + cse_var_1) + ax3_inner)] =
                        (((((1 <= ax0_ax1_fused_ax2_fused) && (ax0_ax1_fused_ax2_fused < 129)) &&
                           (1 <= (cse_var_1 + ax3_inner))) &&
                          (ax3_outer < 1))
                             ? ((float *)p0_1)[(((cse_var_1 + (ax0_ax1_fused_ax2_fused * 3)) + ax3_inner) - 4)]
                             : 0.000000e+00f);
                }
            }
        }
    }
    return 0;
}

#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_pad_1(void *args, int32_t *arg_type_ids, int32_t num_args, void *out_ret_value,
                                          int32_t *out_ret_tcode, void *resource_handle)
{
    int32_t p0_code = arg_type_ids[0];
    int32_t T_pad_code = arg_type_ids[1];
    void *p0 = (((TVMValue *)args)[0].v_handle);
    void *T_pad = (((TVMValue *)args)[1].v_handle);
    void *p0_1 = (((DLTensor *)p0)[0].data);
    void *tvmgen_fused_nn_pad_1_p0_shape = (((DLTensor *)p0)[0].shape);
    void *tvmgen_fused_nn_pad_1_p0_strides = (((DLTensor *)p0)[0].strides);
    int32_t dev_id = (((DLTensor *)p0)[0].device.device_id);
    void *T_pad_1 = (((DLTensor *)T_pad)[0].data);
    void *tvmgen_fused_nn_pad_1_T_pad_shape = (((DLTensor *)T_pad)[0].shape);
    void *tvmgen_fused_nn_pad_1_T_pad_strides = (((DLTensor *)T_pad)[0].strides);
    if (!(tvmgen_fused_nn_pad_1_p0_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_pad_1_T_pad_strides == NULL))
    {
    }
    for (int32_t ax0_ax1_fused_ax2_fused = 0; ax0_ax1_fused_ax2_fused < 360; ++ax0_ax1_fused_ax2_fused)
    {
        int32_t cse_var_1 = (ax0_ax1_fused_ax2_fused % 45);
        ((float *)T_pad_1)[ax0_ax1_fused_ax2_fused] =
            (((1 <= cse_var_1) && (cse_var_1 < 43))
                 ? ((float *)p0_1)[((((ax0_ax1_fused_ax2_fused / 45) * 42) + cse_var_1) - 1)]
                 : 0.000000e+00f);
    }
    return 0;
}
