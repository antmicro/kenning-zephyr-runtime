/*
 * Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "kenning_inference_lib/core/inference_server.h"
#include "kenning_inference_lib/core/utils.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app, CONFIG_APP_LOG_LEVEL);

ZPL_CODE_SCOPE_DEFINE(client_request,
#ifdef CONFIG_KENNING_ZEPHELIN_TRACE_REQUESTS
                      true
#else
                      false
#endif
);

int main(void)
{
    if (STATUS_OK != init_server())
    {
        LOG_ERR("Server init failed");
        return 1;
    }
    // main runtime loop
    while (1)
    {
        ZPL_MARK_CODE_SCOPE(client_request)
        {
            protocol_event_t event;
            if (STATUS_OK == wait_for_protocol_event(&event))
            {
                handle_protocol_event(&event);
            }
        }
    }
    return 0;
}
