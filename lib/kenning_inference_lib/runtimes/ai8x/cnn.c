/*
 * Copyright (c) 2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cnn.h"

#include <zephyr/drivers/clock_control.h>
#include <zephyr/drivers/clock_control/adi_max32_clock_control.h>
#include <zephyr/irq.h>
#include <zephyr/kernel.h>

#define CNN_IRQn 101

// registers should be obtained from dts (GCR is specified, GCFR isn't)
struct gcfr_regs *const GCFR = (struct gcfr_regs *)CNN_GCFR_REG_ADDR;
struct gcr_regs *const GCR = (struct gcr_regs *)CNN_GCR_REG_ADDR;

// CNN completion indicator
static int cnn_done;

void cnn_isr(void)
{
    *((volatile uint32_t *)CNN_REG_QUADRANT_0_ADDR) &= CNN_REG_QUADRANT_ISR_ACK;
    *((volatile uint32_t *)CNN_REG_QUADRANT_1_ADDR) &= CNN_REG_QUADRANT_ISR_ACK;
    *((volatile uint32_t *)CNN_REG_QUADRANT_2_ADDR) &= CNN_REG_QUADRANT_ISR_ACK;
    *((volatile uint32_t *)CNN_REG_QUADRANT_3_ADDR) &= CNN_REG_QUADRANT_ISR_ACK;
    cnn_done = 1;
}

void cnn_set_clock(uint32_t div, uint32_t sel)
{
    union gcr_pclkdiv pclkdiv;
    pclkdiv.bits = GCR->pclkdiv;
    pclkdiv.fields.cnnclkdiv = div;
    pclkdiv.fields.cnnclksel = sel;
    GCR->pclkdiv = pclkdiv.bits;
}

void cnn_enable()
{
    const struct device *const gcr = DEVICE_DT_GET(DT_NODELABEL(gcr));
    struct max32_perclk sys = {
        .bus = ADI_MAX32_CLOCK_BUS0,
        .bit = 25,
        .clk_src = 8, // IPLL
    };

    GCFR->reg3 = 0xf; // Reset
    GCFR->reg1 = 0xf; // Mask memory
    GCFR->reg0 = 0xf; // Power
    k_msleep(10);
    GCFR->reg2 = 0x0; // Iso
    GCFR->reg3 = 0x0; // Reset

    cnn_set_clock(1, 3); // DIV 4 IPLL

    clock_control_on(gcr, &sys);
    IRQ_CONNECT(CNN_IRQn, 0, cnn_isr, NULL, 0);
    irq_enable(CNN_IRQn);
    return;
}

void cnn_disable()
{
    const struct device *const gcr = DEVICE_DT_GET(DT_NODELABEL(gcr));
    struct max32_perclk sys = {
        .bus = ADI_MAX32_CLOCK_BUS0,
        .bit = 25,
        .clk_src = 8, // IPLL
    };

    irq_disable(CNN_IRQn);

    clock_control_off(gcr, &sys);

    GCFR->reg3 = 0xf;  // Reset
    GCFR->reg2 |= 0xf; // Iso
    GCFR->reg0 = 0x0;  // Power
    GCFR->reg1 = 0x0;  // Mask memory
    GCFR->reg3 = 0x0;  // Reset
    return;
}

void cnn_start(void)
{
    *((volatile uint32_t *)CNN_REG_QUADRANT_0_ADDR) = CNN_REG_QUADRANT_0_ENABLE;
    *((volatile uint32_t *)CNN_REG_QUADRANT_1_ADDR) = CNN_REG_QUADRANT_1_ENABLE;
    *((volatile uint32_t *)CNN_REG_QUADRANT_2_ADDR) = CNN_REG_QUADRANT_2_ENABLE;
    *((volatile uint32_t *)CNN_REG_QUADRANT_3_ADDR) = CNN_REG_QUADRANT_3_ENABLE;

    *((volatile uint32_t *)CNN_REG_QUADRANT_0_ADDR) = CNN_REG_QUADRANT_MASTER_ENABLE;
}

int cnn_wait(int timeout)
{
    int status = CNN_OK;
    int time = 0;

    while (!cnn_done)
    {
        k_msleep(1);
        if (++time > timeout)
        {
            status = CNN_FAIL;
            break;
        }
    }

    return status;
}
