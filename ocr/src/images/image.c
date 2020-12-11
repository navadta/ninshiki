
#include "images/image.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "images/colors.h"
#include "images/conversions.h"
#include "utils/error.h"
#include "utils/utils.h"

IMAGE *image_init(unsigned int width, unsigned int height, TYPE type) {
    IMAGE *image = malloc(sizeof(IMAGE));
    image->width = width;
    image->height = height;
    image->type = type;
    switch (type) {
        case COLOR_RGB:
            image->pixels.rgb = malloc(width * height * sizeof(RGB));
            break;
        case COLOR_RGBA:
            image->pixels.rgba = malloc(width * height * sizeof(RGBA));
            break;
        case COLOR_GRAYSCALE:
            image->pixels.grayscale =
                malloc(width * height * sizeof(GRAYSCALE));
            break;
        case COLOR_BINARY:
            image->pixels.binary = malloc(width * height * sizeof(BINARY));
            break;
    }
    return image;
}

void image_free(IMAGE *image) {
    switch (image->type) {
        case COLOR_RGB:
            free(image->pixels.rgb);
            break;
        case COLOR_RGBA:
            free(image->pixels.rgba);
            break;
        case COLOR_GRAYSCALE:
            free(image->pixels.grayscale);
            break;
        case COLOR_BINARY:
            free(image->pixels.binary);
            break;
    }
    free(image);
}

ERROR load_bitmap(const char *path, IMAGE **image) {
    FILE *file = fopen(path, "r");  // Open file with read access
    if (!file) {
        set_last_error_message(
            IO_ERROR, concat3("Couldn't open file (", strerror(errno), ")"));
        return IO_ERROR;
    }

    unsigned char header[14];
    if (!fread(header, 14, 1, file)) {
        set_last_error_message(IO_ERROR, "Couldn't read Bitmap Header");
        return IO_ERROR;
    }
    if (header[0] != 'B' || header[1] != 'M') {
        set_last_error_message(NOT_HANDLED, "Bitmap format not handled");
        return NOT_HANDLED;
    }
    unsigned int image_offset = *(unsigned int *) (header + 0x0A);

    if (fseek(file, 0x0E, SEEK_SET)) {
        set_last_error_message(
            IO_ERROR,
            concat3("Couldn't seek to file position (", strerror(errno), ")"));
        return IO_ERROR;
    }
    unsigned char bitmap_info[40];
    if (!fread(&bitmap_info, 40, 1, file)) {
        set_last_error_message(IO_ERROR, "Couldn't read Bitmap informations");
        return IO_ERROR;
    }
    unsigned short bpp = *(unsigned short *) (bitmap_info + 14);
    if (bpp != 24) {
        set_last_error_message(NOT_HANDLED,
                               "Bitmap format not handled (Not 24bpp)");
        return NOT_HANDLED;
    }
    unsigned int compression = *(unsigned int *) (bitmap_info + 16);
    if (compression != 0) {
        set_last_error_message(NOT_HANDLED,
                               "Bitmap format not handled (Compressed)");
        return NOT_HANDLED;
    }

    unsigned int width = *(unsigned int *) (bitmap_info + 4);
    unsigned int height = *(unsigned int *) (bitmap_info + 8);
    *image = image_init(width, height, COLOR_RGB);

    if (fseek(file, image_offset, SEEK_SET)) {
        set_last_error_message(
            IO_ERROR,
            concat3("Couldn't seek to file position (", strerror(errno), ")"));
        return IO_ERROR;
    }
    unsigned int padding = (4 - (width * 3) % 4) % 4;
    unsigned int row_size = width * 3 + padding;
    unsigned char *buffer = malloc(row_size * height);
    if (fread(buffer, sizeof(unsigned char), row_size * height, file) !=
        row_size * height) {
        set_last_error_message(
            IO_ERROR, concat3("Couldn't read Bitmap (", strerror(errno), ")"));
        return IO_ERROR;
    }
    struct color {
        unsigned char b, g, r;
    };
    struct color *pixels = malloc(width * height * sizeof(struct color));
    unsigned int position = 0;
    for (unsigned int index = 0; index < height; index++) {
        memcpy((pixels + index * width), (buffer + position),
               sizeof(struct color) * width);
        position += sizeof(struct color) * width + padding;
    }
    free(buffer);
    for (unsigned int i = 0; i < width * height; i++) {
        struct color bgr = *(struct color *) (pixels + i);
        RGB rgb;
        rgb.red = bgr.r;
        rgb.green = bgr.g;
        rgb.blue = bgr.b;
        unsigned int x = i % width, y = i / width;
        unsigned int index = (height - y - 1) * width + x;
        (*image)->pixels.rgb[index] = rgb;
    }
    free(pixels);

    if (fclose(file)) {
        set_last_error_message(
            IO_ERROR, concat3("Couldn't close file (", strerror(errno), ")"));
        return IO_ERROR;
    }

    return SUCCESS;
}

