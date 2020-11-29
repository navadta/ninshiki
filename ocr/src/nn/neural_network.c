#include "nn/neural_network.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils/error.h"
#include "utils/matrix.h"
#include "utils/random.h"
#include "utils/utils.h"

ERROR layer_init(LAYER *layer, unsigned int inputs, unsigned int outputs,
                 double (*weights_function)(void *context, unsigned int,
                                            unsigned int),
                 void *weights_context,
                 double (*bias_function)(void *context, unsigned int,
                                         unsigned int),
                 void *bias_context) {
    layer->inputs = inputs;
    layer->outputs = outputs;
    layer->weights = malloc(sizeof(MATRIX));
    ERROR err = matrix_init(layer->weights, outputs, inputs, weights_function,
                            weights_context);
    if (err) return err;
    layer->bias = malloc(sizeof(MATRIX));
    err = matrix_init(layer->bias, outputs, 1, bias_function, bias_context);
    if (err) return err;
    return SUCCESS;
}

double layer_random_init(void *context, unsigned int row, unsigned int column) {
    (void) row;
    (void) column;
    return normal_distribution(0.0, 1.0) / *((double *) context);
}

ERROR layer_init_random(LAYER *layer, unsigned int inputs,
                        unsigned int outputs) {
    double w_ctx = sqrt(inputs);
    double b_ctx = 1.0;
    return layer_init(layer, inputs, outputs, layer_random_init,
                      (void *) &w_ctx, layer_random_init, (void *) &b_ctx);
}

void layer_free(LAYER *layer) {
    matrix_free(layer->weights);
    free(layer->weights);
    layer->weights = NULL;
    matrix_free(layer->bias);
    free(layer->bias);
    layer->bias = NULL;
}

//  Save the layer in filename as:
//      Matrix of weights
//      Matrix of bias
int layer_save(FILE *file, LAYER *layer) {
    int err_weights = matrix_save(file, layer->weights);
    matrix_save(file, layer->bias);
    return err_weights;
}

ERROR network_init(NETWORK *network, LAYER *layers, unsigned int layers_count,
                   double (*activation_function)(void *, double),
                   double (*activation_function_derivative)(void *, double)) {
    network->layers_count = layers_count;
    network->activation_function = activation_function;
    network->activation_function_derivative = activation_function_derivative;
    network->layers = layers;
    return SUCCESS;
}

void network_free(NETWORK *network) {
    for (unsigned int i = 0; i < network->layers_count; i++)
        layer_free(network->layers + i);
}

int network_save(FILE *file, NETWORK *network) {
    if (file == NULL) return 1;
    char str[10];
    sprintf(str, "%u\n", network->layers_count);
    fputs(str, file);
    for (unsigned int i = 0; i < network->layers_count; i++) {
        layer_save(file, network->layers + i);
    }
    return 0;
}

// Load a layer from a file
ERROR layer_load(FILE *file, LAYER *layer) {
    MATRIX *weights = malloc(sizeof(MATRIX));
    MATRIX *bias = malloc(sizeof(MATRIX));

    ERROR err = matrix_load(file, weights);
    if (err) return err;

    err = matrix_load(file, bias);
    if (err) return err;

    layer->inputs = weights->columns;
    layer->outputs = weights->rows;
    layer->weights = weights;
    layer->bias = bias;
    return SUCCESS;
}
// load network witout activation function
ERROR network_load(FILE *file, NETWORK *network) {
    unsigned int layer_count = 0;
    if (fscanf(file, "%u", &layer_count) == 0) return IO_ERROR;

    LAYER *layers = malloc(layer_count * sizeof(LAYER));
    for (unsigned int i = 0; i < layer_count; i++) {
        ERROR err = layer_load(file, layers + i);
        if (err) return err;
    }

    return network_init(network, layers, layer_count, NULL, NULL);
}

ERROR layer_feedforward(NETWORK *network, void *context, MATRIX *input,
                        LAYER *current) {
    ERROR err = SUCCESS;

    // Apply our weights and biases
    err_throw(err, matrix_mul(current->weights, input));
    err_throw(err, matrix_add(current->bias, input));

    // Apply our activation function so it really becomes our output
    if (network->activation_function != NULL)
        matrix_apply(input, network->activation_function, context);

    return err;
}

ERROR network_feedforward(NETWORK *network, void *context, MATRIX *input,
                          MATRIX *output) {
    ERROR err = SUCCESS;

    // Allocate our output
    matrix_clone(output, input);

    // Feed forward each of our layers except the input layer
    for (unsigned int i = 1; i < network->layers_count; i++) {
        err_throw(err, layer_feedforward(network, context, output,
                                         network->layers + i));
    }

    return err;
}
