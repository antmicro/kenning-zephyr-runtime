
#include <stdint.h>

#define CNN_OK 0
#define CNN_ERR 1

static int32_t min(int32_t a, int32_t b) { return a < b ? a : b; }

static void memcpy32(uint32_t *dst, uint32_t *src, uint32_t n)
{
    for (uint32_t i = 0; i < n; i++)
    {
        dst[i] = src[i];
    }
}

int cnn_init(void)
{
    *((volatile uint32_t *)0x51000000) = 0x00000008; // Enable clocks
    *((volatile uint32_t *)0x52000000) = 0x00000008; // Enable clocks
    *((volatile uint32_t *)0x53000000) = 0x00000008; // Enable clocks
    *((volatile uint32_t *)0x54000000) = 0x00000008; // Enable clocks
    *((volatile uint32_t *)0x50001000) = 0x00000000; // AON control
    *((volatile uint32_t *)0x51000004) = 0x0000040e; // SRAM control
    *((volatile uint32_t *)0x52000004) = 0x0000040e; // SRAM control
    *((volatile uint32_t *)0x53000004) = 0x0000040e; // SRAM control
    *((volatile uint32_t *)0x54000004) = 0x0000040e; // SRAM control
    *((volatile uint32_t *)0x5100000c) = 0x00001c80; // Clear registers
    *((volatile uint32_t *)0x5200000c) = 0x00001c80; // Clear registers
    *((volatile uint32_t *)0x5300000c) = 0x00001c80; // Clear registers
    *((volatile uint32_t *)0x5400000c) = 0x00001c80; // Clear registers
    while ((*((volatile uint32_t *)0x5100000c) & 0x2000000) != 0x2000000)
        ; // Wait for clear
    while ((*((volatile uint32_t *)0x5200000c) & 0x2000000) != 0x2000000)
        ; // Wait for clear
    while ((*((volatile uint32_t *)0x5300000c) & 0x2000000) != 0x2000000)
        ; // Wait for clear
    while ((*((volatile uint32_t *)0x5400000c) & 0x2000000) != 0x2000000)
        ;                                            // Wait for clear
    *((volatile uint32_t *)0x5100000c) = 0x00000000; // Reset BIST
    *((volatile uint32_t *)0x5200000c) = 0x00000000; // Reset BIST
    *((volatile uint32_t *)0x5300000c) = 0x00000000; // Reset BIST
    *((volatile uint32_t *)0x5400000c) = 0x00000000; // Reset BIST

    *((volatile uint32_t *)0x51000000) = 0x00100008; // Stop SM
    *((volatile uint32_t *)0x51000008) = 0x00000005; // Layer count
    *((volatile uint32_t *)0x52000000) = 0x00100008; // Stop SM
    *((volatile uint32_t *)0x52000008) = 0x00000005; // Layer count
    *((volatile uint32_t *)0x53000000) = 0x00100008; // Stop SM
    *((volatile uint32_t *)0x53000008) = 0x00000005; // Layer count
    *((volatile uint32_t *)0x54000000) = 0x00100008; // Stop SM
    *((volatile uint32_t *)0x54000008) = 0x00000005; // Layer count

    return CNN_OK;
}

