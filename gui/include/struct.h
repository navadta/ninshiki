#ifndef STRUCT_H
#define STRUCT_H

#include <gtk/gtk.h>
#include <images/image.h>

typedef struct ui {
    GtkWindow *window;
    GtkButton *image_button;
    GtkButton *start_button;
    GtkButton *save_button;
    GtkButton *about_button;
    GtkButton *quit_button;
    GtkButton *rotation_button;
    GtkButton *training_button;
    GtkTextView *text_panel;
    GtkTextView *text_panel_2;
    GtkImage *image_panel;
    gint image_x;
    gint image_y;
    GtkScale *rotation_scale;
} UI;

typedef struct app {
    UI ui;
    NETWORK network;
    IMAGE *image;
} App;

#endif
