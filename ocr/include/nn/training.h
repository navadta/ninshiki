#ifndef H_TRAINING
#define H_TRAINING

#include "nn/neural_network.h"
#include "utils/error.h"
#include "utils/matrix.h"

ERROR network_train(NETWORK *network, void *context, void *derivative_context,
                    unsigned int epochs, double learning_rate,
                    double regularization_rate, unsigned int batch_size,
                    unsigned int inputs_count, MATRIX *inputs,
                    MATRIX *expected);

#endif