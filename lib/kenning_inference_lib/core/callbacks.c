/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kenning_inference_lib/core/callbacks.h>
#include <kenning_inference_lib/core/loaders.h>
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
status_t unsupported_callback(message_hdr_t *hdr, resp_message_t *resp)
{
    status_t status = STATUS_OK;

    LOG_WRN("Unsupported message received: %d (%s)", hdr->message_type, MESSAGE_TYPE_STR[hdr->message_type]);

    status = protocol_prepare_fail_resp(resp);
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
status_t ok_callback(message_hdr_t *hdr, resp_message_t *resp)
{
    status_t status = STATUS_OK;
    VALIDATE_HEADER(MESSAGE_TYPE_OK, hdr);

    LOG_WRN("Unexpected message received: MESSAGE_TYPE_OK");
    status = protocol_prepare_fail_resp(resp);
    return status;
}

/**
 * Handles ERROR message
 *
 * @param request incoming message. It is overwritten with NULL as there is no
 * response
 *
 * @returns error status of the callback
 */
status_t error_callback(message_hdr_t *hdr, resp_message_t *resp)
{
    status_t status = STATUS_OK;
    VALIDATE_HEADER(MESSAGE_TYPE_ERROR, hdr);

    LOG_WRN("Unexpected message received: MESSAGE_TYPE_ERROR");
    status = protocol_prepare_fail_resp(resp);
    return status;
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
status_t data_callback(message_hdr_t *hdr, resp_message_t *resp) // TODO
{
    status_t status = STATUS_OK;

    VALIDATE_HEADER(MESSAGE_TYPE_DATA, hdr);

    status = model_load_input_from_loader(MESSAGE_SIZE_PAYLOAD(hdr->message_size));

    CHECK_STATUS_LOG(status, resp, "model_load_input returned 0x%x (%s)", status, get_status_str(status));

    status = protocol_prepare_success_resp(resp);
    return status;
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
status_t model_callback(message_hdr_t *hdr, resp_message_t *resp)
{
    status_t status = STATUS_OK;

    VALIDATE_HEADER(MESSAGE_TYPE_MODEL, hdr);

    status = model_load_weights_from_loader();

    CHECK_STATUS_LOG(status, resp, "model_load_weights returned 0x%x (%s)", status, get_status_str(status));

    status = protocol_prepare_success_resp(resp);
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
status_t process_callback(message_hdr_t *hdr, resp_message_t *resp)
{
    status_t status = STATUS_OK;

    VALIDATE_HEADER(MESSAGE_TYPE_PROCESS, hdr);

    status = model_run();

    CHECK_STATUS_LOG(status, resp, "model_run returned 0x%x (%s)", status, get_status_str(status));

    status = protocol_prepare_success_resp(resp);
    return status;
}

/**
 * Handles OUTPUT message. It retrieves model inference output and sends it back
 *
 * @param request incoming message. It is overwritten by the response message
 * (DATA message containing model output or ERROR message)
 *
 * @returns error status of the callback
 */
status_t output_callback(message_hdr_t *hdr, resp_message_t *resp)
{
    status_t status = STATUS_OK;
    size_t model_output_size = 0;

    VALIDATE_HEADER(MESSAGE_TYPE_OUTPUT, hdr);

    status = model_get_output(CONFIG_KENNING_RESPONSE_PAYLOAD_SIZE, resp->payload, &model_output_size);

    CHECK_STATUS_LOG(status, resp, "model_get_output returned 0x%x (%s)", status, get_status_str(status));

    resp->hdr.message_size = model_output_size + sizeof(message_type_t);
    resp->hdr.message_type = MESSAGE_TYPE_OK;

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
status_t stats_callback(message_hdr_t *hdr, resp_message_t *resp)
{
    status_t status = STATUS_OK;
    size_t statistics_length = 0;

    VALIDATE_HEADER(MESSAGE_TYPE_STATS, hdr);

    status = model_get_statistics(CONFIG_KENNING_RESPONSE_PAYLOAD_SIZE, resp->payload, &statistics_length);

    CHECK_STATUS_LOG(status, resp, "model_get_statistics returned 0x%x (%s)", status, get_status_str(status));

    resp->hdr.message_size = statistics_length + sizeof(message_type_t);
    resp->hdr.message_type = MESSAGE_TYPE_OK;

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
status_t iospec_callback(message_hdr_t *hdr, resp_message_t *resp)
{
    status_t status = STATUS_OK;

    VALIDATE_HEADER(MESSAGE_TYPE_IOSPEC, hdr);

    status = model_load_struct_from_loader();

    CHECK_STATUS_LOG(status, resp, "model_load_struct returned 0x%x (%s)", status, get_status_str(status));

    status = protocol_prepare_success_resp(resp);
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
status_t runtime_callback(message_hdr_t *hdr, resp_message_t *resp)
{
    status_t status = STATUS_OK;
    int llext_status = 0;

    VALIDATE_HEADER(MESSAGE_TYPE_RUNTIME, hdr);

    size_t llext_size = MESSAGE_SIZE_PAYLOAD(hdr->message_size);

    LOG_INF("Attempting to load %d", llext_size);
    struct llext_buf_loader buf_loader = LLEXT_BUF_LOADER(g_ldr_tables[0][LOADER_TYPE_RUNTIME]->addr, llext_size);
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
            LOG_ERR("LLEXT already exists (%x). This should never happen", (unsigned int)p_llext);
            status = CALLBACKS_STATUS_ERROR;
        }

        // load LLEXT
        llext_status = llext_load(loader, "runtime", &p_llext, &ldr_param);
        BREAK_ON_TRUE_LOG_SET_STATUS(status, CALLBACKS_STATUS_ERROR, llext_status, "LLEXT runtime load error: %d",
                                     llext_status);

        llext_status = llext_bringup(p_llext);
        BREAK_ON_TRUE_LOG_SET_STATUS(status, CALLBACKS_STATUS_ERROR, llext_status, "LLEXT runtime bringup error: %d",
                                     llext_status);

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

    CHECK_STATUS_LOG(status, resp, "LLEXT runtime load returned: 0x%x (%s)", status, get_status_str(status));

    return status;
}

#endif // defined(CONFIG_LLEXT) || defined(CONFIG_ZTEST)
