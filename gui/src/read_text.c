#include <gtk/gtk.h>
#include <hunspell/hunspell.h>
#include <images/conversions.h>
#include <images/image.h>
#include <images/segmentation.h>
#include <images/transformations.h>
#include <nn/neural_network.h>
#include <stdio.h>
#include <stdlib.h>
#include <struct.h>
#include <utils/error.h>
#include <utils/matrix.h>
#include <utils/utils.h>

int spellcheck(char *word, Hunhandle *hunspell, char *text, int new_line,
               int begin) {
    char **s;
    int suggestions = Hunspell_suggest(hunspell, &s, word);
    if (!suggestions) {
        for (int i = 0; word[i] != 0; i++) text[begin++] = word[i];
        if (new_line)
            text[begin++] = '\n';
        else
            text[begin++] = ' ';
        text[begin] = 0;
        return begin;
    }

    for (int i = 0; s[0][i] != 0; i++) text[begin++] = s[0][i];
    if (new_line)
        text[begin++] = '\n';
    else
        text[begin++] = ' ';
    text[begin] = 0;
    return begin;
}

void set_text(char *txt, GtkTextView *text_panel) {
    GtkTextTagTable *table = gtk_text_tag_table_new();
    GtkTextBuffer *buffer = gtk_text_buffer_new(table);
    gtk_text_buffer_set_text(buffer, txt, strlen(txt));
    gtk_text_view_set_buffer(text_panel, buffer);
}

ERROR use_network(LINE *line, App *app, IMAGE *clone) {
    ERROR err = SUCCESS;
    char text[10000];
    char text2[10000];
    int begin = 0;
    int begin2 = 0;
    text[begin] = 0;
    text2[begin2] = 0;
    Hunhandle *h =
        Hunspell_create("resources/fr_FR.aff", "resources/fr_FR.dic");
    char checkable = 0;
    if (Hunspell_spell(h, "bonjour")) {
        checkable = 1;
    }

    while (line) {
        WORD *word = line->words;
        while (word) {
            char w[100];
            int pos = 0;
            CHARACTER *character = word->characters;
            while (character) {
                unsigned int width = character->right - character->left;
                unsigned int height = line->higher - line->lower;

                IMAGE *sub;
                err_throw(err,
                          image_sub(clone, &sub, word->left + character->left,
                                    line->lower, width, height));

                unsigned int max = width;
                if (height > max) max = height;
                err_throw(err, image_fill(sub, max, max));
                err_throw(err, image_scale(sub, 32, 32));

                MATRIX input;
                image_to_matrix(sub, &input);
                image_free(sub);
                matrix_flatten_column(&input);
                MATRIX output;

                network_feedforward(&(app->network), NULL, &input, &output);
                unsigned int row = 0;
                unsigned int column = 0;
                matrix_max(&output, &row, &column);

                w[pos++] = CHARSET[row];

                character = character->next;
            }
            w[pos] = 0;
            for (int i = 0; w[i] != 0; i++) text[begin++] = w[i];
            if (word->next == NULL)
                text[begin++] = '\n';
            else
                text[begin++] = ' ';

            if (checkable)
                begin2 = spellcheck(w, h, text2, !word->next, begin2);
            else {
                for (int i = 0; w[i] != 0; i++) {
                    text[begin++] = w[i];
                    text2[begin2++] = w[i];
                }
                if (word->next == NULL) {
                    text[begin++] = '\n';
                    text2[begin2++] = '\n';
                } else {
                    text[begin++] = ' ';
                    text2[begin2++] = ' ';
                }
                text[begin] = 0;
                text[begin2] = 0;
            }

            word = word->next;
        }
        line = line->next;
    }
    Hunspell_destroy(h);
    set_text(text, app->ui.text_panel);
    set_text(text2, app->ui.text_panel_2);
    return err;
}

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

ERROR read_text(App *app) {
    ERROR err = SUCCESS;
    // clone image
    IMAGE *clone;
    err_throw(err, image_clone(app->image, &clone));

    // binarize
    int angle = gtk_range_get_value((GtkRange *) app->ui.rotation_scale);
    if (angle > 0) image_rotate(clone, -angle);

    float thresh = compute_otsus_threshold(clone);
    err_throw(err, image_to_binary(clone, basic_threshold, (void *) &thresh));

    // segmentation
    LINE *line = line_segmentation(clone);

    // laiser le nn sur chaque caractère
    // -> retourne une liste chainé de line and word
    err_throw(err, use_network(line, app, clone));

    return err;
}
