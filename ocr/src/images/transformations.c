#include "images/transformations.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "images/colors.h"
#include "images/image.h"
#include "utils/error.h"

double abs_double(double x) {
    if (x >= 0) return x;
    return -x;
}

static int compute_raycast_sum(IMAGE *image, int height, double angle) {
    angle = angle * PI / 180.0;
    int ray = (int) abs_double(cos(angle) * (int) image->width);
    int w_start = ((int) image->width - ray) / 2;

    int sum = 0;
    for (int w = w_start; w < w_start + ray; w++) {
        int nh = (int) height + tan(angle) * w;
        if (nh >= 0 && nh < (int) image->height &&
            !is_white_pixel(image, w, nh))
            sum++;
    }
    return sum;
}

// https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
static double compute_variance(IMAGE *image, double angle) {
    // Less accuracy, but faster result
    int h_start = (int) image->height / 8;
    int h_len = (7 * (int) image->height) / 8;
    double k = image->width / 15.;

    double sum = 0.;
    double sum_sq = 0.;
    for (int h = h_start; h < h_start + h_len; h += 4) {
        int s = compute_raycast_sum(image, h, angle);
        sum += s - k;
        sum_sq += (s - k) * (s - k);
    }
    return (sum_sq - (sum * sum) / h_len) / (h_len - 1);
}

static double find_skew_angle(IMAGE *image, double lower_bound,
                              double upper_bound, double precision) {
    double skew_angle = 0.;
    double max_variance = 0.;

    for (double angle = lower_bound; angle <= upper_bound; angle += precision) {
        double variance = compute_variance(image, angle);
        if (variance > max_variance) {
            max_variance = variance;
            skew_angle = angle;
        }
    }

    return skew_angle;
}

double image_skew_angle(IMAGE *image) {
    double skew_angle = find_skew_angle(image, -15.f, 15.f, 1.);
    skew_angle = find_skew_angle(image, skew_angle - 3, skew_angle + 3, 0.1);
    return skew_angle;
}

ERROR image_rotate(IMAGE *image, double angle) {
    angle = angle * PI / 180.0;
    double cos_ang = cos(angle);
    double sin_ang = sin(angle);

    int new_width = (int) (abs_double(image->width * cos_ang) +
                           abs_double(image->height * sin_ang));
    int new_height = (int) (abs_double(-((int) image->width) * sin_ang) +
                            abs_double(image->height * cos_ang));

    COLORS colors;
    colors.rgb = NULL;
    colors.rgba = NULL;
    colors.grayscale = NULL;
    colors.binary = NULL;
    switch (image->type) {
        case COLOR_RGB:
            colors.rgb = malloc(new_width * new_height * sizeof(RGB));
            break;
        case COLOR_RGBA:
            colors.rgba = malloc(new_width * new_height * sizeof(RGBA));
            break;
        case COLOR_GRAYSCALE:;
            colors.grayscale =
                malloc(new_width * new_height * sizeof(GRAYSCALE));
            break;
        case COLOR_BINARY:;
            colors.binary = malloc(new_width * new_height * sizeof(BINARY));
            break;
        default:
            break;
    }

    for (int h = 0; h < new_height; h++) {
        for (int w = 0; w < new_width; w++) {
            int index = h * new_width + w;
            int nh =
                (h - new_height / 2) * cos_ang - (w - new_width / 2) * sin_ang;
            int nw =
                (w - new_width / 2) * cos_ang + (h - new_height / 2) * sin_ang;

            nh += image->height / 2;
            nw += image->width / 2;

            if (nh > 0 && nh < (int) image->height && nw > 0 &&
                nw < (int) image->width) {
                COLORS pixel;
                get_pixel(image, nw, nh, &pixel);
                switch (image->type) {
                    case COLOR_RGB:
                        (colors.rgb + index)->red = pixel.rgb->red;
                        (colors.rgb + index)->green = pixel.rgb->green;
                        (colors.rgb + index)->blue = pixel.rgb->blue;
                        break;
                    case COLOR_RGBA:
                        (colors.rgba + index)->red = pixel.rgba->red;
                        (colors.rgba + index)->green = pixel.rgba->green;
                        (colors.rgba + index)->blue = pixel.rgba->blue;
                        (colors.rgba + index)->alpha = pixel.rgba->alpha;
                        break;
                    case COLOR_GRAYSCALE:
                        (colors.grayscale + index)->grayscale =
                            pixel.grayscale->grayscale;
                        break;
                    case COLOR_BINARY:
                        (colors.binary + index)->binary = pixel.binary->binary;
                        break;
                    default:
                        break;
                }
            } else {
                switch (image->type) {
                    case COLOR_RGB:
                        (colors.rgb + index)->red = 255;
                        (colors.rgb + index)->green = 255;
                        (colors.rgb + index)->blue = 255;
                        break;
                    case COLOR_RGBA:
                        (colors.rgba + index)->red = 255;
                        (colors.rgba + index)->green = 255;
                        (colors.rgba + index)->blue = 255;
                        (colors.rgba + index)->alpha = 255;
                        break;
                    case COLOR_GRAYSCALE:
                        (colors.grayscale + index)->grayscale = 1.f;
                        break;
                    case COLOR_BINARY:
                        (colors.binary + index)->binary = 1;
                        break;
                    default:
                        break;
                }
            }
        }
    }

    image->width = (unsigned int) new_width;
    image->height = (unsigned int) new_height;

    switch (image->type) {
        case COLOR_RGB:
            free(image->pixels.rgb);
            image->pixels.rgb = colors.rgb;
            break;
        case COLOR_RGBA:
            free(image->pixels.rgba);
            image->pixels.rgba = colors.rgba;
            break;
        case COLOR_GRAYSCALE:
            free(image->pixels.grayscale);
            image->pixels.grayscale = colors.grayscale;
            break;
        case COLOR_BINARY:
            free(image->pixels.binary);
            image->pixels.binary = colors.binary;
            break;
        default:
            break;
    }

    return SUCCESS;
}
