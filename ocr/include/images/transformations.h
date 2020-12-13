#include "images/image.h"
#include "utils/error.h"

#ifndef H_TRANSOFMATIONS
#define H_TRANSOFMATIONS

#define PI 3.141592

double image_skew_angle(IMAGE *image);
ERROR image_rotate(IMAGE *image, double angle);

ERROR image_scale(IMAGE *image, unsigned int width, unsigned int height);

ERROR image_sub(IMAGE *image, IMAGE **sub, unsigned int x, unsigned int y,
                unsigned int width, unsigned int height);

#endif
