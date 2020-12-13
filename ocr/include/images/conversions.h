#include "images/image.h"
#include "utils/error.h"
#include "utils/utils.h"

#ifndef H_CONVERSIONS
#define H_CONVERSIONS

ERROR image_to_grayscale(IMAGE *image);

ERROR image_to_rgb(IMAGE *image);

float basic_threshold(void *context, unsigned int x, unsigned int y);

ERROR image_to_binary(IMAGE *image,
                      float (*threshold)(void *, unsigned int, unsigned int),
                      void *context);

ERROR image_to_matrix(IMAGE *image, MATRIX *matrix);

#endif
