/*
 * Copyright (c) 2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_RUNTIMES_AI8X_CNN_H_
#define KENNING_INFERENCE_LIB_RUNTIMES_AI8X_CNN_H_

#include <stdint.h>

#define CNN_OK 0
#define CNN_FAIL 1

#define CNN_GCFR_REG_ADDR 0x40005800UL
#define CNN_GCR_REG_ADDR 0x40000000UL

// Taken from
// https://github.com/analogdevicesinc/msdk/tree/89609813e1bac31cd7d93e1bb60b0aecb5a90bc6/Examples/MAX78002/CNN

#define CNN_REG_QUADRANT_0_ADDR 0x51000000
#define CNN_REG_QUADRANT_1_ADDR 0x52000000
#define CNN_REG_QUADRANT_2_ADDR 0x53000000
#define CNN_REG_QUADRANT_3_ADDR 0x54000000

#define CNN_REG_QUADRANT_ISR_ACK (~((1 << 12) | 1))
#define CNN_REG_QUADRANT_0_ENABLE 0x00100808
#define CNN_REG_QUADRANT_1_ENABLE 0x00100809
#define CNN_REG_QUADRANT_2_ENABLE 0x00100809
#define CNN_REG_QUADRANT_3_ENABLE 0x00100809
#define CNN_REG_QUADRANT_MASTER_ENABLE 0x00100009

/**
 * GCFR registers.
 *
 * Based on
 * https://github.com/analogdevicesinc/msdk/blob/89609813e1bac31cd7d93e1bb60b0aecb5a90bc6/Libraries/CMSIS/Device/Maxim/MAX78002/Include/gcfr_regs.h#L80
 */
struct __attribute__((packed)) gcfr_regs
{
    volatile uint32_t reg0; /* 0x00: GCFR REG0 Register */
    volatile uint32_t reg1; /* 0x04: GCFR REG1 Register */
    volatile uint32_t reg2; /* 0x08: GCFR REG2 Register */
    volatile uint32_t reg3; /* 0x0C: GCFR REG3 Register */
};

/**
 * GCR registers.
 *
 * Based on
 * https://github.com/analogdevicesinc/msdk/blob/89609813e1bac31cd7d93e1bb60b0aecb5a90bc6/Libraries/CMSIS/Device/Maxim/MAX78002/Include/gcr_regs.h#L80
 */
struct __attribute__((packed)) gcr_regs
{
    volatile uint32_t sysctrl;   /* 0x00 GCR SYSCTRL Register */
    volatile uint32_t rst0;      /* 0x04 GCR RST0 Register */
    volatile uint32_t clkctrl;   /* 0x08 GCR CLKCTRL Register */
    volatile uint32_t pm;        /* 0x0C GCR PM Register */
    volatile uint32_t ipll_ctrl; /* 0x10 GCR IPLL_CTRL Register */
    volatile uint32_t rsv_0x14;
    volatile uint32_t pclkdiv; /* 0x18: GCR PCLKDIV Register */
    volatile uint32_t rsv_0x1c_0x23[2];
    volatile uint32_t pclkdis0; /* 0x24: GCR PCLKDIS0 Register */
    volatile uint32_t memctrl;  /* 0x28: GCR MEMCTRL Register */
    volatile uint32_t memz;     /* 0x2C: GCR MEMZ Register */
    volatile uint32_t rsv_0x30_0x3f[4];
    volatile uint32_t sysst;    /* 0x40: GCR SYSST Register */
    volatile uint32_t rst1;     /* 0x44: GCR RST1 Register */
    volatile uint32_t pclkdis1; /* 0x48: GCR PCLKDIS1 Register */
    volatile uint32_t eventen;  /* 0x4C: GCR EVENTEN Register */
    volatile uint32_t revision; /* 0x50: GCR REVISION Register */
    volatile uint32_t sysie;    /* 0x54: GCR SYSIE Register */
    volatile uint32_t rsv_0x58_0x63[3];
    volatile uint32_t eccerr;  /* 0x64: GCR ECCERR Register */
    volatile uint32_t eccced;  /* 0x68: GCR ECCCED Register */
    volatile uint32_t eccie;   /* 0x6C: GCR ECCIE Register */
    volatile uint32_t eccaddr; /* 0x70: GCR ECCADDR Register */
    volatile uint32_t rsv_0x74_0x7f[3];
    volatile uint32_t gpr0; /* 0x80: GCR GPR0 Register */
};

/**
 * GCR PCLKDIV register.
 *
 * Based on MAX78002 User Guide and
 * https://github.com/analogdevicesinc/msdk/blob/89609813e1bac31cd7d93e1bb60b0aecb5a90bc6/Examples/MAX78002/CNN/kinetics/cnn.c#L1613
 */
union gcr_pclkdiv
{
    uint32_t bits;
    struct __attribute__((packed))
    {
        uint32_t RES1 : 14;
        uint32_t cnnclkdiv : 3;
        uint32_t cnnclksel : 2;
        uint32_t RES2 : 13;
    } fields;
};

/**
 * CNN interrupt handler
 */
void cnn_isr(void);

/**
 * Setups CNN clock
 *
 * @param div clock divider
 * @param sel clock selector
 */
void cnn_set_clock(uint32_t div, uint32_t sel);

/**
 * Enables CNN accelerator
 */
void cnn_enable();

/**
 * Disables CNN accelerator
 */
void cnn_disable();

/**
 * Starts CNN accelerator
 */
void cnn_start();

/**
 * Waits for CNN accelerator completion
 *
 * @param timeout waiting timeout in miliseconds
 *
 * @returns CNN_OK if accelerator completed inference, CNN_FAIL otherwise
 */
int cnn_wait(int timeout);

#endif // KENNING_INFERENCE_LIB_RUNTIMES_AI8X_CNN_H_
