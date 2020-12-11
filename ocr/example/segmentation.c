#include "images/segmentation.h"

#include <stdio.h>
#include <stdlib.h>

#include "images/colors.h"
#include "images/conversions.h"
#include "images/convolution.h"
#include "images/image.h"
#include "images/transformations.h"
#include "utils/error.h"
#include "utils/utils.h"

float compute_otsus_threshold(IMAGE *image) {
    image_to_grayscale(image);

    // Get the histogram
    long double histogram[256];

    // initialize all intensity values to 0
    for (int i = 0; i < 256; i++) histogram[i] = 0;

    COLORS pixel;
    // calculate the no of pixels for each intensity values
    for (unsigned int y = 0; y < image->height; y++)
        for (unsigned int x = 0; x < image->width; x++) {
            get_pixel(image, x, y, &pixel);
            histogram[(int) (pixel.grayscale->grayscale * 255)]++;
        }

    // Calculate the bin_edges
    long double bin_edges[256];
    bin_edges[0] = 0.0;
    long double increment = 0.99609375;
    for (int i = 1; i < 256; i++) bin_edges[i] = bin_edges[i - 1] + increment;

    // Calculate bin_mids
    long double bin_mids[256];
    for (int i = 0; i < 256; i++)
        bin_mids[i] = (bin_edges[i] + bin_edges[(i + 1) % 256]) / 2;

    // Iterate over all thresholds (indices) and get the probabilities weight1,
    // weight2
    long double weight1[256];
    weight1[0] = histogram[0];
    for (int i = 1; i < 256; i++) weight1[i] = histogram[i] + weight1[i - 1];

    int total_sum = 0;
    for (int i = 0; i < 256; i++) total_sum = total_sum + histogram[i];
    long double weight2[256];
    weight2[0] = total_sum;
    for (int i = 1; i < 256; i++)
        weight2[i] = weight2[i - 1] - histogram[i - 1];

    // Calculate the class means: mean1 and mean2
    long double histogram_bin_mids[256];
    for (int i = 0; i < 256; i++)
        histogram_bin_mids[i] = histogram[i] * bin_mids[i];

    long double cumsum_mean1[256];
    cumsum_mean1[0] = histogram_bin_mids[0];
    for (int i = 1; i < 256; i++)
        cumsum_mean1[i] = cumsum_mean1[i - 1] + histogram_bin_mids[i];

    long double cumsum_mean2[256];
    cumsum_mean2[0] = histogram_bin_mids[255];
    for (int i = 1, j = 254; i < 256 && j >= 0; i++, j--)
        cumsum_mean2[i] = cumsum_mean2[i - 1] + histogram_bin_mids[j];

    long double mean1[256];
    for (int i = 0; i < 256; i++) mean1[i] = cumsum_mean1[i] / weight1[i];

    long double mean2[256];
    for (int i = 0, j = 255; i < 256 && j >= 0; i++, j--)
        mean2[j] = cumsum_mean2[i] / weight2[j];

    // Calculate Inter_class_variance
    long double Inter_class_variance[255];
    long double dnum = 10000000000;
    for (int i = 0; i < 255; i++)
        Inter_class_variance[i] =
            ((weight1[i] * weight2[i] * (mean1[i] - mean2[i + 1])) / dnum) *
            (mean1[i] - mean2[i + 1]);

    // Maximize interclass variance
    long double maxi = 0;
    int getmax = 0;
    for (int i = 0; i < 255; i++) {
        if (maxi < Inter_class_variance[i]) {
            maxi = Inter_class_variance[i];
            getmax = i;
        }
    }

    return bin_mids[getmax] / 255.f;
}

ERROR draw_segmentation(IMAGE *image, LINE *line) {
    ERROR err = SUCCESS;

    COLORS line_color;
    line_color.rgb = calloc(1, sizeof(RGB));
    line_color.rgb->blue = 255;

    COLORS word_color;
    word_color.rgb = calloc(1, sizeof(RGB));
    word_color.rgb->green = 255;

    COLORS character_color;
    character_color.rgb = calloc(1, sizeof(RGB));
    character_color.rgb->red = 255;

    while (line) {
        for (unsigned int y = line->lower; y < line->higher; y++) {
            err_throw(err, set_pixel(image, 0, y, line_color));
            err_throw(err, set_pixel(image, 1, y, line_color));
        }
        WORD *word = line->words;
        while (word) {
            for (unsigned int x = word->left; x < word->right; x++) {
                err_throw(err, set_pixel(image, x, line->higher, word_color));
            }
            CHARACTER *character = word->characters;
            while (character) {
                for (unsigned int x = character->left; x < character->right - 1;
                     x++) {
                    err_throw(err,
                              set_pixel(image, word->left + x + 1,
                                        line->higher - 2, character_color));
                }
                character = character->next;
            }
            word = word->next;
        }
        line = line->next;
    }

    free(line_color.rgb);
    free(word_color.rgb);
    free(character_color.rgb);

    return err;
}

ERROR program(const char *path, const char *name) {
    ERROR err = SUCCESS;

    IMAGE *image;
    err_throw(err, load_bitmap(concat2(path, name), &image));

    float blur[3][3] = {{1.f / 9.f, 1.f / 9.f, 1.f / 9.f},
                        {1.f / 9.f, 1.f / 9.f, 1.f / 9.f},
                        {1.f / 9.f, 1.f / 9.f, 1.f / 9.f}};
    float sharpen[3][3] = {
        {0.f, -1.f, 0.f}, {-1.f, 5.f, -1.f}, {0.f, -1.f, 0.f}};
    float gaussian_blur[3][3] = {{1.f / 16.f, 2.f / 16.f, 1.f / 16.f},
                                 {2.f / 16.f, 4.f / 16.f, 1.f / 16.f},
                                 {1.f / 16.f, 2.f / 16.f, 1.f / 16.f}};

    double angle = image_skew_angle(image);
    printf("Skew angle: %f\n", angle);
    image_rotate(image, -angle);

    IMAGE *cloned;
    err_throw(err, image_clone(image, &cloned));

    err_throw(err, convolve(image, 3, blur));
    err_throw(err, convolve(image, 3, sharpen));
    err_throw(err, convolve(image, 3, blur));
    err_throw(err, convolve(image, 3, gaussian_blur));

    float thresh = compute_otsus_threshold(image);

    err_throw(err, image_to_binary(image, basic_threshold, (void *) &thresh));

    LINE *line = line_segmentation(image);
    err_throw(err, draw_segmentation(cloned, line));

    err_throw(err, save_to_bitmap(cloned, concat2(path, "segmented.bmp")));

    line_free(line);
    image_free(image);
    image_free(cloned);

    return err;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("You need to precise a path and a bitmap image\n");
        return 1;
    }

    ERROR err = program(argv[1], argv[2]);
    printf("%s\n", format_last_error(err));

    return err;
}
