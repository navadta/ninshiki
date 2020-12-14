#include <gtk/gtk.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "images/colors.h"
#include "images/conversions.h"
#include "images/image.h"
#include "images/transformations.h"
#include "nn/neural_network.h"
#include "nn/training.h"
#include "utils/error.h"
#include "utils/matrix.h"
#include "utils/utils.h"

ERROR image_load(IMAGE **image, char *path) {
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, NULL);
    if (pixbuf == NULL) return IO_ERROR;
    *image = malloc(sizeof(IMAGE));
    if (*image == NULL) return ALLOCATION_FAILED;
    (*image)->width = (unsigned int) gdk_pixbuf_get_width(pixbuf);
    (*image)->height = (unsigned int) gdk_pixbuf_get_height(pixbuf);
    (*image)->type = COLOR_RGB;
    (*image)->pixels.rgb =
        malloc((*image)->width * (*image)->height * sizeof(RGB));
    if ((*image)->pixels.rgb == NULL) {
        free(*image);
        return ALLOCATION_FAILED;
    }
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int has_alpha = gdk_pixbuf_get_has_alpha(pixbuf);
    const unsigned char *pixels = gdk_pixbuf_read_pixels(pixbuf);

    int offset = 0;
    for (unsigned int y = 0; y < (*image)->height; y++) {
        for (unsigned int x = 0; x < (*image)->width; x++) {
            int index = y * (*image)->width + x;
            RGB *pixel = (*image)->pixels.rgb + index;
            pixel->red = *(pixels + offset++);
            pixel->green = *(pixels + offset++);
            pixel->blue = *(pixels + offset++);
            if (has_alpha) offset++;
        }
        offset += rowstride - (*image)->width * (has_alpha ? 4 : 3);
    }
    g_object_unref(G_OBJECT(pixbuf));
    return SUCCESS;
}

ERROR network_create(NETWORK *network,
                     double (*activation_function)(void *, double),
                     double (*activation_function_derivative)(void *, double)) {
    LAYER *layers = malloc(3 * sizeof(LAYER));

    layer_init_random(layers, 32 * 32, 32 * 32);
    layer_init_random(layers + 1, 32 * 32, 60);
    layer_init_random(layers + 2, 60, CHARSET_LENGTH);

    network_init(network, layers, 3, activation_function,
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

ERROR program(const char *path, unsigned long activation) {
    ERROR err = SUCCESS;

    printf("Activation Function: %s\n",
           activation == 1 ? "Sigmoid\n" : "ELU (Exponential Linear Unit)\n");
    printf("Initializing random...\n\n");
    srand(time(NULL));

    NETWORK network;
    err_throw(err, network_create(&network, activation == 1 ? sigmoid : elu,
                                  activation == 1 ? sigmoid_prime : elu_prime));

    char *fonts[] = {"arial.png", "nunito.png", "roboto.png"};
    unsigned int samples = 3;

    MATRIX *inputs = malloc(CHARSET_LENGTH * samples * sizeof(MATRIX));
    MATRIX *expected = malloc(CHARSET_LENGTH * samples * sizeof(MATRIX));
    printf("Loading dataset...\n\n");
    for (unsigned int i = 0; i < CHARSET_LENGTH; i++) {
        char character = CHARSET[i];
        unsigned char position = char_index(character);
        for (unsigned int j = 0; j < samples; j++) {
            unsigned int index = i * samples + j;
            char *font = fonts[j];

            char p[1024];
            snprintf(p, 1024, "%s/%s_%d.bmp", path, font, position);

            IMAGE *image;

            err_throw_msg(err, image_load(&image, p), "Couldn't load image");
            err_throw(err, image_scale(image, 32, 32));
            float threshold = 0.5f;
            err_throw(err, image_to_binary(image, basic_threshold,
                                           (void *) &threshold));
            err_throw(err, image_to_matrix(image, inputs + index));
            err_throw(err, matrix_flatten_column(inputs + index));

            image_free(image);

            err_throw(err, char_to_matrix(character, expected + index));
        }
        printf("Loaded %i/%i\n", i + 1, CHARSET_LENGTH);
    }

    printf("Training...\n\n");
    double alpha = 0.1;
    err_throw(err, network_train(&network, (void *) &alpha, (void *) &alpha,
                                 10000, 0.01, 0.0, 50, CHARSET_LENGTH * samples,
                                 inputs, expected));

    MATRIX *output = malloc(sizeof(MATRIX));
    for (unsigned int j = samples * 4 + 43; j < samples * 4 + 47; j++) {
        if (j > 0) matrix_free(output);
        network_feedforward(&network, (void *) &alpha, inputs + j, output);

        unsigned int erow = 0;
        unsigned int ecolumn = 0;
        matrix_max(expected + j, &erow, &ecolumn);

        unsigned int row = 0;
        unsigned int column = 0;
        matrix_max(output, &row, &column);

        printf(
            "EXPECTED: %f (coords [%u, %u])\nOUTPUT: %f (coords "
            "[%u, %u])\n\n",
            matrix_get(expected + j, erow, ecolumn), erow, ecolumn,
            matrix_get(output, row, column), row, column);
    }

    FILE *before = fopen("network.txt", "a");
    network_save(before, &network);
    fclose(before);

    matrix_free(output);
    free(output);

    for (unsigned int i = 0; i < CHARSET_LENGTH * samples; i++) {
        matrix_free(&inputs[i]);
        matrix_free(&expected[i]);
    }

    free(inputs);
    free(expected);

    network_free(&network);
    free(network.layers);

    return err;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf(
            "You need to precise an activation function:\n - 1 for Sigmoid\n - "
            "2 for ELU (Exponential Linear Unit)\nAnd the dataset path for "
            "training\n");
        return 1;
    }

    unsigned long param = strtoul(argv[1], NULL, 10);

    if (param != 1 && param != 2) {
        printf(
            "You need to precise an activation function:\n - 1 for Sigmoid\n - "
            "2 for ELU (Exponential Linear Unit)\n");
        return 1;
    }

    ERROR err = program(argv[2], param);
    printf("%s\n", format_last_error(err));

    return err;
}
