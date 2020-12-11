#include "images/image.h"
#include "utils/error.h"

#ifndef H_CONVOLUTION
#define H_CONVOLUTION

ERROR convolve(IMAGE *image, unsigned int n, float mask[n][n]);

ERROR erode(IMAGE *image, unsigned int n, unsigned char mask[n][n]);

#endif
