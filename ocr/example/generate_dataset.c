#include <gtk/gtk.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "images/colors.h"
#include "images/conversions.h"
#include "images/image.h"
#include "images/segmentation.h"
#include "images/transformations.h"
#include "nn/neural_network.h"
#include "nn/training.h"
#include "utils/error.h"
#include "utils/matrix.h"
#include "utils/utils.h"

ERROR image_load(IMAGE **image, char *path) {
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, NULL);
    if (pixbuf == NULL) return IO_ERROR;
    *image = malloc(sizeof(IMAGE));
    if (*image == NULL) return ALLOCATION_FAILED;
    (*image)->width = (unsigned int) gdk_pixbuf_get_width(pixbuf);
    (*image)->height = (unsigned int) gdk_pixbuf_get_height(pixbuf);
    (*image)->type = COLOR_RGB;
    (*image)->pixels.rgb =
        malloc((*image)->width * (*image)->height * sizeof(RGB));
    if ((*image)->pixels.rgb == NULL) {
        free(*image);
        return ALLOCATION_FAILED;
    }
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int has_alpha = gdk_pixbuf_get_has_alpha(pixbuf);
    const unsigned char *pixels = gdk_pixbuf_read_pixels(pixbuf);

    int offset = 0;
    for (unsigned int y = 0; y < (*image)->height; y++) {
        for (unsigned int x = 0; x < (*image)->width; x++) {
            int index = y * (*image)->width + x;
            RGB *pixel = (*image)->pixels.rgb + index;
            pixel->red = *(pixels + offset++);
            pixel->green = *(pixels + offset++);
            pixel->blue = *(pixels + offset++);
            if (has_alpha) offset++;
        }
        offset += rowstride - (*image)->width * (has_alpha ? 4 : 3);
    }
    g_object_unref(G_OBJECT(pixbuf));
    return SUCCESS;
}

ERROR program(const char *path, const char *name) {
    ERROR err = SUCCESS;

    printf("Generating dataset...\n\n");

    char p[1024];
    snprintf(p, 1024, "%s/%s", path, name);

    IMAGE *image;

    err_throw_msg(err, image_load(&image, p), "Couldn't load image");
    float threshold = 0.5f;
    err_throw(err,
              image_to_binary(image, basic_threshold, (void *) &threshold));

    LINE *line = line_segmentation(image);

    unsigned int index = 0;
    while (line) {
        WORD *word = line->words;
        while (word) {
            CHARACTER *character = word->characters;
            while (character) {
                int c = index++;
                snprintf(p, 1024, "%s/%s_%d.bmp", path, name, c);

                unsigned int width = character->right - character->left;
                unsigned int height = line->higher - line->lower;

                IMAGE *sub;
                err_throw(err,
                          image_sub(image, &sub, word->left + character->left,
                                    line->lower, width, height));
                unsigned int max = width;
                if (height > max) max = height;
                err_throw(err, image_fill(sub, max, max));
                err_throw(err, image_scale(sub, 32, 32));
                err_throw(err, image_to_rgb(sub));
                err_throw(err, save_to_bitmap(sub, p));
                image_free(sub);

                character = character->next;
            }
            word = word->next;
        }
        line = line->next;
    }

    return err;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("You need to precise a path and an image\n");
        return 1;
    }

    ERROR err = program(argv[1], argv[2]);
    printf("%s\n", format_last_error(err));

    return err;
}
