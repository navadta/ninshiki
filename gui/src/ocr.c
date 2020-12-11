#include <gtk/gtk.h>
#include <images/colors.h>
#include <images/conversions.h>
#include <images/image.h>

ERROR image_load(IMAGE **image, char *path) {
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, NULL);
    if (pixbuf == NULL) return IO_ERROR;
    *image = malloc(sizeof(IMAGE));
    if (*image == NULL) return ALLOCATION_FAILED;
    (*image)->width = (unsigned int) gdk_pixbuf_get_width(pixbuf);
    (*image)->height = (unsigned int) gdk_pixbuf_get_height(pixbuf);
    (*image)->type = COLOR_RGB;
    (*image)->pixels.rgb =
        malloc((*image)->width * (*image)->height * sizeof(RGB));
    if ((*image)->pixels.rgb == NULL) {
        free(*image);
        return ALLOCATION_FAILED;
    }
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int has_alpha = gdk_pixbuf_get_has_alpha(pixbuf);
    const unsigned char *pixels = gdk_pixbuf_read_pixels(pixbuf);

    int offset = 0;
    for (unsigned int y = 0; y < (*image)->height; y++) {
        for (unsigned int x = 0; x < (*image)->width; x++) {
            int index = y * (*image)->width + x;
            RGB *pixel = (*image)->pixels.rgb + index;
            pixel->red = *(pixels + offset++);
            pixel->green = *(pixels + offset++);
            pixel->blue = *(pixels + offset++);
            if (has_alpha) offset++;
        }
        offset += rowstride - (*image)->width * (has_alpha ? 4 : 3);
    }
    g_object_unref(G_OBJECT(pixbuf));
    return SUCCESS;
}