int cnn_configure(void)
{
    // Layer 0 quadrant 0
    *((volatile uint32_t *)0x51100004) = 0x00010004; // Rows
    *((volatile uint32_t *)0x51100008) = 0x00010011; // Columns
    *((volatile uint32_t *)0x51100018) = 0x00000010; // Stride
    *((volatile uint32_t *)0x5110001c) = 0x0002a800; // SRAM write ptr
    *((volatile uint32_t *)0x51100024) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x51100030) = 0x00882960; // Layer control
    *((volatile uint32_t *)0x51100034) = 0x0000e000; // Layer control 2
    *((volatile uint32_t *)0x51100038) = 0x00009fee; // Mask count
    *((volatile uint32_t *)0x5110003c) = 0x00009fe0; // Mask offset
    *((volatile uint32_t *)0x51100040) = 0x00000007; // Output channel count
    *((volatile uint32_t *)0x51100044) = 0x0000000f; // TRAM ptr max
    *((volatile uint32_t *)0x5110004c) = 0x0080a000; // Post processing register
    *((volatile uint32_t *)0x51100048) = 0x00010001; // Mask and processor enables

    // Layer 0 quadrant 1
    *((volatile uint32_t *)0x52100004) = 0x00010004; // Rows
    *((volatile uint32_t *)0x52100008) = 0x00010011; // Columns
    *((volatile uint32_t *)0x52100018) = 0x00000010; // Stride
    *((volatile uint32_t *)0x5210001c) = 0x0002a800; // SRAM write ptr
    *((volatile uint32_t *)0x52100024) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x52100030) = 0x00880960; // Layer control
    *((volatile uint32_t *)0x52100034) = 0x0000e000; // Layer control 2
    *((volatile uint32_t *)0x52100038) = 0x00009fee; // Mask count
    *((volatile uint32_t *)0x5210003c) = 0x00009fe0; // Mask offset
    *((volatile uint32_t *)0x52100040) = 0x00000007; // Output channel count
    *((volatile uint32_t *)0x52100044) = 0x0000000f; // TRAM ptr max
    *((volatile uint32_t *)0x5210004c) = 0x0080b000; // Post processing register

    // Layer 0 quadrant 2
    *((volatile uint32_t *)0x53100004) = 0x00010004; // Rows
    *((volatile uint32_t *)0x53100008) = 0x00010011; // Columns
    *((volatile uint32_t *)0x53100018) = 0x00000010; // Stride
    *((volatile uint32_t *)0x5310001c) = 0x0002a800; // SRAM write ptr
    *((volatile uint32_t *)0x53100024) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x53100030) = 0x00880960; // Layer control
    *((volatile uint32_t *)0x53100034) = 0x0000e000; // Layer control 2
    *((volatile uint32_t *)0x53100038) = 0x00009fee; // Mask count
    *((volatile uint32_t *)0x5310003c) = 0x00009fe0; // Mask offset
    *((volatile uint32_t *)0x53100040) = 0x00000007; // Output channel count
    *((volatile uint32_t *)0x53100044) = 0x0000000f; // TRAM ptr max
    *((volatile uint32_t *)0x5310004c) = 0x0080a000; // Post processing register

    // Layer 0 quadrant 3
    *((volatile uint32_t *)0x54100004) = 0x00010004; // Rows
    *((volatile uint32_t *)0x54100008) = 0x00010011; // Columns
    *((volatile uint32_t *)0x54100018) = 0x00000010; // Stride
    *((volatile uint32_t *)0x5410001c) = 0x0002a800; // SRAM write ptr
    *((volatile uint32_t *)0x54100024) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x54100030) = 0x00880960; // Layer control
    *((volatile uint32_t *)0x54100034) = 0x0000e000; // Layer control 2
    *((volatile uint32_t *)0x54100038) = 0x00009fee; // Mask count
    *((volatile uint32_t *)0x5410003c) = 0x00009fe0; // Mask offset
    *((volatile uint32_t *)0x54100040) = 0x00000007; // Output channel count
    *((volatile uint32_t *)0x54100044) = 0x0000000f; // TRAM ptr max
    *((volatile uint32_t *)0x5410004c) = 0x0080a000; // Post processing register

    // Layer 1 quadrant 0
    *((volatile uint32_t *)0x51100104) = 0x00010002; // Rows
    *((volatile uint32_t *)0x51100108) = 0x0001000f; // Columns
    *((volatile uint32_t *)0x51100118) = 0x00000010; // Stride
    *((volatile uint32_t *)0x5110011c) = 0x00008000; // SRAM write ptr
    *((volatile uint32_t *)0x51100124) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x5110012c) = 0x00002800; // SRAM read ptr
    *((volatile uint32_t *)0x51100130) = 0x00882920; // Layer control
    *((volatile uint32_t *)0x51100134) = 0x0001e000; // Layer control 2
    *((volatile uint32_t *)0x51100138) = 0x0000001e; // Mask count
    *((volatile uint32_t *)0x51100140) = 0x0000000f; // Output channel count
    *((volatile uint32_t *)0x51100144) = 0x0000000d; // TRAM ptr max
    *((volatile uint32_t *)0x5110014c) = 0x00809000; // Post processing register

    // Layer 1 quadrant 1
    *((volatile uint32_t *)0x52100104) = 0x00010002; // Rows
    *((volatile uint32_t *)0x52100108) = 0x0001000f; // Columns
    *((volatile uint32_t *)0x52100118) = 0x00000010; // Stride
    *((volatile uint32_t *)0x5210011c) = 0x00008000; // SRAM write ptr
    *((volatile uint32_t *)0x52100124) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x5210012c) = 0x00002800; // SRAM read ptr
    *((volatile uint32_t *)0x52100130) = 0x00880920; // Layer control
    *((volatile uint32_t *)0x52100134) = 0x0001e000; // Layer control 2
    *((volatile uint32_t *)0x52100138) = 0x0000001e; // Mask count
    *((volatile uint32_t *)0x52100140) = 0x0000000f; // Output channel count
    *((volatile uint32_t *)0x52100144) = 0x0000000d; // TRAM ptr max
    *((volatile uint32_t *)0x5210014c) = 0x00808000; // Post processing register
    *((volatile uint32_t *)0x52100148) = 0x0ff00ff0; // Mask and processor enables

    // Layer 1 quadrant 2
    *((volatile uint32_t *)0x53100104) = 0x00010002; // Rows
    *((volatile uint32_t *)0x53100108) = 0x0001000f; // Columns
    *((volatile uint32_t *)0x53100118) = 0x00000010; // Stride
    *((volatile uint32_t *)0x5310011c) = 0x00008000; // SRAM write ptr
    *((volatile uint32_t *)0x53100124) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x5310012c) = 0x00002800; // SRAM read ptr
    *((volatile uint32_t *)0x53100130) = 0x00880920; // Layer control
    *((volatile uint32_t *)0x53100134) = 0x0001e000; // Layer control 2
    *((volatile uint32_t *)0x53100138) = 0x0000001e; // Mask count
    *((volatile uint32_t *)0x53100140) = 0x0000000f; // Output channel count
    *((volatile uint32_t *)0x53100144) = 0x0000000d; // TRAM ptr max
    *((volatile uint32_t *)0x5310014c) = 0x00808000; // Post processing register

    // Layer 1 quadrant 3
    *((volatile uint32_t *)0x54100104) = 0x00010002; // Rows
    *((volatile uint32_t *)0x54100108) = 0x0001000f; // Columns
    *((volatile uint32_t *)0x54100118) = 0x00000010; // Stride
    *((volatile uint32_t *)0x5410011c) = 0x00008000; // SRAM write ptr
    *((volatile uint32_t *)0x54100124) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x5410012c) = 0x00002800; // SRAM read ptr
    *((volatile uint32_t *)0x54100130) = 0x00880920; // Layer control
    *((volatile uint32_t *)0x54100134) = 0x0001e000; // Layer control 2
    *((volatile uint32_t *)0x54100138) = 0x0000001e; // Mask count
    *((volatile uint32_t *)0x54100140) = 0x0000000f; // Output channel count
    *((volatile uint32_t *)0x54100144) = 0x0000000d; // TRAM ptr max
    *((volatile uint32_t *)0x5410014c) = 0x00808000; // Post processing register

    // Layer 2 quadrant 0
    *((volatile uint32_t *)0x51100204) = 0x00010000; // Rows
    *((volatile uint32_t *)0x51100208) = 0x00010000; // Columns
    *((volatile uint32_t *)0x51100218) = 0x00000010; // Stride
    *((volatile uint32_t *)0x5110021c) = 0x0003a800; // SRAM write ptr
    *((volatile uint32_t *)0x51100220) = 0x00000001; // Write ptr time slot offs
    *((volatile uint32_t *)0x51100224) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x51100230) = 0x00002920; // Layer control
    *((volatile uint32_t *)0x51100234) = 0x0001e00d; // Layer control 2
    *((volatile uint32_t *)0x51100238) = 0x000001be; // Mask count
    *((volatile uint32_t *)0x51100240) = 0x000000df; // Output channel count
    *((volatile uint32_t *)0x5110020c) = 0x00000100; // 1D
    *((volatile uint32_t *)0x5110024c) = 0x00804000; // Post processing register
    *((volatile uint32_t *)0x51100248) = 0xfff0fff0; // Mask and processor enables

    // Layer 2 quadrant 1
    *((volatile uint32_t *)0x52100204) = 0x00010000; // Rows
    *((volatile uint32_t *)0x52100208) = 0x00010000; // Columns
    *((volatile uint32_t *)0x52100218) = 0x00000010; // Stride
    *((volatile uint32_t *)0x5210021c) = 0x0003a800; // SRAM write ptr
    *((volatile uint32_t *)0x52100220) = 0x00000001; // Write ptr time slot offs
    *((volatile uint32_t *)0x52100224) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x52100230) = 0x00000920; // Layer control
    *((volatile uint32_t *)0x52100234) = 0x0001e00d; // Layer control 2
    *((volatile uint32_t *)0x52100238) = 0x000001be; // Mask count
    *((volatile uint32_t *)0x52100240) = 0x000000df; // Output channel count
    *((volatile uint32_t *)0x5210020c) = 0x00000100; // 1D
    *((volatile uint32_t *)0x5210024c) = 0x00804000; // Post processing register
    *((volatile uint32_t *)0x52100248) = 0x000f000f; // Mask and processor enables

    // Layer 2 quadrant 2
    *((volatile uint32_t *)0x53100204) = 0x00010000; // Rows
    *((volatile uint32_t *)0x53100208) = 0x00010000; // Columns
    *((volatile uint32_t *)0x53100218) = 0x00000010; // Stride
    *((volatile uint32_t *)0x5310021c) = 0x0003a800; // SRAM write ptr
    *((volatile uint32_t *)0x53100220) = 0x00000001; // Write ptr time slot offs
    *((volatile uint32_t *)0x53100224) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x53100230) = 0x00000920; // Layer control
    *((volatile uint32_t *)0x53100234) = 0x0001e00d; // Layer control 2
    *((volatile uint32_t *)0x53100238) = 0x000001be; // Mask count
    *((volatile uint32_t *)0x53100240) = 0x000000df; // Output channel count
    *((volatile uint32_t *)0x5310020c) = 0x00000100; // 1D
    *((volatile uint32_t *)0x5310024c) = 0x00804000; // Post processing register

    // Layer 2 quadrant 3
    *((volatile uint32_t *)0x54100204) = 0x00010000; // Rows
    *((volatile uint32_t *)0x54100208) = 0x00010000; // Columns
    *((volatile uint32_t *)0x54100218) = 0x00000010; // Stride
    *((volatile uint32_t *)0x5410021c) = 0x0003a800; // SRAM write ptr
    *((volatile uint32_t *)0x54100220) = 0x00000001; // Write ptr time slot offs
    *((volatile uint32_t *)0x54100224) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x54100230) = 0x00000920; // Layer control
    *((volatile uint32_t *)0x54100234) = 0x0001e00d; // Layer control 2
    *((volatile uint32_t *)0x54100238) = 0x000001be; // Mask count
    *((volatile uint32_t *)0x54100240) = 0x000000df; // Output channel count
    *((volatile uint32_t *)0x5410020c) = 0x00000100; // 1D
    *((volatile uint32_t *)0x5410024c) = 0x00804000; // Post processing register

    // Layer 3 quadrant 0
    *((volatile uint32_t *)0x51100304) = 0x00010000; // Rows
    *((volatile uint32_t *)0x51100308) = 0x00010000; // Columns
    *((volatile uint32_t *)0x51100318) = 0x00000010; // Stride
    *((volatile uint32_t *)0x5110031c) = 0x00058000; // SRAM write ptr
    *((volatile uint32_t *)0x51100320) = 0x00000001; // Write ptr time slot offs
    *((volatile uint32_t *)0x51100324) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x5110032c) = 0x00002800; // SRAM read ptr
    *((volatile uint32_t *)0x51100330) = 0x00006920; // Layer control
    *((volatile uint32_t *)0x51100334) = 0x0000e000; // Layer control 2
    *((volatile uint32_t *)0x51100338) = 0x0000000e; // Mask count
    *((volatile uint32_t *)0x51100340) = 0x00000007; // Output channel count
    *((volatile uint32_t *)0x5110030c) = 0x00000100; // 1D
    *((volatile uint32_t *)0x5110034c) = 0x00808000; // Post processing register

    // Layer 3 quadrant 1
    *((volatile uint32_t *)0x52100304) = 0x00010000; // Rows
    *((volatile uint32_t *)0x52100308) = 0x00010000; // Columns
    *((volatile uint32_t *)0x52100318) = 0x00000010; // Stride
    *((volatile uint32_t *)0x5210031c) = 0x00058000; // SRAM write ptr
    *((volatile uint32_t *)0x52100320) = 0x00000001; // Write ptr time slot offs
    *((volatile uint32_t *)0x52100324) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x5210032c) = 0x00002800; // SRAM read ptr
    *((volatile uint32_t *)0x52100330) = 0x00000920; // Layer control
    *((volatile uint32_t *)0x52100334) = 0x0000e000; // Layer control 2
    *((volatile uint32_t *)0x52100338) = 0x0000000e; // Mask count
    *((volatile uint32_t *)0x52100340) = 0x00000007; // Output channel count
    *((volatile uint32_t *)0x5210030c) = 0x00000100; // 1D
    *((volatile uint32_t *)0x5210034c) = 0x00808000; // Post processing register
    *((volatile uint32_t *)0x52100348) = 0xf000f000; // Mask and processor enables

    // Layer 3 quadrant 2
    *((volatile uint32_t *)0x53100304) = 0x00010000; // Rows
    *((volatile uint32_t *)0x53100308) = 0x00010000; // Columns
    *((volatile uint32_t *)0x53100318) = 0x00000010; // Stride
    *((volatile uint32_t *)0x5310031c) = 0x00058000; // SRAM write ptr
    *((volatile uint32_t *)0x53100320) = 0x00000001; // Write ptr time slot offs
    *((volatile uint32_t *)0x53100324) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x5310032c) = 0x00002800; // SRAM read ptr
    *((volatile uint32_t *)0x53100330) = 0x00000920; // Layer control
    *((volatile uint32_t *)0x53100334) = 0x0000e000; // Layer control 2
    *((volatile uint32_t *)0x53100338) = 0x0000000e; // Mask count
    *((volatile uint32_t *)0x53100340) = 0x00000007; // Output channel count
    *((volatile uint32_t *)0x5310030c) = 0x00000100; // 1D
    *((volatile uint32_t *)0x5310034c) = 0x00808000; // Post processing register
    *((volatile uint32_t *)0x53100348) = 0x0fff0fff; // Mask and processor enables

    // Layer 3 quadrant 3
    *((volatile uint32_t *)0x54100304) = 0x00010000; // Rows
    *((volatile uint32_t *)0x54100308) = 0x00010000; // Columns
    *((volatile uint32_t *)0x54100318) = 0x00000010; // Stride
    *((volatile uint32_t *)0x5410031c) = 0x00058000; // SRAM write ptr
    *((volatile uint32_t *)0x54100320) = 0x00000001; // Write ptr time slot offs
    *((volatile uint32_t *)0x54100324) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x5410032c) = 0x00002800; // SRAM read ptr
    *((volatile uint32_t *)0x54100330) = 0x00000920; // Layer control
    *((volatile uint32_t *)0x54100334) = 0x0000e000; // Layer control 2
    *((volatile uint32_t *)0x54100338) = 0x0000000e; // Mask count
    *((volatile uint32_t *)0x54100340) = 0x00000007; // Output channel count
    *((volatile uint32_t *)0x5410030c) = 0x00000100; // 1D
    *((volatile uint32_t *)0x5410034c) = 0x00808000; // Post processing register

    // Layer 4 quadrant 0
    *((volatile uint32_t *)0x51100404) = 0x00010000; // Rows
    *((volatile uint32_t *)0x51100408) = 0x00010000; // Columns
    *((volatile uint32_t *)0x51100418) = 0x00000010; // Stride
    *((volatile uint32_t *)0x5110041c) = 0x00002800; // SRAM write ptr
    *((volatile uint32_t *)0x51100420) = 0x00000001; // Write ptr time slot offs
    *((volatile uint32_t *)0x51100424) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x51100430) = 0x0000c920; // Layer control
    *((volatile uint32_t *)0x51100434) = 0x00006000; // Layer control 2
    *((volatile uint32_t *)0x51100438) = 0x00000006; // Mask count
    *((volatile uint32_t *)0x51100440) = 0x00000003; // Output channel count
    *((volatile uint32_t *)0x5110040c) = 0x00000100; // 1D
    *((volatile uint32_t *)0x5110044c) = 0x00808000; // Post processing register

    // Layer 4 quadrant 1
    *((volatile uint32_t *)0x52100404) = 0x00010000; // Rows
    *((volatile uint32_t *)0x52100408) = 0x00010000; // Columns
    *((volatile uint32_t *)0x52100418) = 0x00000010; // Stride
    *((volatile uint32_t *)0x5210041c) = 0x00002800; // SRAM write ptr
    *((volatile uint32_t *)0x52100420) = 0x00000001; // Write ptr time slot offs
    *((volatile uint32_t *)0x52100424) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x52100430) = 0x00000920; // Layer control
    *((volatile uint32_t *)0x52100434) = 0x00006000; // Layer control 2
    *((volatile uint32_t *)0x52100438) = 0x00000006; // Mask count
    *((volatile uint32_t *)0x52100440) = 0x00000003; // Output channel count
    *((volatile uint32_t *)0x5210040c) = 0x00000100; // 1D
    *((volatile uint32_t *)0x5210044c) = 0x00808000; // Post processing register

    // Layer 4 quadrant 2
    *((volatile uint32_t *)0x53100404) = 0x00010000; // Rows
    *((volatile uint32_t *)0x53100408) = 0x00010000; // Columns
    *((volatile uint32_t *)0x53100418) = 0x00000010; // Stride
    *((volatile uint32_t *)0x5310041c) = 0x00002800; // SRAM write ptr
    *((volatile uint32_t *)0x53100420) = 0x00000001; // Write ptr time slot offs
    *((volatile uint32_t *)0x53100424) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x53100430) = 0x00000920; // Layer control
    *((volatile uint32_t *)0x53100434) = 0x00006000; // Layer control 2
    *((volatile uint32_t *)0x53100438) = 0x00000006; // Mask count
    *((volatile uint32_t *)0x53100440) = 0x00000003; // Output channel count
    *((volatile uint32_t *)0x5310040c) = 0x00000100; // 1D
    *((volatile uint32_t *)0x5310044c) = 0x00808000; // Post processing register
    *((volatile uint32_t *)0x53100448) = 0xf000f000; // Mask and processor enables

    // Layer 4 quadrant 3
    *((volatile uint32_t *)0x54100404) = 0x00010000; // Rows
    *((volatile uint32_t *)0x54100408) = 0x00010000; // Columns
    *((volatile uint32_t *)0x54100418) = 0x00000010; // Stride
    *((volatile uint32_t *)0x5410041c) = 0x00002800; // SRAM write ptr
    *((volatile uint32_t *)0x54100420) = 0x00000001; // Write ptr time slot offs
    *((volatile uint32_t *)0x54100424) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x54100430) = 0x00000920; // Layer control
    *((volatile uint32_t *)0x54100434) = 0x00006000; // Layer control 2
    *((volatile uint32_t *)0x54100438) = 0x00000006; // Mask count
    *((volatile uint32_t *)0x54100440) = 0x00000003; // Output channel count
    *((volatile uint32_t *)0x5410040c) = 0x00000100; // 1D
    *((volatile uint32_t *)0x5410044c) = 0x00808000; // Post processing register
    *((volatile uint32_t *)0x54100448) = 0x000f000f; // Mask and processor enables

    // Layer 5 quadrant 0
    *((volatile uint32_t *)0x51100504) = 0x00010000; // Rows
    *((volatile uint32_t *)0x51100508) = 0x00010000; // Columns
    *((volatile uint32_t *)0x51100518) = 0x00000010; // Stride
    *((volatile uint32_t *)0x51100520) = 0x00000001; // Write ptr time slot offs
    *((volatile uint32_t *)0x51100524) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x5110052c) = 0x00002800; // SRAM read ptr
    *((volatile uint32_t *)0x51100530) = 0x00000920; // Layer control
    *((volatile uint32_t *)0x51100534) = 0x00002000; // Layer control 2
    *((volatile uint32_t *)0x51100538) = 0x00000002; // Mask count
    *((volatile uint32_t *)0x51100540) = 0x00000001; // Output channel count
    *((volatile uint32_t *)0x5110050c) = 0x00000100; // 1D
    *((volatile uint32_t *)0x5110054c) = 0x0080c000; // Post processing register
    *((volatile uint32_t *)0x51100548) = 0x000f000f; // Mask and processor enables

    // Layer 5 quadrant 1
    *((volatile uint32_t *)0x52100504) = 0x00010000; // Rows
    *((volatile uint32_t *)0x52100508) = 0x00010000; // Columns
    *((volatile uint32_t *)0x52100518) = 0x00000010; // Stride
    *((volatile uint32_t *)0x52100520) = 0x00000001; // Write ptr time slot offs
    *((volatile uint32_t *)0x52100524) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x5210052c) = 0x00002800; // SRAM read ptr
    *((volatile uint32_t *)0x52100530) = 0x00000920; // Layer control
    *((volatile uint32_t *)0x52100534) = 0x00002000; // Layer control 2
    *((volatile uint32_t *)0x52100538) = 0x00000002; // Mask count
    *((volatile uint32_t *)0x52100540) = 0x00000001; // Output channel count
    *((volatile uint32_t *)0x5210050c) = 0x00000100; // 1D
    *((volatile uint32_t *)0x5210054c) = 0x0080c000; // Post processing register

    // Layer 5 quadrant 2
    *((volatile uint32_t *)0x53100504) = 0x00010000; // Rows
    *((volatile uint32_t *)0x53100508) = 0x00010000; // Columns
    *((volatile uint32_t *)0x53100518) = 0x00000010; // Stride
    *((volatile uint32_t *)0x53100520) = 0x00000001; // Write ptr time slot offs
    *((volatile uint32_t *)0x53100524) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x5310052c) = 0x00002800; // SRAM read ptr
    *((volatile uint32_t *)0x53100530) = 0x00000920; // Layer control
    *((volatile uint32_t *)0x53100534) = 0x00002000; // Layer control 2
    *((volatile uint32_t *)0x53100538) = 0x00000002; // Mask count
    *((volatile uint32_t *)0x53100540) = 0x00000001; // Output channel count
    *((volatile uint32_t *)0x5310050c) = 0x00000100; // 1D
    *((volatile uint32_t *)0x5310054c) = 0x0080c000; // Post processing register

    // Layer 5 quadrant 3
    *((volatile uint32_t *)0x54100504) = 0x00010000; // Rows
    *((volatile uint32_t *)0x54100508) = 0x00010000; // Columns
    *((volatile uint32_t *)0x54100518) = 0x00000010; // Stride
    *((volatile uint32_t *)0x54100520) = 0x00000001; // Write ptr time slot offs
    *((volatile uint32_t *)0x54100524) = 0x00008000; // Write ptr mask offs
    *((volatile uint32_t *)0x5410052c) = 0x00002800; // SRAM read ptr
    *((volatile uint32_t *)0x54100530) = 0x00000920; // Layer control
    *((volatile uint32_t *)0x54100534) = 0x00002000; // Layer control 2
    *((volatile uint32_t *)0x54100538) = 0x00000002; // Mask count
    *((volatile uint32_t *)0x54100540) = 0x00000001; // Output channel count
    *((volatile uint32_t *)0x5410050c) = 0x00000100; // 1D
    *((volatile uint32_t *)0x5410054c) = 0x0080c000; // Post processing register

    return CNN_OK;
}

int cnn_unload(uint32_t *out_buf32)
{
    uint8_t *out_buf = (uint8_t *)out_buf32;
    uint32_t val;
    volatile uint32_t *addr;

    // Custom unload for this network, layer 5 (layer_6): 8-bit data, shape: (2, 1, 1)
    addr = (volatile uint32_t *)0x51800000;
    val = *addr++;
    *out_buf++ = (val & 0xff);
    *out_buf++ = ((val >> 8) & 0xff);

    return CNN_OK;
}

int cnn_load_input(uint32_t *src, uint32_t off, uint32_t n)
{
    while (n > 0)
    {
        if (off < 23)
        {
            uint32_t to_write = min(23 - off, n);
            memcpy32((uint32_t *)0x51800000 + (off - 0), src, to_write);
            n -= to_write;
            off += to_write;
            continue;
        }
        return -1;
    }
    return 0;
}
