#include "images/colors.h"
#include "utils/error.h"

#ifndef H_IMAGE
#define H_IMAGE

typedef struct image {
    unsigned int width, height;
    TYPE type;
    COLORS pixels;
} IMAGE;

IMAGE *image_init(unsigned int width, unsigned int height, TYPE type);
void image_free(IMAGE *image);

ERROR load_bitmap(const char *path, IMAGE **image);

ERROR save_to_bitmap(IMAGE *image, const char *path);

ERROR get_pixel(IMAGE *image, unsigned int x, unsigned int y, COLORS *color);

ERROR set_pixel(IMAGE *image, unsigned int x, unsigned int y, COLORS color);

ERROR image_clone(IMAGE *image, IMAGE **cloned);

ERROR image_invert(IMAGE *image);

unsigned char is_white_pixel(IMAGE *image, int x, int y);

#endif
