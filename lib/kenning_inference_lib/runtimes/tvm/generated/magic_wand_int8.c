// tvm target: c -keys=cpu
#define TVM_EXPORTS
#include "tvm/runtime/c_backend_api.h"
#include "tvm/runtime/c_runtime_api.h"
#include <math.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_layout_transform_cast_subtract(void *args, int32_t *arg_type_ids, int32_t num_args,
                                                                void *out_ret_value, int32_t *out_ret_tcode,
                                                                void *resource_handle);
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_layout_transform_reshape_cast_subtract(void *args, int32_t *arg_type_ids,
                                                                        int32_t num_args, void *out_ret_value,
                                                                        int32_t *out_ret_tcode, void *resource_handle);
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast(
        void *args, int32_t *arg_type_ids, int32_t num_args, void *out_ret_value, int32_t *out_ret_tcode,
        void *resource_handle);
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1(
        void *args, int32_t *arg_type_ids, int32_t num_args, void *out_ret_value, int32_t *out_ret_tcode,
        void *resource_handle);
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_cast(void *args, int32_t *arg_type_ids,
                                                                                 int32_t num_args, void *out_ret_value,
                                                                                 int32_t *out_ret_tcode,
                                                                                 void *resource_handle);
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_subtract_cast_multiply(
        void *args, int32_t *arg_type_ids, int32_t num_args, void *out_ret_value, int32_t *out_ret_tcode,
        void *resource_handle);
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_max_pool2d(void *args, int32_t *arg_type_ids, int32_t num_args, void *out_ret_value,
                                               int32_t *out_ret_tcode, void *resource_handle);
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_max_pool2d_cast_subtract(void *args, int32_t *arg_type_ids, int32_t num_args,
                                                             void *out_ret_value, int32_t *out_ret_tcode,
                                                             void *resource_handle);
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_softmax_divide_round_add_clip_cast(void *args, int32_t *arg_type_ids,
                                                                       int32_t num_args, void *out_ret_value,
                                                                       int32_t *out_ret_tcode, void *resource_handle);
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_reshape_cast_subtract(void *args, int32_t *arg_type_ids, int32_t num_args,
                                                       void *out_ret_value, int32_t *out_ret_tcode,
                                                       void *resource_handle);
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL float expf(float);
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL float roundf(float);
#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_layout_transform_cast_subtract(void *args, int32_t *arg_type_ids, int32_t num_args,
                                                                void *out_ret_value, int32_t *out_ret_tcode,
                                                                void *resource_handle)
{
    int32_t p0_code = arg_type_ids[0];
    int32_t p1_code = arg_type_ids[1];
    int32_t T_subtract_code = arg_type_ids[2];
    void *p0 = (((TVMValue *)args)[0].v_handle);
    void *p1 = (((TVMValue *)args)[1].v_handle);
    void *T_subtract = (((TVMValue *)args)[2].v_handle);
    void *p0_1 = (((DLTensor *)p0)[0].data);
    void *tvmgen_fused_layout_transform_cast_subtract_p0_shape = (((DLTensor *)p0)[0].shape);
    void *tvmgen_fused_layout_transform_cast_subtract_p0_strides = (((DLTensor *)p0)[0].strides);
    int32_t dev_id = (((DLTensor *)p0)[0].device.device_id);
    void *p1_1 = (((DLTensor *)p1)[0].data);
    void *tvmgen_fused_layout_transform_cast_subtract_p1_shape = (((DLTensor *)p1)[0].shape);
    void *tvmgen_fused_layout_transform_cast_subtract_p1_strides = (((DLTensor *)p1)[0].strides);
    void *T_subtract_1 = (((DLTensor *)T_subtract)[0].data);
    void *tvmgen_fused_layout_transform_cast_subtract_T_subtract_shape = (((DLTensor *)T_subtract)[0].shape);
    void *tvmgen_fused_layout_transform_cast_subtract_T_subtract_strides = (((DLTensor *)T_subtract)[0].strides);
    if (!(tvmgen_fused_layout_transform_cast_subtract_p0_strides == NULL))
    {
    }
    if (!(tvmgen_fused_layout_transform_cast_subtract_T_subtract_strides == NULL))
    {
    }
    for (int32_t ax2 = 0; ax2 < 128; ++ax2)
    {
        for (int32_t ax3_inner = 0; ax3_inner < 3; ++ax3_inner)
        {
            int32_t cse_var_1 = ((ax2 * 3) + ax3_inner);
            ((int16_t *)T_subtract_1)[cse_var_1] = (((int16_t)((int8_t *)p0_1)[cse_var_1]) - ((int16_t *)p1_1)[0]);
        }
    }
    return 0;
}

