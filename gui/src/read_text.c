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

#define CHARSET                  \
    "0123456789"                 \
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
    "abcdefghijklmnopqrstuvwxyz" \
    "-.,?"
#define CHARSET_LENGTH 66

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
    // hunspell chaque mot + écriture dans la boite de texte
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

ERROR read_text(App *app) {
    ERROR err = SUCCESS;
    // clone image
    IMAGE *clone;
    err_throw(err, image_clone(app->image, &clone));

    // binarizer
    int angle = gtk_range_get_value((GtkRange *) app->ui.rotation_scale);
    if (angle > 0) image_rotate(clone, -angle);

    // float thresh = compute_otsus_threshold(clone)
    float thresh = 0.5f;

    err_throw(err, image_to_binary(clone, basic_threshold, (void *) &thresh));

    // segmentation
    LINE *line = line_segmentation(clone);

    // laiser le nn sur chaque caractère
    // -> retourne une liste chainé de line and word
    err_throw(err, use_network(line, app, clone));

    return err;
}
