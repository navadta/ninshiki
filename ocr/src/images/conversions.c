#include "images/conversions.h"

#include <stdlib.h>

#include "images/colors.h"
#include "images/image.h"
#include "utils/error.h"
#include "utils/matrix.h"
#include "utils/utils.h"

ERROR image_to_grayscale(IMAGE *image) {
    if (image->type == COLOR_GRAYSCALE) return SUCCESS;
    GRAYSCALE *pixels =
        malloc(image->width * image->height * sizeof(GRAYSCALE));
    for (unsigned int i = 0; i < image->width * image->height; i++) {
        float grayscale;
        switch (image->type) {
            case COLOR_RGB:
                grayscale = (0.21f * image->pixels.rgb[i].red +
                             0.72f * image->pixels.rgb[i].green +
                             0.07f * image->pixels.rgb[i].blue) /
                            255.f;
                break;
            case COLOR_RGBA:
                grayscale = (0.21f * image->pixels.rgba[i].red +
                             0.72f * image->pixels.rgba[i].green +
                             0.07f * image->pixels.rgba[i].blue) *
                            image->pixels.rgba[i].alpha / 255.f;
                break;
            case COLOR_BINARY:
                grayscale = image->pixels.binary[i].binary;
                break;
            default:
                grayscale = 0.f;
                break;
        }
        pixels[i].grayscale = grayscale < 0.f   ? 0.f
                              : grayscale > 1.f ? 1.f
                                                : grayscale;
    }

    switch (image->type) {
        case COLOR_RGB:
            free(image->pixels.rgb);
            break;
        case COLOR_RGBA:
            free(image->pixels.rgba);
            break;
        case COLOR_BINARY:
            free(image->pixels.binary);
            break;
        default:
            break;
    }

    image->type = COLOR_GRAYSCALE;
    image->pixels.grayscale = pixels;

    return SUCCESS;
}

ERROR image_to_rgb(IMAGE *image) {
    if (image->type == COLOR_RGB) return SUCCESS;
    RGB *pixels = malloc(image->width * image->height * sizeof(RGB));
    for (unsigned int i = 0; i < image->width * image->height; i++) {
        unsigned char red = 0;
        unsigned char green = 0;
        unsigned char blue = 0;
        switch (image->type) {
            case COLOR_RGBA:
                red = image->pixels.rgba[i].red;
                green = image->pixels.rgba[i].green;
                blue = image->pixels.rgba[i].blue;
                break;
            case COLOR_GRAYSCALE:;
                float grayscale = (image->pixels.grayscale + i)->grayscale;
                red = grayscale * (unsigned char) 255;
                green = grayscale * (unsigned char) 255;
                blue = grayscale * (unsigned char) 255;
                break;
            case COLOR_BINARY:;
                unsigned char binary = (image->pixels.binary + i)->binary;
                red = (unsigned char) (binary * 255);
                green = (unsigned char) (binary * 255);
                blue = (unsigned char) (binary * 255);
                break;
            default:
                red = (unsigned char) 0;
                green = (unsigned char) 0;
                blue = (unsigned char) 0;
                break;
        }
        pixels[i].red = red;
        pixels[i].green = green;
        pixels[i].blue = blue;
    }

    switch (image->type) {
        case COLOR_RGBA:
            free(image->pixels.rgba);
            break;
        case COLOR_GRAYSCALE:
            free(image->pixels.grayscale);
            break;
        case COLOR_BINARY:
            free(image->pixels.binary);
            break;
        default:
            break;
    }

    image->type = COLOR_RGB;
    image->pixels.rgb = pixels;

    return SUCCESS;
}

float basic_threshold(void *context, unsigned int x, unsigned int y) {
    (void) x;
    (void) y;
    return *((float *) context);
}

ERROR image_to_binary(IMAGE *image,
                      float (*threshold)(void *, unsigned int, unsigned int),
                      void *context) {
    if (image->type == COLOR_BINARY) return SUCCESS;
    int error = image_to_grayscale(image);
    if (error) return error;
    BINARY *pixels = malloc(image->width * image->height * sizeof(BINARY));
    for (unsigned int y = 0; y < image->height; y++)
        for (unsigned int x = 0; x < image->width; x++) {
            unsigned int index = y * image->width + x;
            pixels[index].binary = image->pixels.grayscale[index].grayscale >
                                           threshold(context, x, y)
                                       ? 1
                                       : 0;
        }

    free(image->pixels.grayscale);

    image->type = COLOR_BINARY;
    image->pixels.binary = pixels;

    return SUCCESS;
}

double matrix_from_image(void *context, unsigned int row, unsigned int column) {
    IMAGE *image = (IMAGE *) context;
    return (double) (image->pixels.binary + row * image->width + column)
        ->binary;
}

ERROR image_to_matrix(IMAGE *image, MATRIX *matrix) {
    ERROR err = SUCCESS;
    if (image->type != COLOR_BINARY) return NOT_HANDLED;
    err_throw(err, matrix_init(matrix, image->height, image->width,
                               matrix_from_image, (void *) image));
    return err;
}
