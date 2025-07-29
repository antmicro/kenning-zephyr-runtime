/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "kenning_inference_lib/core/inference_server.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app, CONFIG_APP_LOG_LEVEL);

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
        protocol_event_t event;

        if (STATUS_OK == wait_for_protocol_event(&event))
        {
            handle_protocol_event(&event);
        }
    }
    return 0;
}
