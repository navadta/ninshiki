#ifndef BASIC_H
#define BASIC_H

#include <gtk/gtk.h>

void start(GtkButton *button, gpointer user_data);
void save(GtkButton *button, gpointer user_data);
void select_image(GtkButton *button, gpointer user_data);
void about(GtkButton *button, gpointer user_data);
void rotate(GtkButton *button, gpointer user_data);
void train_network(GtkButton *button, gpointer user_data);

#endif
