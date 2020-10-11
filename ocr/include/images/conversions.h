#include "images/image.h"
#include "utils/error.h"

#ifndef H_CONVERSIONS
#define H_CONVERSIONS

ERROR image_to_grayscale(IMAGE *image);

ERROR image_to_rgb(IMAGE *image);

ERROR image_to_binary(IMAGE *image,
                      float (*threshold)(unsigned int, unsigned int));

#endif