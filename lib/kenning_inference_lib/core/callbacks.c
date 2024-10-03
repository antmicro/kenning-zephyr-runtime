/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kenning_inference_lib/core/callbacks.h>
#include <kenning_inference_lib/core/model.h>
#include <kenning_inference_lib/core/runtime_wrapper.h>
#include <kenning_inference_lib/core/utils.h>

#ifdef CONFIG_LLEXT
#include <zephyr/llext/buf_loader.h>
#include <zephyr/llext/llext.h>
#endif // CONFIG_LLEXT

#ifndef __UNIT_TEST__
#include <zephyr/logging/log.h>
#else // __UNIT_TEST__
#include "mocks/llext.h"
#include "mocks/log.h"
#endif

LOG_MODULE_REGISTER(callbacks, CONFIG_CALLBACKS_LOG_LEVEL);

GENERATE_MODULE_STATUSES_STR(CALLBACKS);
const callback_ptr_t g_msg_callback[NUM_MESSAGE_TYPES] = {
#define ENTRY(msg_type, callback_func) callback_func,
    CALLBACKS_TABLE(ENTRY)
#undef ENTRY
};

extern const char *const MESSAGE_TYPE_STR[];

/**
 * Handles unsupported message
 *
 * @param request incoming message. It is overwritten with ERROR message
 *
 * @returns error status of the callback
 */
status_t unsupported_callback(message_t **request)
{
    status_t status = STATUS_OK;

    RETURN_ERROR_IF_POINTER_INVALID(request, CALLBACKS_STATUS_INV_PTR);

    LOG_WRN("Unsupported message received: %d (%s)", (*request)->message_type,
            MESSAGE_TYPE_STR[(*request)->message_type]);

    status = protocol_prepare_fail_resp(request);
    RETURN_ON_ERROR(status, status);

    return STATUS_OK;
}

/**
 * Handles OK message
 *
 * @param request incoming message. It is overwritten with NULL as there is no
 * response
 *
 * @returns error status of the callback
 */
status_t ok_callback(message_t **request)
{
    VALIDATE_REQUEST(MESSAGE_TYPE_OK, request);

    LOG_WRN("Unexpected message received: MESSAGE_TYPE_OK");
    *request = NULL;
    return STATUS_OK;
}

/**
 * Handles ERROR message
 *
 * @param request incoming message. It is overwritten with NULL as there is no
 * response
 *
 * @returns error status of the callback
 */
status_t error_callback(message_t **request)
{
    VALIDATE_REQUEST(MESSAGE_TYPE_ERROR, request);

    LOG_WRN("Unexpected message received: MESSAGE_TYPE_ERROR");
    *request = NULL;
    return STATUS_OK;
}

/**
 * Handles DATA message that contains model input. It calls model's function
 * that loads it.
 *
 * @param request incoming message. It is overwritten by the response message
 * (OK/ERROR message)
 *
 * @returns error status of the callback
 */
status_t data_callback(message_t **request)
{
    status_t status = STATUS_OK;

    VALIDATE_REQUEST(MESSAGE_TYPE_DATA, request);

    status = model_load_input((*request)->payload, MESSAGE_SIZE_PAYLOAD((*request)->message_size));

    CHECK_STATUS_LOG(status, request, "model_load_input returned 0x%x (%s)", status, get_status_str(status));

    status = protocol_prepare_success_resp(request);
    RETURN_ON_ERROR(status, status);

    return STATUS_OK;
}

/**
 * Handles MODEL message that contains model data. It calls model's function
 * that loads the model.
 *
 * @param request incoming message. It is overwritten by the response message
 * (OK/ERROR message)
 *
 * @returns error status of the callback
 */
status_t model_callback(message_t **request)
{
    status_t status = STATUS_OK;

    VALIDATE_REQUEST(MESSAGE_TYPE_MODEL, request);

    status = model_load_weights((*request)->payload, MESSAGE_SIZE_PAYLOAD((*request)->message_size));

    CHECK_STATUS_LOG(status, request, "model_load_weights returned 0x%x (%s)", status, get_status_str(status));

    status = protocol_prepare_success_resp(request);
    RETURN_ON_ERROR(status, status);

    return STATUS_OK;
}

/**
 * Handles PROCESS message. It calls model's function that runs it
 *
 * @param request incoming message. It is overwritten by the response message
 * (OK/ERROR message)
 *
 * @returns error status of the callback
 */
status_t process_callback(message_t **request)
{
    status_t status = STATUS_OK;

    VALIDATE_REQUEST(MESSAGE_TYPE_PROCESS, request);

    status = model_run();

    CHECK_STATUS_LOG(status, request, "model_run returned 0x%x (%s)", status, get_status_str(status));

    status = protocol_prepare_success_resp(request);
    RETURN_ON_ERROR(status, status);

    return STATUS_OK;
}

