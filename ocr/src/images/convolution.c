#include "images/convolution.h"

#include <stdlib.h>

#include "images/conversions.h"

unsigned char clamp(float f) {
    return f > 255 ? 255 : f < 0 ? 0 : (unsigned char) f;
}

unsigned int is_valid(int x, int y, int width, int height) {
    return x >= 0 && x < width && y >= 0 && y < height;
}

ERROR convolve(IMAGE *image, unsigned int n, float mask[n][n]) {
    ERROR error = image_to_rgb(image);
    if (error) return error;

    int offset = n / 2;

    RGB *colors = malloc(image->width * image->height * sizeof(RGB));

    for (unsigned int y = 0; y < image->height; y++) {
        for (unsigned int x = 0; x < image->width; x++) {
            float red = 0, green = 0, blue = 0;
            for (int dy = -offset; dy <= offset; dy++) {
                for (int dx = -offset; dx <= offset; dx++) {
                    if (is_valid(x + dx, y + dy, image->width, image->height)) {
                        unsigned int index = (y + dy) * image->width + x + dx;
                        RGB *color = image->pixels.rgb + index;

                        float coefficient = mask[dy + offset][dx + offset];

                        // float coefficient = *((mask + (dy + offset) * n) + dx
                        // + offset);

                        red += color->red * coefficient;
                        green += color->green * coefficient;
                        blue += color->blue * coefficient;
                    }
                }
            }

            unsigned int index = y * image->width + x;
            RGB *color = colors + index;

            color->red = clamp(red);
            color->green = clamp(green);
            color->blue = clamp(blue);
        }
    }

    free(image->pixels.rgb);
    image->pixels.rgb = colors;

    return SUCCESS;
}
