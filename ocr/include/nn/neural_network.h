#ifndef NEURAL_NETWORK_H
#define NEURAL_NETWORK_H

#include <stdio.h>

#include "utils/error.h"
#include "utils/matrix.h"

typedef struct layer {
    unsigned int inputs;
    unsigned int outputs;
    MATRIX *weights;
    MATRIX *bias;
} LAYER;

typedef struct network {
    unsigned int layers_count;
    double (*activation_function)(void *, double);
    double (*activation_function_derivative)(void *, double);
    LAYER *layers;
} NETWORK;

ERROR layer_init(LAYER *layer, unsigned int inputs, unsigned int outputs,
                 double (*weights_function)(void *context, unsigned int,
                                            unsigned int),
                 void *weights_context,
                 double (*bias_function)(void *context, unsigned int,
                                         unsigned int),
                 void *bias_context);
ERROR layer_init_random(LAYER *layer, unsigned int inputs,
                        unsigned int outputs);
void layer_free(LAYER *layer);

int layer_save(FILE *file, LAYER *layer);

ERROR layer_load(FILE *file, LAYER *layer);

ERROR network_init(NETWORK *network, LAYER *layers, unsigned int layers_count,
                   double (*activation_function)(void *, double),
                   double (*activation_function_derivative)(void *, double));
void network_free(NETWORK *network);

int network_save(FILE *file, NETWORK *network);

ERROR network_load(FILE *file, NETWORK *network);

ERROR network_feedforward(NETWORK *network, void *context, MATRIX *input,
                          MATRIX *output);

#endif