#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_layout_transform_reshape_cast_subtract(void *args, int32_t *arg_type_ids,
                                                                        int32_t num_args, void *out_ret_value,
                                                                        int32_t *out_ret_tcode, void *resource_handle)
{
    int32_t p0_code = arg_type_ids[0];
    int32_t p1_code = arg_type_ids[1];
    int32_t T_subtract_code = arg_type_ids[2];
    void *p0 = (((TVMValue *)args)[0].v_handle);
    void *p1 = (((TVMValue *)args)[1].v_handle);
    void *T_subtract = (((TVMValue *)args)[2].v_handle);
    void *p0_1 = (((DLTensor *)p0)[0].data);
    void *tvmgen_fused_layout_transform_reshape_cast_subtract_p0_shape = (((DLTensor *)p0)[0].shape);
    void *tvmgen_fused_layout_transform_reshape_cast_subtract_p0_strides = (((DLTensor *)p0)[0].strides);
    int32_t dev_id = (((DLTensor *)p0)[0].device.device_id);
    void *p1_1 = (((DLTensor *)p1)[0].data);
    void *tvmgen_fused_layout_transform_reshape_cast_subtract_p1_shape = (((DLTensor *)p1)[0].shape);
    void *tvmgen_fused_layout_transform_reshape_cast_subtract_p1_strides = (((DLTensor *)p1)[0].strides);
    void *T_subtract_1 = (((DLTensor *)T_subtract)[0].data);
    void *tvmgen_fused_layout_transform_reshape_cast_subtract_T_subtract_shape = (((DLTensor *)T_subtract)[0].shape);
    void *tvmgen_fused_layout_transform_reshape_cast_subtract_T_subtract_strides =
        (((DLTensor *)T_subtract)[0].strides);
    if (!(tvmgen_fused_layout_transform_reshape_cast_subtract_p0_strides == NULL))
    {
    }
    if (!(tvmgen_fused_layout_transform_reshape_cast_subtract_T_subtract_strides == NULL))
    {
    }
    for (int32_t ax1_outer = 0; ax1_outer < 14; ++ax1_outer)
    {
        for (int32_t ax1_inner = 0; ax1_inner < 16; ++ax1_inner)
        {
            ((int16_t *)T_subtract_1)[((ax1_outer * 16) + ax1_inner)] =
                (((int16_t)((int8_t *)p0_1)[((ax1_inner * 14) + ax1_outer)]) - ((int16_t *)p1_1)[0]);
        }
    }
    return 0;
}