ERROR save_to_bitmap(IMAGE *image, const char *path) {
    if (image->type != COLOR_RGB) {
        set_last_error_message(NOT_HANDLED,
                               "Only RGB Images can be saved to Bitmaps");
        return NOT_HANDLED;
    }
    unsigned short bpp = 24;
    unsigned int padding = (4 - (image->width * 3) % 4) % 4;
    unsigned int row_size = image->width * 3 + padding;
    unsigned int size = 14 + 40 + row_size * image->height;
    unsigned char *bitmap = calloc(size, 1);

    // Header
    bitmap[0] = 'B';
    bitmap[1] = 'M';
    *((unsigned int *) (bitmap + 0x02)) = size;
    *((unsigned int *) (bitmap + 0x0A)) = 14 + 40;

    // Info Header
    *((unsigned int *) (bitmap + 0x0E)) = 40;
    *((unsigned int *) (bitmap + 0x12)) = image->width;
    *((unsigned int *) (bitmap + 0x16)) = image->height;
    *((unsigned short *) (bitmap + 0x1A)) = 1;
    *((unsigned short *) (bitmap + 0x1C)) = bpp;
    *((unsigned int *) (bitmap + 0x22)) = row_size * image->height;
    *((unsigned int *) (bitmap + 0x26)) = 3780;
    *((unsigned int *) (bitmap + 0x2A)) = 3780;

    // Data
    unsigned int offset = 0x36;
    unsigned int index = 0;
    for (int y = image->height; y > 0; --y) {
        for (unsigned int x = 0; x < image->width; ++x) {
            unsigned int i = (y - 1) * image->width + x;
            RGB *rgb = (image->pixels.rgb + i);
            bitmap[index + offset] = rgb->blue;
            bitmap[index + 1 + offset] = rgb->green;
            bitmap[index + 2 + offset] = rgb->red;
            index += 3;
        }
        index += padding;  // Add padding
    }

    FILE *file = fopen(path, "w");  // Open file with write access
    if (!file) {
        set_last_error_message(
            IO_ERROR, concat3("Couldn't open file (", strerror(errno), ")"));
        return IO_ERROR;
    }

    if (!fwrite(bitmap, size, 1, file)) {
        set_last_error_message(IO_ERROR, concat3("Couldn't write to file (",
                                                 strerror(errno), ")"));
        return IO_ERROR;
    }

    free(bitmap);

    if (fclose(file)) {
        set_last_error_message(
            IO_ERROR, concat3("Couldn't close file (", strerror(errno), ")"));
        return IO_ERROR;
    }

    return SUCCESS;
}

// TODO More concise error messages
ERROR get_pixel(IMAGE *image, unsigned int x, unsigned int y, COLORS *color) {
    if (x >= image->width || y >= image->height) {
        set_last_error_message(INDEX_OUT_OF_BOUNDS, "image#get_pixel");
        return INDEX_OUT_OF_BOUNDS;
    }
    switch (image->type) {
        case COLOR_RGB:
            color->rgb = &(image->pixels.rgb[y * image->width + x]);
            break;
        case COLOR_RGBA:
            color->rgba = &(image->pixels.rgba[y * image->width + x]);
            break;
        case COLOR_GRAYSCALE:
            color->grayscale = &(image->pixels.grayscale[y * image->width + x]);
            break;
        case COLOR_BINARY:
            color->binary = &(image->pixels.binary[y * image->width + x]);
            break;
    }
    return SUCCESS;
}

