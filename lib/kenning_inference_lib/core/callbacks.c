/*
 * Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>

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

#ifdef CONFIG_KENNING_SEND_LOGS
#include "kenning_inference_lib/core/logger.h"
#endif

LOG_MODULE_REGISTER(callbacks, CONFIG_CALLBACKS_LOG_LEVEL);

GENERATE_MODULE_STATUSES_STR(CALLBACKS);
const callback_ptr_t g_msg_callback[NUM_MESSAGE_TYPES] = {
#define ENTRY(msg_type, callback_func) callback_func,
    CALLBACKS_TABLE(ENTRY)
#undef ENTRY
};

extern const char *const MESSAGE_TYPE_STR[];

static bool client_connected = false;

/**
 * Handles unsupported message
 *
 * @param request incoming request.
 * @param resp_payload payload, that will be sent in response by the server (empty here)
 *
 * @returns error status of the callback
 */
status_t unsupported_callback(protocol_event_t *request, protocol_payload_t *resp_payload)
{
    status_t status = STATUS_OK;

    LOG_WRN("Unsupported message received: %d (%s)", request->message_type, MESSAGE_TYPE_STR[request->message_type]);

    RETURN_ON_ERROR(status, status);

    return STATUS_OK;
}

/**
 * Serves as a way for client (Kenning) to initiate communication or signal end of communication.
 * PING request with the SUCCESS flag set means that Kenning has connected and wants to start
 * an inference session. Request with FAIL flag set means that the inference sessions has ended and
 * Kenning is disconnecting. Request with both flags set means ending the previous session and starting a new one.
 *
 * @param request incoming request
 * @param resp_payload payload, that will be sent in response by the server (empty here)
 *
 * @returns error status of the callback
 */
status_t ping_callback(protocol_event_t *request, protocol_payload_t *resp_payload)
{
    if (request->flags.general_purpose_flags.fail)
    {
        client_connected = false;
        LOG_INF("Client disconnected.");
#ifdef CONFIG_KENNING_SEND_LOGS
        logger_stop();
#endif
    }
    if (request->flags.general_purpose_flags.success)
    {
        if (client_connected)
        {
            LOG_ERR("Client already connected.");
            return CALLBACKS_STATUS_ERROR;
        }
        else
        {
            client_connected = true;
#ifdef CONFIG_KENNING_SEND_LOGS
            logger_start();
#endif
            LOG_INF("Client connected");
            return STATUS_OK;
        }
    }
    return STATUS_OK;
}

/**
 * Handles DATA message that contains model input. It calls model's function
 * that loads it.
 *
 * @param request incoming request.
 * @param resp_payload payload, that will be sent in response by the server (empty here)
 *
 * @returns error status of the callback
 */
status_t data_callback(protocol_event_t *request, protocol_payload_t *resp_payload) // TODO
{
    status_t status = STATUS_OK;

    VALIDATE_HEADER(MESSAGE_TYPE_DATA, request);

    status = model_load_input_from_loader(request->payload.size);

    CHECK_STATUS_LOG(status, "model_load_input returned 0x%x (%s)", status, get_status_str(status));

    return status;
}

/**
 * Handles MODEL message that contains model data. It calls model's function
 * that loads the model.
 *
 * @param request incoming request.
 * @param resp_payload payload, that will be sent in response by the server (empty here)
 *
 * @returns error status of the callback
 */
status_t model_callback(protocol_event_t *request, protocol_payload_t *resp_payload)
{
    status_t status = STATUS_OK;

    VALIDATE_HEADER(MESSAGE_TYPE_MODEL, request);

    status = model_load_weights_from_loader();

    CHECK_STATUS_LOG(status, "model_load_weights returned 0x%x (%s)", status, get_status_str(status));

    RETURN_ON_ERROR(status, status);

    return STATUS_OK;
}

/**
 * Handles PROCESS message. It calls model's function that runs it
 *
 * @param request incoming request.
 * @param resp_payload payload, that will be sent in response by the server (empty here)
 *
 * @returns error status of the callback
 */
status_t process_callback(protocol_event_t *request, protocol_payload_t *resp_payload)
{
    status_t status = STATUS_OK;

    VALIDATE_HEADER(MESSAGE_TYPE_PROCESS, request);

    status = model_run_bench();

    CHECK_STATUS_LOG(status, "model_run returned 0x%x (%s)", status, get_status_str(status));

    return status;
}

/**
 * Handles OUTPUT message. It retrieves model inference output and sends it back
 *
 * @param request incoming request.
 * @param resp_payload payload, that will be sent in response by the server (model output)
 *
 * @returns error status of the callback
 */
status_t output_callback(protocol_event_t *request, protocol_payload_t *resp_payload)
{
    status_t status = STATUS_OK;
    size_t model_output_size = 0;

    VALIDATE_HEADER(MESSAGE_TYPE_OUTPUT, request);

    status = model_get_output(CONFIG_KENNING_RESPONSE_PAYLOAD_SIZE, resp_payload->raw_bytes, &model_output_size);

    CHECK_STATUS_LOG(status, "model_get_output returned 0x%x (%s)", status, get_status_str(status));

    resp_payload->size = model_output_size;
    return STATUS_OK;
}

/**
 * Handles STATS message. It retrieves model statistics
 *
 * @param request incoming request.
 * @param resp_payload payload, that will be sent in response by the server (inference statistics)
 *
 * @returns error status of the callback
 */
status_t stats_callback(protocol_event_t *request, protocol_payload_t *resp_payload)
{
    status_t status = STATUS_OK;
    size_t statistics_length = 0;

    VALIDATE_HEADER(MESSAGE_TYPE_STATS, request);

    status = model_get_statistics(CONFIG_KENNING_RESPONSE_PAYLOAD_SIZE, resp_payload->raw_bytes, &statistics_length);

    CHECK_STATUS_LOG(status, "model_get_statistics returned 0x%x (%s)", status, get_status_str(status));

    resp_payload->size = statistics_length;
    return STATUS_OK;
}

/**
 * Handles IOSPEC message. It loads model IO specification
 *
 * @param request incoming request.
 * @param resp_payload payload, that will be sent in response by the server (empty here)
 *
 * @returns error status of the callback
 */
status_t iospec_callback(protocol_event_t *request, protocol_payload_t *resp_payload)
{
    status_t status = STATUS_OK;

    VALIDATE_HEADER(MESSAGE_TYPE_IOSPEC, request);

    status = model_load_struct_from_loader();

    CHECK_STATUS_LOG(status, "model_load_struct returned 0x%x (%s)", status, get_status_str(status));

    return status;
}

#if defined(CONFIG_LLEXT) || defined(CONFIG_ZTEST)

/**
 * Handles RUNTIME message. It loads runtime provided as loadable linkable extension.
 *
 * @param request incoming request.
 * @param resp_payload payload, that will be sent in response by the server (empty here)
 *
 * @returns error status of the callback
 */
status_t runtime_callback(protocol_event_t *request, protocol_payload_t *resp_payload)
{
    status_t status = STATUS_OK;
    int llext_status = 0;

    VALIDATE_HEADER(MESSAGE_TYPE_RUNTIME, request);

    size_t llext_size = request->payload.size;

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
            break;
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

    CHECK_STATUS_LOG(status, "LLEXT runtime load returned: 0x%x (%s)", status, get_status_str(status));

    RETURN_ON_ERROR(status, status);

    return status;
}

#endif // defined(CONFIG_LLEXT) || defined(CONFIG_ZTEST)
