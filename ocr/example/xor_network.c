#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "nn/neural_network.h"
#include "nn/training.h"
#include "utils/error.h"
#include "utils/matrix.h"
#include "utils/utils.h"

ERROR network_create(NETWORK *network,
                     double (*activation_function)(void *, double),
                     double (*activation_function_derivative)(void *, double)) {
    LAYER *layers = malloc(4 * sizeof(LAYER));

    layer_init_random(layers, 2, 2);
    layer_init_random(layers + 1, 2, 2);
    layer_init_random(layers + 2, 2, 2);
    layer_init_random(layers + 3, 2, 1);

    network_init(network, layers, 4, activation_function,
                 activation_function_derivative);
    return SUCCESS;
}
double elu(void *context, double value) {
    double alpha = *((double *) context);
    return value >= 0 ? value : alpha * (exp(value) - 1);
}

double elu_prime(void *context, double value) {
    double alpha = *((double *) context);
    return value > 0 ? 1 : alpha * exp(value);
}

double sigmoid(void *context, double value) {
    (void) context;
    return 1.0 / (1.0 + exp(-value));
}

double sigmoid_prime(void *context, double value) {
    (void) context;
    return sigmoid(context, value) * (1 - sigmoid(context, value));
}

ERROR program(unsigned long activation) {
    ERROR err = SUCCESS;

    printf("Activation Function: %s\n",
           activation == 1 ? "Sigmoid\n" : "ELU (Exponential Linear Unit)\n");
    printf("Initializing random...\n\n");
    srand(time(NULL));

    NETWORK network;
    err_throw(err, network_create(&network, activation == 1 ? sigmoid : elu,
                                  activation == 1 ? sigmoid_prime : elu_prime));

    MATRIX *inputs = malloc(4 * sizeof(MATRIX));
    matrix_init(inputs, 2, 1, NULL, NULL);
    matrix_init(inputs + 1, 2, 1, NULL, NULL);
    matrix_init(inputs + 2, 2, 1, NULL, NULL);
    matrix_init(inputs + 3, 2, 1, NULL, NULL);
    matrix_set(&inputs[0], 0, 0, 0.0);
    matrix_set(&inputs[0], 1, 0, 0.0);
    matrix_set(&inputs[1], 0, 0, 0.0);
    matrix_set(&inputs[1], 1, 0, 1.0);
    matrix_set(&inputs[2], 0, 0, 1.0);
    matrix_set(&inputs[2], 1, 0, 0.0);
    matrix_set(&inputs[3], 0, 0, 1.0);
    matrix_set(&inputs[3], 1, 0, 1.0);

    MATRIX *expected = malloc(4 * sizeof(MATRIX));
    matrix_init(expected + 0, 1, 1, NULL, NULL);
    matrix_init(expected + 1, 1, 1, NULL, NULL);
    matrix_init(expected + 2, 1, 1, NULL, NULL);
    matrix_init(expected + 3, 1, 1, NULL, NULL);
    matrix_set(&expected[0], 0, 0, 0.0);
    matrix_set(&expected[1], 0, 0, 1.0);
    matrix_set(&expected[2], 0, 0, 1.0);
    matrix_set(&expected[3], 0, 0, 0.0);

    printf("Training...\n\n");
    double alpha = 1.0;
    err_throw(err, network_train(&network, (void *) &alpha, (void *) &alpha,
                                 5000, 0.5, .0, 32, 4, inputs, expected));

    MATRIX *output = malloc(sizeof(MATRIX));
    for (unsigned int j = 0; j < 4; j++) {
        network_feedforward(&network, (void *) &alpha, inputs + j, output);
        unsigned int result = matrix_get(output, 0, 0) >= 0.5 ? 1 : 0;
        printf("INPUT: [%f, %f]\nEXPECTED: %u\nOUTPUT: %u (real %f)\n\n",
               matrix_get(inputs + j, 0, 0), matrix_get(inputs + j, 1, 0),
               (unsigned int) matrix_get(expected + j, 0, 0), result,
               matrix_get(output, 0, 0));
    }

    FILE *before = fopen("network.txt", "a");
    network_save(before, &network);
    fclose(before);

    free(output);

    for (unsigned int i = 0; i < 4; i++) {
        matrix_free(&inputs[i]);
        matrix_free(&expected[i]);
    }

    free(inputs);
    free(expected);

    network_free(&network);

    return err;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf(
            "You need to precise an activation function:\n - 1 for Sigmoid\n - "
            "2 for ELU (Exponential Linear Unit)\n");
        return 1;
    }

    unsigned long param = strtoul(argv[1], NULL, 10);

    if (param != 1 && param != 2) {
        printf(
            "You need to precise an activation function:\n - 1 for Sigmoid\n - "
            "2 for ELU (Exponential Linear Unit)\n");
        return 1;
    }

    ERROR err = program(param);
    printf("%s\n", format_last_error(err));

    return err;
}