// TODO More concise error messages
ERROR set_pixel(IMAGE *image, unsigned int x, unsigned int y, COLORS color) {
    if (x >= image->width || y >= image->height) {
        set_last_error_message(INDEX_OUT_OF_BOUNDS, "image#set_pixel");
        return INDEX_OUT_OF_BOUNDS;
    }
    switch (image->type) {
        case COLOR_RGB:
            image->pixels.rgb[y * image->width + x] = *((RGB *) color.rgb);
            break;
        case COLOR_RGBA:
            image->pixels.rgba[y * image->width + x] = *((RGBA *) color.rgba);
            break;
        case COLOR_GRAYSCALE:
            image->pixels.grayscale[y * image->width + x] =
                *((GRAYSCALE *) color.grayscale);
            break;
        case COLOR_BINARY:
            image->pixels.binary[y * image->width + x] =
                *((BINARY *) color.binary);
            break;
    }
    return SUCCESS;
}

ERROR image_clone(IMAGE *image, IMAGE **cloned) {
    *cloned = image_init(image->width, image->height, image->type);
    for (unsigned int index = 0; index < image->width * image->height;
         ++index) {
        switch (image->type) {
            case COLOR_RGB:
                (*cloned)->pixels.rgb[index] = image->pixels.rgb[index];
                break;
            case COLOR_RGBA:
                (*cloned)->pixels.rgba[index] = image->pixels.rgba[index];
                break;
            case COLOR_GRAYSCALE:
                (*cloned)->pixels.grayscale[index] =
                    image->pixels.grayscale[index];
                break;
            case COLOR_BINARY:
                (*cloned)->pixels.binary[index] = image->pixels.binary[index];
                break;
            default:
                break;
        }
    }
    return SUCCESS;
}

ERROR image_invert(IMAGE *image) {
    for (unsigned int i = 0; i < image->width * image->height; ++i) {
        switch (image->type) {
            case COLOR_RGB:
                image->pixels.rgb[i].red = 255 - image->pixels.rgb[i].red;
                image->pixels.rgb[i].green = 255 - image->pixels.rgb[i].green;
                image->pixels.rgb[i].blue = 255 - image->pixels.rgb[i].blue;
                break;
            case COLOR_RGBA:
                image->pixels.rgba[i].red = 255 - image->pixels.rgba[i].red;
                image->pixels.rgba[i].green = 255 - image->pixels.rgba[i].green;
                image->pixels.rgba[i].blue = 255 - image->pixels.rgba[i].blue;
                image->pixels.rgba[i].alpha = 255 - image->pixels.rgba[i].alpha;
                break;
            case COLOR_GRAYSCALE:
                image->pixels.grayscale[i].grayscale =
                    1.f - image->pixels.grayscale[i].grayscale;
                break;
            case COLOR_BINARY:
                image->pixels.binary[i].binary =
                    1 - image->pixels.binary[i].binary;
                break;
        }
    }

    return SUCCESS;
}

unsigned char is_white_pixel(IMAGE *image, int x, int y) {
    int index = y * image->width + x;
    switch (image->type) {
        case COLOR_RGB:
            return (image->pixels.rgb + index)->red == 255 &&
                   (image->pixels.rgb + index)->green == 255 &&
                   (image->pixels.rgb + index)->blue == 255;
        case COLOR_RGBA:
            return (image->pixels.rgba + index)->red == 255 &&
                   (image->pixels.rgba + index)->green == 255 &&
                   (image->pixels.rgba + index)->blue == 255;
        case COLOR_GRAYSCALE:
            return (image->pixels.grayscale + index)->grayscale == 1.0f;
        case COLOR_BINARY:
            return (image->pixels.binary + index)->binary == 1;
        default:
            return 1;
    }
}
