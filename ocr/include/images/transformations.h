#include "images/image.h"
#include "utils/error.h"

#ifndef H_TRANSOFMATIONS
#define H_TRANSOFMATIONS

#define PI 3.141592

double image_skew_angle(IMAGE *image);
ERROR image_rotate(IMAGE *image, double angle);

#endif