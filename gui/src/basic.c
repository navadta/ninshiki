#include "basic.h"

#include <gtk/gtk.h>
#include <images/conversions.h>
#include <images/image.h>
#include <images/transformations.h>
#include <math.h>
#include <nn/neural_network.h>
#include <nn/training.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <utils/utils.h>

#include "ocr.h"
#include "read_text.h"
#include "struct.h"

void about(GtkButton *button, gpointer user_data) {
    (void) button;
    App *app = user_data;
    GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
    GtkWidget *error = gtk_message_dialog_new(
        app->ui.window, flags, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
        "This application has been created by Epita students");

    gtk_message_dialog_format_secondary_text(
        (GtkMessageDialog *) error, "Thank you for using our app ! <3");

    int result = gtk_dialog_run((GtkDialog *) error);
    if (result == GTK_RESPONSE_OK) {
        gtk_widget_destroy(error);
    }
}

void display_error(App *app, const gchar *first, const gchar *second) {
    GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
    GtkWidget *error = gtk_message_dialog_new(
        app->ui.window, flags, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", first);

    gtk_message_dialog_format_secondary_text((GtkMessageDialog *) error, "%s",
                                             second);
    gtk_widget_destroy(error);
}

void select_image(GtkButton *button, gpointer user_data) {
    (void) button;
    App *app = user_data;

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_add_pixbuf_formats(filter);

    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Open File", app->ui.window, GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel",
        GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);

    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename =
            gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

        if (app->image != NULL) image_free(app->image);
        ERROR err = image_load(&(app->image), filename);
        if (err != SUCCESS) {
            gtk_widget_destroy(dialog);
            display_error(app, "Failed to load image",
                          "Try with another iamge");
            return;
        }

        double angle = image_skew_angle(app->image);
        gtk_range_set_value((GtkRange *) app->ui.rotation_scale, (int) angle);

        IMAGE *clone;
        image_clone(app->image, &clone);

        if (image_rotate(clone, (double) -angle)) return;

        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data(
            (const guchar *) clone->pixels.rgb, GDK_COLORSPACE_RGB, FALSE, 8,
            clone->width, clone->height, clone->width * 3, NULL, NULL);
        pixbuf = gdk_pixbuf_scale_simple(pixbuf, app->ui.image_x,
                                         app->ui.image_y, GDK_INTERP_BILINEAR);
        gtk_image_set_from_pixbuf(app->ui.image_panel, pixbuf);
        image_free(clone);
    }

    gtk_widget_destroy(dialog);
}

void start(GtkButton *button, gpointer user_data) {
    (void) button;
    App *app = user_data;

    if (app->image == NULL) return;

    if (read_text(app) != SUCCESS)
        display_error(app, "an error has occurred during the reading",
                      "Check if the image is valid");
}

void save(GtkButton *button, gpointer user_data) {
    (void) button;
    App *app = user_data;

    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Save File", app->ui.window, GTK_FILE_CHOOSER_ACTION_SAVE, "Cancel",
        GTK_RESPONSE_CANCEL, "Save", GTK_RESPONSE_ACCEPT, NULL);
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);

    gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);

    gint res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(chooser);
        if (*filename == 0) {
            display_error(app, "The file need a name",
                          "Please write something");
        } else {
            FILE *save = fopen(filename, "w");
            fclose(save);
            char *text;
            GtkTextIter start;
            GtkTextIter end;
            gboolean result;
            GError *err = NULL;
            GtkTextBuffer *savebuffer =
                gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->ui.text_panel));
            gtk_text_buffer_get_start_iter(savebuffer, &start);
            gtk_text_buffer_get_end_iter(savebuffer, &end);
            text = gtk_text_buffer_get_text(savebuffer, &start, &end, TRUE);
            gtk_text_buffer_set_modified(savebuffer, FALSE);
            result = g_file_set_contents(filename, text, -1, &err);
            if (result == FALSE) {
                display_error(app, "an error has occured", "Please retry");
            } else {
                FILE *save = fopen(filename, "a");
                fputs("\n", save);
                fclose(save);
            }
        }
    }

    gtk_widget_destroy(dialog);
}

void rotate(GtkButton *button, gpointer user_data) {
    (void) button;
    App *app = user_data;
    if (app->image == NULL) return;

    int angle = gtk_range_get_value((GtkRange *) app->ui.rotation_scale);
    IMAGE *clone;
    image_clone(app->image, &clone);

    if (image_rotate(clone, (double) -angle)) return;

    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data(
        (const guchar *) clone->pixels.rgb, GDK_COLORSPACE_RGB, FALSE, 8,
        clone->width, clone->height, clone->width * 3, NULL, NULL);
    pixbuf = gdk_pixbuf_scale_simple(pixbuf, app->ui.image_x, app->ui.image_y,
                                     GDK_INTERP_BILINEAR);
    gtk_image_set_from_pixbuf(app->ui.image_panel, pixbuf);
    image_free(clone);
}

void train_network(GtkButton *button, gpointer user_data) {
    (void) button;

    App *app = user_data;

    GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
    GtkWidget *msg = gtk_message_dialog_new(
        app->ui.window, flags, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
        "You can train a new network. This action will delete the current "
        "network.");

    gtk_message_dialog_format_secondary_text((GtkMessageDialog *) msg,
                                             "Are you sure to do this ?");

    int result = gtk_dialog_run((GtkDialog *) msg);
    if (result == GTK_RESPONSE_OK) {
        gtk_widget_destroy(msg);

        srand(time(NULL));
        for (unsigned int i = 0; i < app->network.layers_count; i++) {
            layer_free(app->network.layers + i);
            layer_init_random(app->network.layers + i,
                              (app->network.layers + i)->inputs,
                              (app->network.layers + i)->outputs);
        }

        char *fonts[] = {"arial.png", "nunito.png", "roboto.png"};
        unsigned int samples = 3;

        MATRIX *inputs = malloc(CHARSET_LENGTH * samples * sizeof(MATRIX));
        MATRIX *expected = malloc(CHARSET_LENGTH * samples * sizeof(MATRIX));
        printf("Loading dataset...\n\n");
        for (unsigned int i = 0; i < CHARSET_LENGTH; i++) {
            char character = CHARSET[i];
            unsigned char position = char_index(character);
            for (unsigned int j = 0; j < samples; j++) {
                unsigned int index = i * samples + j;
                char *font = fonts[j];

                char p[1024];
                snprintf(p, 1024, "resources/dataset/%s_%d.bmp", font,
                         position);

                IMAGE *image;

                image_load(&image, p);
                image_scale(image, 32, 32);
                float threshold = 0.5f;

                image_to_binary(image, basic_threshold, (void *) &threshold);
                image_to_matrix(image, inputs + index);
                matrix_flatten_column(inputs + index);

                image_free(image);

                char_to_matrix(character, expected + index);
            }
            printf("Loaded %i/%i\n", i + 1, CHARSET_LENGTH);
        }

        network_train(&(app->network), NULL, NULL, 3000, 0.1, 5.0, 30,
                      CHARSET_LENGTH * samples, inputs, expected);

        printf("Finished\n");
        msg = gtk_message_dialog_new(app->ui.window, flags, GTK_MESSAGE_INFO,
                                     GTK_BUTTONS_OK, "Finished.");

        result = gtk_dialog_run((GtkDialog *) msg);
        if (result == GTK_RESPONSE_OK) {
            gtk_widget_destroy(msg);
        }
    } else
        gtk_widget_destroy(msg);
}
