#include "images/transformations.h"

#include <math.h>
#include <stdlib.h>

#include "images/image.h"
#include "utils/error.h"

/*ERROR rotate(IMAGE *image, float angle) {
    angle = angle * PI / 180.0;

    RGB *colors = malloc(image->width * image->height * sizeof(RGB));
    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            int new_px = (int) (x * cos(angle) - y * sin(angle));
            int new_py = (int) (x * sin(angle) + y * cos(angle));

            if (new_px < 0) new_px = 0;
            if (new_px >= image->width) new_px = image->width - 1;
            if (new_py < 0) new_py = 0;
            if (new_py >= image->height) new_py = image->height - 1;

            RGB *pixel = (image->pixels.rgb + y * image->width + x);
            RGB *color = (colors + new_py * image->width - new_px);

            color->red = pixel->red;
            color->green = pixel->green;
            color->blue = pixel->blue;
        }
    }

    free(image->pixels.rgb);
    image->pixels.rgb = colors;

    return SUCCESS;
}*/
