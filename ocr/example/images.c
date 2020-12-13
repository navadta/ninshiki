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

ERROR program(const char *path, const char *name) {
    ERROR err = SUCCESS;

    IMAGE *image;
    err_throw(err, load_bitmap(concat2(path, name), &image));

    err_throw(err, image_to_grayscale(image));
    err_throw(err, image_to_rgb(image));
    err_throw(err, save_to_bitmap(image, concat2(path, "grayscaled.bmp")));

    float wide_gauss[5][5] = {
        {2 / 159.f, 4 / 159.f, 5 / 159.f, 4 / 159.f, 2 / 159.f},
        {4 / 159.f, 9 / 159.f, 12 / 159.f, 9 / 159.f, 4 / 159.f},
        {5 / 159.f, 12 / 159.f, 15 / 159.f, 12 / 159.f, 5 / 159.f},
        {4 / 159.f, 9 / 159.f, 12 / 159.f, 9 / 159.f, 4 / 159.f},
        {2 / 159.f, 4 / 159.f, 5 / 159.f, 4 / 159.f, 2 / 159.f}};

    err_throw(err, convolve(image, 5, wide_gauss));
    err_throw(err, image_to_rgb(image));
    err_throw(err, save_to_bitmap(image, concat2(path, "convolved.bmp")));

    float threshold = compute_otsus_threshold(image);
    err_throw(err,
              image_to_binary(image, basic_threshold, (void *) &threshold));
    err_throw(err, image_to_rgb(image));
    err_throw(err, save_to_bitmap(image, concat2(path, "binarized.bmp")));

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
