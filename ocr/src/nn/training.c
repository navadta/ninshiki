#include "nn/training.h"

#include <stdio.h>
#include <stdlib.h>

#include "nn/neural_network.h"
#include "utils/error.h"
#include "utils/matrix.h"
#include "utils/utils.h"

typedef struct training_layer {
    unsigned int index;
    MATRIX *input;
    MATRIX *output;
    MATRIX *delta;
} TRAINING_LAYER;

typedef struct training_network {
    NETWORK *network;
    TRAINING_LAYER *layers;
} TRAINING_NETWORK;

typedef struct modifications {
    MATRIX *weights;
    MATRIX *bias;
} MODIFICATIONS;

LAYER *layer(TRAINING_NETWORK *training_network,
             TRAINING_LAYER *training_layer) {
    return training_network->network->layers + training_layer->index;
}

void matrix_full_free(MATRIX *matrix) {
    matrix_free(matrix);
    free(matrix);
}

ERROR layer_feedforward_train(TRAINING_NETWORK *network, void *context,
                              TRAINING_LAYER *previous,
                              TRAINING_LAYER *current) {
    ERROR err = SUCCESS;

    // Free output if it was previously allocated then clone the output of the
    // previous layer
    FREE(current->output, matrix_full_free);
    current->output = malloc(sizeof(MATRIX));
    err_throw(err, matrix_clone(current->output, previous->output));

    // Apply our weights and biases
    err_throw(err,
              matrix_mul(layer(network, current)->weights, current->output));
    err_throw(err, matrix_add(layer(network, current)->bias, current->output));

    // Clone it as our input (needed for backpropagation)
    FREE(current->input, matrix_full_free);
    current->input = malloc(sizeof(MATRIX));
    err_throw(err, matrix_clone(current->input, current->output));

    // Apply our activation function so it really becomes our output
    if (network->network->activation_function != NULL)
        matrix_apply(current->output, network->network->activation_function,
                     context);

    return err;
}

ERROR network_feedforward_train(TRAINING_NETWORK *network, void *context,
                                MATRIX *input) {
    ERROR err = SUCCESS;

    // Get the input layer of our network
    TRAINING_LAYER *input_layer = network->layers;

    // Free the layer input if it was previously allocated then clone our input
    FREE(input_layer->input, matrix_full_free);
    input_layer->input = malloc(sizeof(MATRIX));
    err_throw(err, matrix_clone(input_layer->input, input));

    // Free the layer output if it was previously allocated then clone our input
    FREE(input_layer->output, matrix_full_free);
    input_layer->output = malloc(sizeof(MATRIX));
    err_throw(err, matrix_clone(input_layer->output, input));

    // Feed forward each of our layers except the input layer
    for (unsigned int i = 1; i < network->network->layers_count; i++) {
        err_throw(err, layer_feedforward_train(network, context,
                                               network->layers + i - 1,
                                               network->layers + i));
    }

    return err;
}

ERROR layer_backpropagation(TRAINING_NETWORK *network, void *context,
                            TRAINING_LAYER *previous, TRAINING_LAYER *current) {
    ERROR err = SUCCESS;

    // Allocate our transpose matrix
    MATRIX *transpose = malloc(sizeof(MATRIX));

    // Calculate the transpose of the weights
    err_throw(err, matrix_clone(transpose, layer(network, current)->weights));
    err_throw(err, matrix_transpose(transpose));

    // Multiply the transpose by the delta matrix and store the result in the
    // transpose matrix
    err_throw(err, matrix_mul_2(transpose, current->delta));

    // Don't mind this, some black magic's happening here
    MATRIX *in = malloc(sizeof(MATRIX));
    err_throw(err, matrix_clone(in, previous->input));
    matrix_apply(in, network->network->activation_function_derivative, context);
    err_throw(err, matrix_hadamard_mul(transpose, in));

    // Free the previous delta matrix if it was allocated then set it to our
    // newly computed matrix
    FREE(previous->delta, matrix_full_free);
    previous->delta = in;

    // Free the transpose matrix
    matrix_free(transpose);
    free(transpose);

    return err;
}

ERROR network_backpropagation(TRAINING_NETWORK *network, void *context) {
    ERROR err = SUCCESS;

    // Run the backpropagation algorithm on all the layers
    for (unsigned int i = network->network->layers_count - 1; i > 0; i--) {
        err_throw(err, layer_backpropagation(network, context,
                                             network->layers + i - 1,
                                             network->layers + i));
    }

    return err;
}

ERROR network_gradient(TRAINING_NETWORK *network,
                       MODIFICATIONS *modifications) {
    ERROR err = SUCCESS;

    // Calculate the gradient for each layer
    for (unsigned int i = network->network->layers_count - 1; i > 0; i--) {
        // Get the current and the previous layer
        TRAINING_LAYER *current = network->layers + i;
        TRAINING_LAYER *previous = network->layers + i - 1;

        // Clone the output of the previous layer
        MATRIX *output = malloc(sizeof(MATRIX));
        err_throw(err, matrix_clone(output, previous->output));  // layer->delta
        // Transpose it
        err_throw(err, matrix_transpose(output));
        // Multiply the delta of this layer with the transpose of the previous
        // layer output
        err_throw(err, matrix_mul(current->delta, output));

        // Store the weights and biases modifications
        (modifications + i - 1)->weights = output;
        (modifications + i - 1)->bias = malloc(sizeof(MATRIX));
        err_throw(err,
                  matrix_clone((modifications + i - 1)->bias, current->delta));
    }

    return err;
}

