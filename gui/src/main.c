#include <gtk/gtk.h>
#include <math.h>
#include <nn/neural_network.h>
#include <stdio.h>

#include "basic.h"
#include "ocr.h"
#include "struct.h"

double sigmoid(void *context, double value) {
    (void) context;
    return 1.0 / (1.0 + exp(-value));
}

double sigmoid_prime(void *context, double value) {
    (void) context;
    return sigmoid(context, value) * (1 - sigmoid(context, value));
}

int main() {
    // Initializes GTK.
    gtk_init(NULL, NULL);

    // Constructs a GtkBuilder instance.
    GtkBuilder *builder = gtk_builder_new();

    // Loads the UI description.
    // (Exits if an error occurs.)
    GError *error = NULL;
    if (gtk_builder_add_from_file(builder, "resources/gui.glade", &error) ==
        0) {
        g_printerr("Error loading file: %s\n", error->message);
        g_clear_error(&error);
        return 1;
    }

    // Gets the widgets.
    GtkWindow *window = GTK_WINDOW(gtk_builder_get_object(builder, "window"));

    GtkButton *image_button =
        GTK_BUTTON(gtk_builder_get_object(builder, "select_image_button"));
    GtkButton *start_button =
        GTK_BUTTON(gtk_builder_get_object(builder, "start_button"));
    GtkButton *save_button =
        GTK_BUTTON(gtk_builder_get_object(builder, "save_button"));
    GtkButton *about_button =
        GTK_BUTTON(gtk_builder_get_object(builder, "about_button"));
    GtkButton *quit_button =
        GTK_BUTTON(gtk_builder_get_object(builder, "quit_button"));
    GtkButton *training_button =
        GTK_BUTTON(gtk_builder_get_object(builder, "training_button"));
    GtkButton *rotation_button =
        GTK_BUTTON(gtk_builder_get_object(builder, "valid_rotation_button"));

    GtkTextView *text_panel =
        GTK_TEXT_VIEW(gtk_builder_get_object(builder, "text_panel"));
    GtkTextView *text_panel_2 =
        GTK_TEXT_VIEW(gtk_builder_get_object(builder, "text_panel_2"));

    GtkImage *image_panel =
        GTK_IMAGE(gtk_builder_get_object(builder, "image_panel"));
    GtkScale *rotation_scale =
        GTK_SCALE(gtk_builder_get_object(builder, "rotation"));

    gtk_range_set_range((GtkRange *) rotation_scale, 0, 359);

    App app = {
        .ui =
            {
                .window = window,
                .image_button = image_button,
                .start_button = start_button,
                .save_button = save_button,
                .about_button = about_button,
                .quit_button = quit_button,
                .training_button = training_button,
                .rotation_button = rotation_button,
                .text_panel = text_panel,
                .text_panel_2 = text_panel_2,
                .image_panel = image_panel,
                .image_x =
                    gtk_widget_get_allocated_width(GTK_WIDGET(image_panel)),
                .image_y =
                    gtk_widget_get_allocated_height(GTK_WIDGET(image_panel)),
                .rotation_scale = rotation_scale,
            },
        .image = NULL,
    };

    FILE *network_file = fopen("resources/network", "r");
    network_load(network_file, &(app.network));
    fclose(network_file);
    app.network.activation_function = sigmoid;
    app.network.activation_function_derivative = sigmoid_prime;

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(image_button, "clicked", G_CALLBACK(select_image), &app);
    g_signal_connect(start_button, "clicked", G_CALLBACK(start), &app);
    g_signal_connect(save_button, "clicked", G_CALLBACK(save), &app);
    g_signal_connect(about_button, "clicked", G_CALLBACK(about), &app);
    g_signal_connect(quit_button, "clicked", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(training_button, "clicked", G_CALLBACK(train_network),
                     &app);
    g_signal_connect(rotation_button, "clicked", G_CALLBACK(rotate), &app);

    gtk_main();

    return 0;
}