/**
 * Handles OUTPUT message. It retrieves model inference output and sends it back
 *
 * @param request incoming message. It is overwritten by the response message
 * (DATA message containing model output or ERROR message)
 *
 * @returns error status of the callback
 */
status_t output_callback(message_t **request)
{
    status_t status = STATUS_OK;
    size_t model_output_size = 0;

    VALIDATE_REQUEST(MESSAGE_TYPE_OUTPUT, request);

    status = model_get_output(MAX_MESSAGE_SIZE_BYTES - sizeof(message_t), (*request)->payload, &model_output_size);

    CHECK_STATUS_LOG(status, request, "model_get_output returned 0x%x (%s)", status, get_status_str(status));

    (*request)->message_size = model_output_size + sizeof(message_type_t);
    (*request)->message_type = MESSAGE_TYPE_OK;

    return STATUS_OK;
}

/**
 * Handles STATS message. It retrieves model statistics
 *
 * @param request incoming message. It is overwritten by the response message
 * (STATS message containing model statistics or ERROR message)
 *
 * @returns error status of the callback
 */
status_t stats_callback(message_t **request)
{
    status_t status = STATUS_OK;
    size_t statistics_length = 0;

    VALIDATE_REQUEST(MESSAGE_TYPE_STATS, request);

    status = model_get_statistics(MAX_MESSAGE_SIZE_BYTES - sizeof(message_t), (uint8_t *)&(*request)->payload,
                                  &statistics_length);

    CHECK_STATUS_LOG(status, request, "model_get_statistics returned 0x%x (%s)", status, get_status_str(status));

    (*request)->message_size = statistics_length + sizeof(message_type_t);
    (*request)->message_type = MESSAGE_TYPE_OK;

    return STATUS_OK;
}

/**
 * Handles IOSPEC message. It loads model IO specification
 *
 * @param request incoming message. It is overwritten by the response message
 * (OK/ERROR message)
 *
 * @returns error status of the callback
 */
status_t iospec_callback(message_t **request)
{
    status_t status = STATUS_OK;

    VALIDATE_REQUEST(MESSAGE_TYPE_IOSPEC, request);

    status = model_load_struct((*request)->payload, MESSAGE_SIZE_PAYLOAD((*request)->message_size));

    CHECK_STATUS_LOG(status, request, "model_load_struct returned 0x%x (%s)", status, get_status_str(status));

    status = protocol_prepare_success_resp(request);
    RETURN_ON_ERROR(status, status);

    return STATUS_OK;
}

#if defined(CONFIG_LLEXT) || defined(CONFIG_ZTEST)

/**
 * Handles RUNTIME message. It loads runtime provided as loadable linkable extension.
 *
 * @param request incoming message. It is overwritten by the response message (OK/ERROR message)
 *
 * @returns error status of the callback
 */
status_t runtime_callback(message_t **request)
{
    status_t status = STATUS_OK;
    int llext_status = 0;

    VALIDATE_REQUEST(MESSAGE_TYPE_RUNTIME, request);

    size_t llext_size = MESSAGE_SIZE_PAYLOAD((*request)->message_size);

    LOG_INF("Attempting to load %d", llext_size);

    struct llext_buf_loader buf_loader = LLEXT_BUF_LOADER((*request)->payload, llext_size);
    struct llext_loader *loader = &buf_loader.loader;
    struct llext_load_param ldr_param = LLEXT_LOAD_PARAM_DEFAULT;

    do
    {
        // deinitalize runtime
        status = runtime_deinit();
        BREAK_ON_ERROR(status);

        // unload LLEXT if already exists
        struct llext *p_llext = llext_by_name("runtime");
        if (IS_VALID_POINTER(p_llext))
        {
            llext_teardown(p_llext);
            llext_status = llext_unload(&p_llext);
            if (0 != llext_status)
            {
                LOG_ERR("LLEXT runtime unload error: %d", llext_status);
                status = CALLBACKS_STATUS_ERROR;
                break;
            }
        }

        // load LLEXT
        llext_status = llext_load(loader, "runtime", &p_llext, &ldr_param);
        llext_bringup(p_llext);
        if (0 != llext_status)
        {
            LOG_ERR("LLEXT runtme load error: %d", llext_status);
            status = CALLBACKS_STATUS_ERROR;
            break;
        }
        LOG_INF("LLEXT loaded symbols (%d):", p_llext->exp_tab.sym_cnt);
        for (int i = 0; i < p_llext->exp_tab.sym_cnt; ++i)
        {
            struct llext_symbol *sym = &p_llext->exp_tab.syms[i];
            LOG_INF("%s at (0x%x)", sym->name, (uint32_t)sym->addr);
        }

    } while (0);

    if (status == STATUS_OK)
    {
        status = model_init();
    }

    PREPARE_RESPONSE(status);

    return status;
}

#endif // defined(CONFIG_LLEXT) || defined(CONFIG_ZTEST)