#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast(
        void *args, int32_t *arg_type_ids, int32_t num_args, void *out_ret_value, int32_t *out_ret_tcode,
        void *resource_handle)
{
    int32_t p0_code = arg_type_ids[0];
    int32_t p1_code = arg_type_ids[1];
    int32_t p2_code = arg_type_ids[2];
    int32_t p3_code = arg_type_ids[3];
    int32_t p4_code = arg_type_ids[4];
    int32_t p5_code = arg_type_ids[5];
    int32_t p6_code = arg_type_ids[6];
    int32_t T_cast_code = arg_type_ids[7];
    void *p0 = (((TVMValue *)args)[0].v_handle);
    void *p1 = (((TVMValue *)args)[1].v_handle);
    void *p2 = (((TVMValue *)args)[2].v_handle);
    void *p3 = (((TVMValue *)args)[3].v_handle);
    void *p4 = (((TVMValue *)args)[4].v_handle);
    void *p5 = (((TVMValue *)args)[5].v_handle);
    void *p6 = (((TVMValue *)args)[6].v_handle);
    void *T_cast = (((TVMValue *)args)[7].v_handle);
    void *p0_1 = (((DLTensor *)p0)[0].data);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_p0_shape = (((DLTensor *)p0)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_p0_strides =
        (((DLTensor *)p0)[0].strides);
    int32_t dev_id = (((DLTensor *)p0)[0].device.device_id);
    void *p1_1 = (((DLTensor *)p1)[0].data);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_p1_shape = (((DLTensor *)p1)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_p1_strides =
        (((DLTensor *)p1)[0].strides);
    void *p2_1 = (((DLTensor *)p2)[0].data);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_p2_shape = (((DLTensor *)p2)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_p2_strides =
        (((DLTensor *)p2)[0].strides);
    void *p3_1 = (((DLTensor *)p3)[0].data);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_p3_shape = (((DLTensor *)p3)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_p3_strides =
        (((DLTensor *)p3)[0].strides);
    void *p4_1 = (((DLTensor *)p4)[0].data);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_p4_shape = (((DLTensor *)p4)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_p4_strides =
        (((DLTensor *)p4)[0].strides);
    void *p5_1 = (((DLTensor *)p5)[0].data);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_p5_shape = (((DLTensor *)p5)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_p5_strides =
        (((DLTensor *)p5)[0].strides);
    void *p6_1 = (((DLTensor *)p6)[0].data);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_p6_shape = (((DLTensor *)p6)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_p6_strides =
        (((DLTensor *)p6)[0].strides);
    void *T_cast_1 = (((DLTensor *)T_cast)[0].data);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_T_cast_shape =
        (((DLTensor *)T_cast)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_T_cast_strides =
        (((DLTensor *)T_cast)[0].strides);
    if (!(tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_p0_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_p1_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_p2_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_p3_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_p4_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_p5_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_T_cast_strides == NULL))
    {
    }
    int16_t data_vec[384];
    void *data_pad = TVMBackendAllocWorkspace(1, dev_id, (uint64_t)1310, 0, 16);
    if (data_pad == NULL)
    {
        return -1;
    }
    for (int32_t bs_c_fused_h_fused = 0; bs_c_fused_h_fused < 128; ++bs_c_fused_h_fused)
    {
        for (int32_t w = 0; w < 3; ++w)
        {
            int32_t cse_var_1 = ((bs_c_fused_h_fused * 3) + w);
            data_vec[cse_var_1] = ((int16_t *)p0_1)[cse_var_1];
        }
    }
    for (int32_t i0_i1_fused_i2_fused = 0; i0_i1_fused_i2_fused < 131; ++i0_i1_fused_i2_fused)
    {
        for (int32_t i3 = 0; i3 < 5; ++i3)
        {
            ((int16_t *)data_pad)[((i0_i1_fused_i2_fused * 5) + i3)] =
                (((((1 <= i0_i1_fused_i2_fused) && (i0_i1_fused_i2_fused < 129)) && (1 <= i3)) && (i3 < 4))
                     ? data_vec[(((i0_i1_fused_i2_fused * 3) + i3) - 4)]
                     : (int16_t)0);
        }
    }
    for (int32_t occ_k_h_fused = 0; occ_k_h_fused < 8; ++occ_k_h_fused)
    {
        for (int32_t k_w = 0; k_w < 3; ++k_w)
        {
            for (int32_t ocb = 0; ocb < 4; ++ocb)
            {
                data_vec[(((occ_k_h_fused * 12) + (k_w * 4)) + ocb)] =
                    ((int16_t *)p1_1)[(((((occ_k_h_fused >> 2) * 48) + (ocb * 12)) + ((occ_k_h_fused & 3) * 3)) + k_w)];
            }
        }
    }
    for (int32_t ax0_ax1_outer_fused_ax2_fused = 0; ax0_ax1_outer_fused_ax2_fused < 256;
         ++ax0_ax1_outer_fused_ax2_fused)
    {
        int32_t conv2d_NCHWc_global[12];
        for (int32_t oc_block_c_init = 0; oc_block_c_init < 4; ++oc_block_c_init)
        {
            conv2d_NCHWc_global[oc_block_c_init] = 0;
        }
        for (int32_t oc_block_c_init_1 = 0; oc_block_c_init_1 < 4; ++oc_block_c_init_1)
        {
            conv2d_NCHWc_global[(oc_block_c_init_1 + 4)] = 0;
        }
        for (int32_t oc_block_c_init_2 = 0; oc_block_c_init_2 < 4; ++oc_block_c_init_2)
        {
            conv2d_NCHWc_global[(oc_block_c_init_2 + 8)] = 0;
        }
        for (int32_t kh = 0; kh < 4; ++kh)
        {
            for (int32_t kw = 0; kw < 3; ++kw)
            {
                for (int32_t oc_block_c = 0; oc_block_c < 4; ++oc_block_c)
                {
                    conv2d_NCHWc_global[oc_block_c] =
                        (conv2d_NCHWc_global[oc_block_c] +
                         (((int32_t)((
                              int16_t *)data_pad)[(((kh * 5) + ((ax0_ax1_outer_fused_ax2_fused & 127) * 5)) + kw)]) *
                          ((int32_t)data_vec[(((((ax0_ax1_outer_fused_ax2_fused >> 7) * 48) + (kh * 12)) + (kw * 4)) +
                                              oc_block_c)])));
                }
                for (int32_t oc_block_c_1 = 0; oc_block_c_1 < 4; ++oc_block_c_1)
                {
                    int32_t cse_var_2 = (oc_block_c_1 + 4);
                    conv2d_NCHWc_global[cse_var_2] =
                        (conv2d_NCHWc_global[cse_var_2] +
                         (((int32_t)((int16_t *)data_pad)[(
                              (((kh * 5) + ((ax0_ax1_outer_fused_ax2_fused & 127) * 5)) + kw) + 1)]) *
                          ((int32_t)data_vec[(((((ax0_ax1_outer_fused_ax2_fused >> 7) * 48) + (kh * 12)) + (kw * 4)) +
                                              oc_block_c_1)])));
                }
                for (int32_t oc_block_c_2 = 0; oc_block_c_2 < 4; ++oc_block_c_2)
                {
                    int32_t cse_var_3 = (oc_block_c_2 + 8);
                    conv2d_NCHWc_global[cse_var_3] =
                        (conv2d_NCHWc_global[cse_var_3] +
                         (((int32_t)((int16_t *)data_pad)[(
                              (((kh * 5) + ((ax0_ax1_outer_fused_ax2_fused & 127) * 5)) + kw) + 2)]) *
                          ((int32_t)data_vec[(((((ax0_ax1_outer_fused_ax2_fused >> 7) * 48) + (kh * 12)) + (kw * 4)) +
                                              oc_block_c_2)])));
                }
            }
        }
        for (int32_t ax3_inner = 0; ax3_inner < 3; ++ax3_inner)
        {
            for (int32_t ax1_inner = 0; ax1_inner < 4; ++ax1_inner)
            {
                int32_t cse_var_5 = (ax0_ax1_outer_fused_ax2_fused >> 7);
                int32_t cse_var_4 = ((cse_var_5 * 4) + ax1_inner);
                int32_t v_ = ((int32_t *)p6_1)[0] +
                             ((int32_t)(((((int64_t)(conv2d_NCHWc_global[((ax3_inner * 4) + ax1_inner)] +
                                                     ((int32_t *)p2_1)[cse_var_4])) *
                                          ((int64_t)((int32_t *)p3_1)[cse_var_4])) +
                                         ((int64_t)1 << ((int64_t)((((int32_t *)p5_1)[cse_var_4] + 31) - 1)))) >>
                                        ((int64_t)(((int32_t *)p5_1)[cse_var_4] + 31))));
                int32_t v__1 = (v_) < (127) ? (v_) : (127);
                ((int8_t *)T_cast_1)[(
                    (((cse_var_5 * 1536) + (ax1_inner * 384)) + ((ax0_ax1_outer_fused_ax2_fused & 127) * 3)) +
                    ax3_inner)] = ((int8_t)((v__1) > (-128) ? (v__1) : (-128)));
            }
        }
    }
    if (TVMBackendFreeWorkspace(1, dev_id, data_pad) != 0)
    {
        return -1;
    }
    return 0;
}

#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1(
        void *args, int32_t *arg_type_ids, int32_t num_args, void *out_ret_value, int32_t *out_ret_tcode,
        void *resource_handle)
{
    int32_t p0_code = arg_type_ids[0];
    int32_t p1_code = arg_type_ids[1];
    int32_t p2_code = arg_type_ids[2];
    int32_t p3_code = arg_type_ids[3];
    int32_t p4_code = arg_type_ids[4];
    int32_t p5_code = arg_type_ids[5];
    int32_t p6_code = arg_type_ids[6];
    int32_t T_cast_code = arg_type_ids[7];
    void *p0 = (((TVMValue *)args)[0].v_handle);
    void *p1 = (((TVMValue *)args)[1].v_handle);
    void *p2 = (((TVMValue *)args)[2].v_handle);
    void *p3 = (((TVMValue *)args)[3].v_handle);
    void *p4 = (((TVMValue *)args)[4].v_handle);
    void *p5 = (((TVMValue *)args)[5].v_handle);
    void *p6 = (((TVMValue *)args)[6].v_handle);
    void *T_cast = (((TVMValue *)args)[7].v_handle);
    void *p0_1 = (((DLTensor *)p0)[0].data);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1_p0_shape =
        (((DLTensor *)p0)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1_p0_strides =
        (((DLTensor *)p0)[0].strides);
    int32_t dev_id = (((DLTensor *)p0)[0].device.device_id);
    void *p1_1 = (((DLTensor *)p1)[0].data);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1_p1_shape =
        (((DLTensor *)p1)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1_p1_strides =
        (((DLTensor *)p1)[0].strides);
    void *p2_1 = (((DLTensor *)p2)[0].data);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1_p2_shape =
        (((DLTensor *)p2)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1_p2_strides =
        (((DLTensor *)p2)[0].strides);
    void *p3_1 = (((DLTensor *)p3)[0].data);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1_p3_shape =
        (((DLTensor *)p3)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1_p3_strides =
        (((DLTensor *)p3)[0].strides);
    void *p4_1 = (((DLTensor *)p4)[0].data);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1_p4_shape =
        (((DLTensor *)p4)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1_p4_strides =
        (((DLTensor *)p4)[0].strides);
    void *p5_1 = (((DLTensor *)p5)[0].data);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1_p5_shape =
        (((DLTensor *)p5)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1_p5_strides =
        (((DLTensor *)p5)[0].strides);
    void *p6_1 = (((DLTensor *)p6)[0].data);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1_p6_shape =
        (((DLTensor *)p6)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1_p6_strides =
        (((DLTensor *)p6)[0].strides);
    void *T_cast_1 = (((DLTensor *)T_cast)[0].data);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1_T_cast_shape =
        (((DLTensor *)T_cast)[0].shape);
    void *tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1_T_cast_strides =
        (((DLTensor *)T_cast)[0].strides);
    if (!(tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1_p0_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1_p1_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1_p2_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1_p3_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1_p4_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1_p5_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1_T_cast_strides == NULL))
    {
    }
    void *data_vec = TVMBackendAllocWorkspace(1, dev_id, (uint64_t)1024, 0, 16);
    if (data_vec == NULL)
    {
        return -1;
    }
    int16_t data_pad[360];
    for (int32_t bs_c_fused_h_fused = 0; bs_c_fused_h_fused < 84; ++bs_c_fused_h_fused)
    {
        for (int32_t vc = 0; vc < 4; ++vc)
        {
            ((int16_t *)data_vec)[((bs_c_fused_h_fused * 4) + vc)] =
                ((int16_t *)p0_1)[((((bs_c_fused_h_fused / 42) * 168) + (vc * 42)) + (bs_c_fused_h_fused % 42))];
        }
    }
    for (int32_t i0_i1_fused_i2_fused = 0; i0_i1_fused_i2_fused < 90; ++i0_i1_fused_i2_fused)
    {
        for (int32_t i4 = 0; i4 < 4; ++i4)
        {
            int32_t cse_var_1 = (i0_i1_fused_i2_fused % 45);
            data_pad[((i0_i1_fused_i2_fused * 4) + i4)] =
                (((1 <= cse_var_1) && (cse_var_1 < 43))
                     ? ((int16_t *)data_vec)[(((((i0_i1_fused_i2_fused / 45) * 168) + (cse_var_1 * 4)) + i4) - 4)]
                     : (int16_t)0);
        }
    }
    for (int32_t occ_k_h_fused = 0; occ_k_h_fused < 16; ++occ_k_h_fused)
    {
        for (int32_t icc = 0; icc < 2; ++icc)
        {
            for (int32_t icb = 0; icb < 4; ++icb)
            {
                for (int32_t ocb = 0; ocb < 4; ++ocb)
                {
                    int32_t cse_var_4 = (icb * 4);
                    int32_t cse_var_3 = (occ_k_h_fused & 3);
                    int32_t cse_var_2 = ((occ_k_h_fused >> 2) * 128);
                    ((int16_t *)data_vec)[((((cse_var_2 + (icc * 64)) + (cse_var_3 * 16)) + cse_var_4) + ocb)] =
                        ((int16_t *)p1_1)[((((cse_var_2 + (ocb * 32)) + (icc * 16)) + cse_var_4) + cse_var_3)];
                }
            }
        }
    }
    for (int32_t ax0_ax1_outer_fused_ax2_fused = 0; ax0_ax1_outer_fused_ax2_fused < 168;
         ++ax0_ax1_outer_fused_ax2_fused)
    {
        int32_t conv2d_NCHWc_global[4];
        for (int32_t oc_block_c_init = 0; oc_block_c_init < 4; ++oc_block_c_init)
        {
            conv2d_NCHWc_global[oc_block_c_init] = 0;
        }
        for (int32_t ic_outer = 0; ic_outer < 2; ++ic_outer)
        {
            for (int32_t kh = 0; kh < 4; ++kh)
            {
                for (int32_t ic_inner = 0; ic_inner < 4; ++ic_inner)
                {
                    for (int32_t oc_block_c = 0; oc_block_c < 4; ++oc_block_c)
                    {
                        conv2d_NCHWc_global[oc_block_c] =
                            (conv2d_NCHWc_global[oc_block_c] +
                             (((int32_t)data_pad[(
                                  (((ic_outer * 180) + (kh * 4)) + ((ax0_ax1_outer_fused_ax2_fused % 42) * 4)) +
                                  ic_inner)]) *
                              ((int32_t)((int16_t *)data_vec)[(
                                  (((((ax0_ax1_outer_fused_ax2_fused / 42) * 128) + (ic_outer * 64)) + (kh * 16)) +
                                   (ic_inner * 4)) +
                                  oc_block_c)])));
                    }
                }
            }
        }
        for (int32_t ax1_inner = 0; ax1_inner < 4; ++ax1_inner)
        {
            int32_t cse_var_6 = (ax0_ax1_outer_fused_ax2_fused / 42);
            int32_t cse_var_5 = ((cse_var_6 * 4) + ax1_inner);
            int32_t v_ = ((int32_t *)p6_1)[0] +
                         ((int32_t)(((((int64_t)(conv2d_NCHWc_global[ax1_inner] + ((int32_t *)p2_1)[cse_var_5])) *
                                      ((int64_t)((int32_t *)p3_1)[cse_var_5])) +
                                     ((int64_t)1 << ((int64_t)((((int32_t *)p5_1)[cse_var_5] + 31) - 1)))) >>
                                    ((int64_t)(((int32_t *)p5_1)[cse_var_5] + 31))));
            int32_t v__1 = (v_) < (127) ? (v_) : (127);
            ((int8_t *)T_cast_1)[(((cse_var_6 * 168) + (ax1_inner * 42)) + (ax0_ax1_outer_fused_ax2_fused % 42))] =
                ((int8_t)((v__1) > (-128) ? (v__1) : (-128)));
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
    TVM_DLL int32_t tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_cast(void *args, int32_t *arg_type_ids,
                                                                                 int32_t num_args, void *out_ret_value,
                                                                                 int32_t *out_ret_tcode,
                                                                                 void *resource_handle)
{
    int32_t p0_code = arg_type_ids[0];
    int32_t p1_code = arg_type_ids[1];
    int32_t p2_code = arg_type_ids[2];
    int32_t p3_code = arg_type_ids[3];
    int32_t T_cast_code = arg_type_ids[4];
    void *p0 = (((TVMValue *)args)[0].v_handle);
    void *p1 = (((TVMValue *)args)[1].v_handle);
    void *p2 = (((TVMValue *)args)[2].v_handle);
    void *p3 = (((TVMValue *)args)[3].v_handle);
    void *T_cast = (((TVMValue *)args)[4].v_handle);
    void *p0_1 = (((DLTensor *)p0)[0].data);
    void *tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_cast_p0_shape = (((DLTensor *)p0)[0].shape);
    void *tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_cast_p0_strides = (((DLTensor *)p0)[0].strides);
    int32_t dev_id = (((DLTensor *)p0)[0].device.device_id);
    void *p1_1 = (((DLTensor *)p1)[0].data);
    void *tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_cast_p1_shape = (((DLTensor *)p1)[0].shape);
    void *tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_cast_p1_strides = (((DLTensor *)p1)[0].strides);
    void *p2_1 = (((DLTensor *)p2)[0].data);
    void *tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_cast_p2_shape = (((DLTensor *)p2)[0].shape);
    void *tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_cast_p2_strides = (((DLTensor *)p2)[0].strides);
    void *p3_1 = (((DLTensor *)p3)[0].data);
    void *tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_cast_p3_shape = (((DLTensor *)p3)[0].shape);
    void *tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_cast_p3_strides = (((DLTensor *)p3)[0].strides);
    void *T_cast_1 = (((DLTensor *)T_cast)[0].data);
    void *tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_cast_T_cast_shape = (((DLTensor *)T_cast)[0].shape);
    void *tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_cast_T_cast_strides =
        (((DLTensor *)T_cast)[0].strides);
    if (!(tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_cast_p0_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_cast_p1_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_cast_p2_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_cast_T_cast_strides == NULL))
    {
    }
    void *packed_weight = TVMBackendAllocWorkspace(1, dev_id, (uint64_t)7168, 0, 16);
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
                ((int16_t *)packed_weight)[((cse_var_1 + (y * 8)) + x)] =
                    ((int16_t *)p1_1)[((cse_var_1 + (x * 224)) + y)];
            }
        }
    }
    for (int32_t ax1_outer_ax0_outer_fused = 0; ax1_outer_ax0_outer_fused < 2; ++ax1_outer_ax0_outer_fused)
    {
        int32_t compute_global[8];
        for (int32_t x_c_init = 0; x_c_init < 8; ++x_c_init)
        {
            compute_global[x_c_init] = 0;
        }
        for (int32_t k_outer = 0; k_outer < 224; ++k_outer)
        {
            for (int32_t x_c = 0; x_c < 8; ++x_c)
            {
                compute_global[x_c] =
                    (compute_global[x_c] +
                     (((int32_t)((int16_t *)p0_1)[k_outer]) *
                      ((int32_t)((
                          int16_t *)packed_weight)[(((ax1_outer_ax0_outer_fused * 1792) + (k_outer * 8)) + x_c)])));
            }
        }
        for (int32_t ax1_inner_inner = 0; ax1_inner_inner < 8; ++ax1_inner_inner)
        {
            int32_t cse_var_2 = ((ax1_outer_ax0_outer_fused * 8) + ax1_inner_inner);
            int32_t v_ =
                ((int32_t *)p3_1)[0] +
                ((int32_t)(((((0 != 0) ? (((int64_t)(compute_global[ax1_inner_inner] + ((int32_t *)p2_1)[cse_var_2]))
                                          << ((int64_t)0))
                                       : ((int64_t)(compute_global[ax1_inner_inner] + ((int32_t *)p2_1)[cse_var_2]))) *
                             (int64_t)1450140262) +
                            ((int64_t)1 << ((int64_t)((7 + 31) - 1)))) >>
                           ((int64_t)(7 + 31))));
            int32_t v__1 = (v_) < (127) ? (v_) : (127);
            ((int8_t *)T_cast_1)[cse_var_2] = ((int8_t)((v__1) > (-128) ? (v__1) : (-128)));
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
    TVM_DLL int32_t tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_subtract_cast_multiply(
        void *args, int32_t *arg_type_ids, int32_t num_args, void *out_ret_value, int32_t *out_ret_tcode,
        void *resource_handle)
{
    int32_t p0_code = arg_type_ids[0];
    int32_t p1_code = arg_type_ids[1];
    int32_t p2_code = arg_type_ids[2];
    int32_t T_multiply_code = arg_type_ids[3];
    void *p0 = (((TVMValue *)args)[0].v_handle);
    void *p1 = (((TVMValue *)args)[1].v_handle);
    void *p2 = (((TVMValue *)args)[2].v_handle);
    void *T_multiply = (((TVMValue *)args)[3].v_handle);
    void *p0_1 = (((DLTensor *)p0)[0].data);
    void *tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_subtract_cast_multiply_p0_shape =
        (((DLTensor *)p0)[0].shape);
    void *tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_subtract_cast_multiply_p0_strides =
        (((DLTensor *)p0)[0].strides);
    int32_t dev_id = (((DLTensor *)p0)[0].device.device_id);
    void *p1_1 = (((DLTensor *)p1)[0].data);
    void *tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_subtract_cast_multiply_p1_shape =
        (((DLTensor *)p1)[0].shape);
    void *tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_subtract_cast_multiply_p1_strides =
        (((DLTensor *)p1)[0].strides);
    void *p2_1 = (((DLTensor *)p2)[0].data);
    void *tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_subtract_cast_multiply_p2_shape =
        (((DLTensor *)p2)[0].shape);
    void *tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_subtract_cast_multiply_p2_strides =
        (((DLTensor *)p2)[0].strides);
    void *T_multiply_1 = (((DLTensor *)T_multiply)[0].data);
    void *tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_subtract_cast_multiply_T_multiply_shape =
        (((DLTensor *)T_multiply)[0].shape);
    void *tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_subtract_cast_multiply_T_multiply_strides =
        (((DLTensor *)T_multiply)[0].strides);
    if (!(tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_subtract_cast_multiply_p0_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_subtract_cast_multiply_p1_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_subtract_cast_multiply_p2_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_subtract_cast_multiply_T_multiply_strides == NULL))
    {
    }
    int16_t packed_weight[64];
    int32_t compute_global[4];
    for (int32_t y = 0; y < 16; ++y)
    {
        for (int32_t x = 0; x < 4; ++x)
        {
            packed_weight[((y * 4) + x)] = ((int16_t *)p1_1)[((x * 16) + y)];
        }
    }
    for (int32_t x_c_init = 0; x_c_init < 4; ++x_c_init)
    {
        compute_global[x_c_init] = 0;
    }
    for (int32_t k_outer = 0; k_outer < 16; ++k_outer)
    {
        for (int32_t x_c = 0; x_c < 4; ++x_c)
        {
            compute_global[x_c] = (compute_global[x_c] + (((int32_t)((int16_t *)p0_1)[k_outer]) *
                                                          ((int32_t)packed_weight[((k_outer * 4) + x_c)])));
        }
    }
    for (int32_t ax1_inner_inner = 0; ax1_inner_inner < 4; ++ax1_inner_inner)
    {
        int32_t v_ =
            ((int32_t)(((((0 != 0)
                              ? (((int64_t)(compute_global[ax1_inner_inner] + ((int32_t *)p2_1)[ax1_inner_inner]))
                                 << ((int64_t)0))
                              : ((int64_t)(compute_global[ax1_inner_inner] + ((int32_t *)p2_1)[ax1_inner_inner]))) *
                         (int64_t)1223677426) +
                        ((int64_t)1 << ((int64_t)((6 + 31) - 1)))) >>
                       ((int64_t)(6 + 31)))) -
            77;
        int32_t v__1 = (v_) < (127) ? (v_) : (127);
        ((float *)T_multiply_1)[ax1_inner_inner] =
            (((float)(((v__1) > (-128) ? (v__1) : (-128)) + 77)) * 2.622537e+00f);
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
    for (int32_t ax0_ax1_fused = 0; ax0_ax1_fused < 16; ++ax0_ax1_fused)
    {
        for (int32_t ax2 = 0; ax2 < 14; ++ax2)
        {
            ((int8_t *)pool_max_1)[((ax0_ax1_fused * 14) + ax2)] = (int8_t)-128;
            for (int32_t rv0 = 0; rv0 < 3; ++rv0)
            {
                int32_t cse_var_1 = ((ax0_ax1_fused * 14) + ax2);
                int8_t v_ = ((int8_t *)pool_max_1)[cse_var_1];
                int8_t v__1 = ((int8_t *)p0_1)[(((ax0_ax1_fused * 42) + (ax2 * 3)) + rv0)];
                ((int8_t *)pool_max_1)[cse_var_1] = ((v_) > (v__1) ? (v_) : (v__1));
            }
        }
    }
    return 0;
}

#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_max_pool2d_cast_subtract(void *args, int32_t *arg_type_ids, int32_t num_args,
                                                             void *out_ret_value, int32_t *out_ret_tcode,
                                                             void *resource_handle)
{
    int32_t p0_code = arg_type_ids[0];
    int32_t p1_code = arg_type_ids[1];
    int32_t T_subtract_code = arg_type_ids[2];
    void *p0 = (((TVMValue *)args)[0].v_handle);
    void *p1 = (((TVMValue *)args)[1].v_handle);
    void *T_subtract = (((TVMValue *)args)[2].v_handle);
    void *p0_1 = (((DLTensor *)p0)[0].data);
    void *tvmgen_fused_nn_max_pool2d_cast_subtract_p0_shape = (((DLTensor *)p0)[0].shape);
    void *tvmgen_fused_nn_max_pool2d_cast_subtract_p0_strides = (((DLTensor *)p0)[0].strides);
    int32_t dev_id = (((DLTensor *)p0)[0].device.device_id);
    void *p1_1 = (((DLTensor *)p1)[0].data);
    void *tvmgen_fused_nn_max_pool2d_cast_subtract_p1_shape = (((DLTensor *)p1)[0].shape);
    void *tvmgen_fused_nn_max_pool2d_cast_subtract_p1_strides = (((DLTensor *)p1)[0].strides);
    void *T_subtract_1 = (((DLTensor *)T_subtract)[0].data);
    void *tvmgen_fused_nn_max_pool2d_cast_subtract_T_subtract_shape = (((DLTensor *)T_subtract)[0].shape);
    void *tvmgen_fused_nn_max_pool2d_cast_subtract_T_subtract_strides = (((DLTensor *)T_subtract)[0].strides);
    if (!(tvmgen_fused_nn_max_pool2d_cast_subtract_p0_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_max_pool2d_cast_subtract_T_subtract_strides == NULL))
    {
    }
    int8_t pool_max[336];
    for (int32_t ax0_ax1_fused = 0; ax0_ax1_fused < 8; ++ax0_ax1_fused)
    {
        for (int32_t ax2 = 0; ax2 < 42; ++ax2)
        {
            pool_max[((ax0_ax1_fused * 42) + ax2)] = (int8_t)-128;
            for (int32_t rv0 = 0; rv0 < 3; ++rv0)
            {
                for (int32_t rv1 = 0; rv1 < 3; ++rv1)
                {
                    int32_t cse_var_1 = ((ax0_ax1_fused * 42) + ax2);
                    int8_t v_ = pool_max[cse_var_1];
                    int8_t v__1 = ((int8_t *)p0_1)[((((ax0_ax1_fused * 384) + (ax2 * 9)) + (rv0 * 3)) + rv1)];
                    pool_max[cse_var_1] = ((v_) > (v__1) ? (v_) : (v__1));
                }
            }
        }
    }
    for (int32_t ax0_ax1_fused_1 = 0; ax0_ax1_fused_1 < 8; ++ax0_ax1_fused_1)
    {
        for (int32_t ax2_1 = 0; ax2_1 < 42; ++ax2_1)
        {
            int32_t cse_var_2 = ((ax0_ax1_fused_1 * 42) + ax2_1);
            ((int16_t *)T_subtract_1)[cse_var_2] = (((int16_t)pool_max[cse_var_2]) - ((int16_t *)p1_1)[0]);
        }
    }
    return 0;
}

#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_nn_softmax_divide_round_add_clip_cast(void *args, int32_t *arg_type_ids,
                                                                       int32_t num_args, void *out_ret_value,
                                                                       int32_t *out_ret_tcode, void *resource_handle)
{
    int32_t p0_code = arg_type_ids[0];
    int32_t T_cast_code = arg_type_ids[1];
    void *p0 = (((TVMValue *)args)[0].v_handle);
    void *T_cast = (((TVMValue *)args)[1].v_handle);
    void *p0_1 = (((DLTensor *)p0)[0].data);
    void *tvmgen_fused_nn_softmax_divide_round_add_clip_cast_p0_shape = (((DLTensor *)p0)[0].shape);
    void *tvmgen_fused_nn_softmax_divide_round_add_clip_cast_p0_strides = (((DLTensor *)p0)[0].strides);
    int32_t dev_id = (((DLTensor *)p0)[0].device.device_id);
    void *T_cast_1 = (((DLTensor *)T_cast)[0].data);
    void *tvmgen_fused_nn_softmax_divide_round_add_clip_cast_T_cast_shape = (((DLTensor *)T_cast)[0].shape);
    void *tvmgen_fused_nn_softmax_divide_round_add_clip_cast_T_cast_strides = (((DLTensor *)T_cast)[0].strides);
    if (!(tvmgen_fused_nn_softmax_divide_round_add_clip_cast_p0_strides == NULL))
    {
    }
    if (!(tvmgen_fused_nn_softmax_divide_round_add_clip_cast_T_cast_strides == NULL))
    {
    }
    float T_softmax_maxelem[1];
    float T_softmax_exp[4];
    float T_softmax_expsum[1];
    T_softmax_maxelem[0] = -3.402823e+38f;
    for (int32_t k = 0; k < 4; ++k)
    {
        float v_ = T_softmax_maxelem[0];
        float v__1 = ((float *)p0_1)[k];
        T_softmax_maxelem[0] = ((v_) > (v__1) ? (v_) : (v__1));
    }
    for (int32_t i1 = 0; i1 < 4; ++i1)
    {
        T_softmax_exp[i1] = expf((((float *)p0_1)[i1] - T_softmax_maxelem[0]));
    }
    T_softmax_expsum[0] = 0.000000e+00f;
    for (int32_t k_1 = 0; k_1 < 4; ++k_1)
    {
        T_softmax_expsum[0] = (T_softmax_expsum[0] + T_softmax_exp[k_1]);
    }
    for (int32_t i1_1 = 0; i1_1 < 4; ++i1_1)
    {
        T_softmax_exp[i1_1] = (T_softmax_exp[i1_1] / T_softmax_expsum[0]);
    }
    for (int32_t ax1 = 0; ax1 < 4; ++ax1)
    {
        float v__2 = roundf((T_softmax_exp[ax1] * 2.560000e+02f)) + -1.280000e+02f;
        float v__3 = (v__2) < (1.270000e+02f) ? (v__2) : (1.270000e+02f);
        ((int8_t *)T_cast_1)[ax1] = ((int8_t)((v__3) > (-1.280000e+02f) ? (v__3) : (-1.280000e+02f)));
    }
    return 0;
}

#ifdef __cplusplus
extern "C"
#endif
    TVM_DLL int32_t tvmgen_fused_reshape_cast_subtract(void *args, int32_t *arg_type_ids, int32_t num_args,
                                                       void *out_ret_value, int32_t *out_ret_tcode,
                                                       void *resource_handle)
{
    int32_t p0_code = arg_type_ids[0];
    int32_t p1_code = arg_type_ids[1];
    int32_t T_subtract_code = arg_type_ids[2];
    void *p0 = (((TVMValue *)args)[0].v_handle);
    void *p1 = (((TVMValue *)args)[1].v_handle);
    void *T_subtract = (((TVMValue *)args)[2].v_handle);
    void *p0_1 = (((DLTensor *)p0)[0].data);
    void *tvmgen_fused_reshape_cast_subtract_p0_shape = (((DLTensor *)p0)[0].shape);
    void *tvmgen_fused_reshape_cast_subtract_p0_strides = (((DLTensor *)p0)[0].strides);
    int32_t dev_id = (((DLTensor *)p0)[0].device.device_id);
    void *p1_1 = (((DLTensor *)p1)[0].data);
    void *tvmgen_fused_reshape_cast_subtract_p1_shape = (((DLTensor *)p1)[0].shape);
    void *tvmgen_fused_reshape_cast_subtract_p1_strides = (((DLTensor *)p1)[0].strides);
    void *T_subtract_1 = (((DLTensor *)T_subtract)[0].data);
    void *tvmgen_fused_reshape_cast_subtract_T_subtract_shape = (((DLTensor *)T_subtract)[0].shape);
    void *tvmgen_fused_reshape_cast_subtract_T_subtract_strides = (((DLTensor *)T_subtract)[0].strides);
    if (!(tvmgen_fused_reshape_cast_subtract_p0_strides == NULL))
    {
    }
    if (!(tvmgen_fused_reshape_cast_subtract_T_subtract_strides == NULL))
    {
    }
    for (int32_t ax1_inner = 0; ax1_inner < 16; ++ax1_inner)
    {
        ((int16_t *)T_subtract_1)[ax1_inner] = (((int16_t)((int8_t *)p0_1)[ax1_inner]) - ((int16_t *)p1_1)[0]);
    }
    return 0;
}
