#include "images/colors.h"
#include "utils/error.h"

#ifndef H_IMAGE
#define H_IMAGE

typedef struct image {
    unsigned int width, height;
    TYPE type;
    COLORS pixels;
} IMAGE;

ERROR load_bitmap(const char *path, IMAGE **image);

ERROR save_to_bitmap(IMAGE *image, const char *path);

ERROR get_pixel(IMAGE *image, unsigned int x, unsigned int y, COLORS *color);

ERROR set_pixel(IMAGE *image, unsigned int x, unsigned int y, COLORS color);

ERROR clone(IMAGE *image, IMAGE **cloned);

ERROR invert(IMAGE *image);

#endif