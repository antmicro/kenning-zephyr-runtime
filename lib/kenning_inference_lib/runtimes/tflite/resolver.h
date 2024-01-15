#ifndef KENNING_INFERENCE_LIB_RUNTIMES_RESOLVER_H
#define KENNING_INFERENCE_LIB_RUNTIMES_RESOLVER_H

#include <tensorflow/lite/micro/micro_mutable_op_resolver.h>

#define TFLITE_RESOLVER_SIZE 128

void tflite_initialize_resolver();

#endif