ERROR network_train_epoch(TRAINING_NETWORK *network, void *context,
                          void *derivative_context, double learning_rate,
                          double regularization_rate, unsigned int batch_size,
                          unsigned int inputs_count, MATRIX *inputs,
                          MATRIX *expected) {
    ERROR err = SUCCESS;

    // Allocate our modifications
    MODIFICATIONS *global_modifications =
        malloc((network->network->layers_count - 1) * sizeof(MODIFICATIONS));

    // Initialize the modifications
    for (unsigned int i = network->network->layers_count - 1; i > 0; i--) {
        // Get the layer
        LAYER *layer = network->network->layers + i;

        // Allocate and initialize the weights matrix for this layer
        global_modifications[i - 1].weights = malloc(sizeof(MATRIX));
        err_throw(err, matrix_init(global_modifications[i - 1].weights,
                                   layer->outputs, layer->inputs,
                                   matrix_init_zeros, NULL));

        // Allocate and initialize the bias matrix for this layer
        global_modifications[i - 1].bias = malloc(sizeof(MATRIX));
        err_throw(err, matrix_init(global_modifications[i - 1].bias,
                                   layer->outputs, 1, matrix_init_zeros, NULL));
    }

    // Loop on all the inputs
    for (unsigned int i = 0; i < batch_size; i++) {
        unsigned int input_index = rand() % inputs_count;

        // Feedforward our training network
        err_throw(err, network_feedforward_train(network, context,
                                                 inputs + input_index));

        // Allocate the error matrix then compute it with our output later and
        // our expected outputs
        MATRIX *error = malloc(sizeof(MATRIX));
        err_throw(err, matrix_clone(error, expected + input_index));
        TRAINING_LAYER *output_layer =
            (network->layers + network->network->layers_count - 1);
        err_throw(err, matrix_sub(output_layer->output, error));
        // Free the delta of our output layer if it was previously allocated
        // then set it to the computed error matrix
        FREE(output_layer->delta, matrix_full_free);
        output_layer->delta = error;

        // Apply the backpropagation to the network
        err_throw(err, network_backpropagation(network, derivative_context));
        MODIFICATIONS *modifications = malloc(
            (network->network->layers_count - 1) * sizeof(MODIFICATIONS));
        // Then calculate the gradient
        err_throw(err, network_gradient(network, modifications));

        // Sum the result with the global modification matrices
        for (unsigned int j = network->network->layers_count - 1; j > 0; j--) {
            err_throw(err, matrix_add(modifications[j - 1].weights,
                                      global_modifications[j - 1].weights));
            err_throw(err, matrix_add(modifications[j - 1].bias,
                                      global_modifications[j - 1].bias));

            FREE(modifications[j - 1].weights, matrix_full_free);
            FREE(modifications[j - 1].bias, matrix_full_free);
        }
        FREE(modifications, free);
    }

    // Compute modification rates
    float weight_modif_rate =
        (learning_rate / batch_size) *
        (1 - learning_rate * (regularization_rate / inputs_count));
    float bias_modif_rate = learning_rate / batch_size;

    // Apply the modifications of each layer
    for (unsigned int i = network->network->layers_count - 1; i > 0; i--) {
        LAYER *layer = network->network->layers + i;
        err_throw(err, matrix_scalar(global_modifications[i - 1].weights,
                                     -weight_modif_rate));
        err_throw(err, matrix_add(global_modifications[i - 1].weights,
                                  layer->weights));
        err_throw(err, matrix_scalar(global_modifications[i - 1].bias,
                                     -bias_modif_rate));
        err_throw(err,
                  matrix_add(global_modifications[i - 1].bias, layer->bias));

        FREE(global_modifications[i - 1].weights, matrix_full_free);
        FREE(global_modifications[i - 1].bias, matrix_full_free);
    }
    FREE(global_modifications, free);

    return err;
}

ERROR network_train(NETWORK *network, void *context, void *derivative_context,
                    unsigned int epochs, double learning_rate,
                    double regularization_rate, unsigned int batch_size,
                    unsigned int inputs_count, MATRIX *inputs,
                    MATRIX *expected) {
    ERROR err = SUCCESS;

    // Allocate our training network
    TRAINING_NETWORK *training_network = malloc(sizeof(TRAINING_NETWORK));

    // Initialize our training network
    training_network->network = network;
    training_network->layers =
        calloc(network->layers_count, sizeof(TRAINING_LAYER));
    for (unsigned int i = 0; i < network->layers_count; i++)
        (training_network->layers + i)->index = i;

    // Loop for each of our epochs
    for (unsigned int epoch = 0; epoch < epochs; epoch++) {
        err_throw(err, network_train_epoch(training_network, context,
                                           derivative_context, learning_rate,
                                           regularization_rate, batch_size,
                                           inputs_count, inputs, expected));
    }

    // Free our training layers
    for (unsigned int i = 0; i < network->layers_count; i++) {
        TRAINING_LAYER *layer = training_network->layers + i;
        FREE(layer->input, matrix_full_free);
        FREE(layer->output, matrix_full_free);
        FREE(layer->delta, matrix_full_free);
    }

    // Free our network
    FREE(training_network->layers, free);
    FREE(training_network, free);

    return err;
}