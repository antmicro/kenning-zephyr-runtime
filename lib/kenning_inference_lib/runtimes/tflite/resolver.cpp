/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <tensorflow/lite/micro/micro_mutable_op_resolver.h>

#include "resolver.h"

tflite::MicroMutableOpResolver<TFLITE_RESOLVER_SIZE> g_tflite_resolver;

// The ops below are meant to be uncommented depending on the executed model
// TODO (glatosinski): Autogenerate the tflite_initialize_resolver based on
// the TFLite model used

void tflite_initialize_resolver()
{
    // g_tflite_resolver.AddAbs();
    // g_tflite_resolver.AddAdd();
    // g_tflite_resolver.AddAddN();
    // g_tflite_resolver.AddArgMax();
    // g_tflite_resolver.AddArgMin();
    // g_tflite_resolver.AddAssignVariable();
    // g_tflite_resolver.AddAveragePool2D();
    // g_tflite_resolver.AddBatchToSpaceNd();
    // g_tflite_resolver.AddBroadcastArgs();
    // g_tflite_resolver.AddBroadcastTo();
    // g_tflite_resolver.AddCallOnce();
    // g_tflite_resolver.AddCast();
    // g_tflite_resolver.AddCeil();
    // g_tflite_resolver.AddCircularBuffer();
    // g_tflite_resolver.AddConcatenation();
    g_tflite_resolver.AddConv2D();
    // g_tflite_resolver.AddCos();
    // g_tflite_resolver.AddCumSum();
    // g_tflite_resolver.AddDepthToSpace();
    // g_tflite_resolver.AddDepthwiseConv2D();
    // g_tflite_resolver.AddDequantize();
    // g_tflite_resolver.AddDetectionPostprocess();
    // g_tflite_resolver.AddDiv();
    // g_tflite_resolver.AddElu();
    // g_tflite_resolver.AddEqual();
    // g_tflite_resolver.AddEthosU();
    // g_tflite_resolver.AddExp();
    // g_tflite_resolver.AddExpandDims();
    // g_tflite_resolver.AddFill();
    // g_tflite_resolver.AddFloor();
    // g_tflite_resolver.AddFloorDiv();
    // g_tflite_resolver.AddFloorMod();
    g_tflite_resolver.AddFullyConnected();
    // g_tflite_resolver.AddGather();
    // g_tflite_resolver.AddGatherNd();
    // g_tflite_resolver.AddGreater();
    // g_tflite_resolver.AddGreaterEqual();
    // g_tflite_resolver.AddHardSwish();
    // g_tflite_resolver.AddIf();
    // g_tflite_resolver.AddL2Normalization();
    // g_tflite_resolver.AddL2Pool2D();
    // g_tflite_resolver.AddLeakyRelu();
    // g_tflite_resolver.AddLess();
    // g_tflite_resolver.AddLessEqual();
    // g_tflite_resolver.AddLog();
    // g_tflite_resolver.AddLogSoftmax();
    // g_tflite_resolver.AddLogicalAnd();
    // g_tflite_resolver.AddLogicalNot();
    // g_tflite_resolver.AddLogicalOr();
    // g_tflite_resolver.AddLogistic();
    g_tflite_resolver.AddMaxPool2D();
    // g_tflite_resolver.AddMaximum();
    // g_tflite_resolver.AddMean();
    // g_tflite_resolver.AddMinimum();
    // g_tflite_resolver.AddMirrorPad();
    // g_tflite_resolver.AddMul();
    // g_tflite_resolver.AddNeg();
    // g_tflite_resolver.AddNotEqual();
    // g_tflite_resolver.AddPack();
    // g_tflite_resolver.AddPad();
    // g_tflite_resolver.AddPadV2();
    // g_tflite_resolver.AddPrelu();
    // g_tflite_resolver.AddQuantize();
    // g_tflite_resolver.AddReadVariable();
    // g_tflite_resolver.AddReduceMax();
    // g_tflite_resolver.AddRelu();
    // g_tflite_resolver.AddRelu6();
    g_tflite_resolver.AddReshape();
    // g_tflite_resolver.AddResizeBilinear();
    // g_tflite_resolver.AddResizeNearestNeighbor();
    // g_tflite_resolver.AddRfft();
    // g_tflite_resolver.AddRound();
    // g_tflite_resolver.AddRsqrt();
    // g_tflite_resolver.AddSelectV2();
    // g_tflite_resolver.AddShape();
    // g_tflite_resolver.AddSin();
    // g_tflite_resolver.AddSlice();
    g_tflite_resolver.AddSoftmax();
    // g_tflite_resolver.AddSpaceToBatchNd();
    // g_tflite_resolver.AddSpaceToDepth();
    // g_tflite_resolver.AddSplit();
    // g_tflite_resolver.AddSplitV();
    // g_tflite_resolver.AddSqrt();
    // g_tflite_resolver.AddSquare();
    // g_tflite_resolver.AddSquaredDifference();
    // g_tflite_resolver.AddSqueeze();
    // g_tflite_resolver.AddStridedSlice();
    // g_tflite_resolver.AddSub();
    // g_tflite_resolver.AddSum();
    // g_tflite_resolver.AddSvdf();
    // g_tflite_resolver.AddTanh();
    // g_tflite_resolver.AddTranspose();
    // g_tflite_resolver.AddTransposeConv();
    // g_tflite_resolver.AddUnidirectionalSequenceLSTM();
    // g_tflite_resolver.AddUnpack();
    // g_tflite_resolver.AddVarHandle();
    // g_tflite_resolver.AddWhile();
    // g_tflite_resolver.AddWindow();
    // g_tflite_resolver.AddZerosLike();
}
