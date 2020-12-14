#ifndef READ_TEXT_H
#define READ_TEXT_H

#include <hunspell/hunspell.h>
#include <images/image.h>
#include <images/segmentation.h>
#include <struct.h>
#include <utils/error.h>

void spellcheck(char *word, Hunhandle *hunspell, char ***s, char *text,
                int end);
void set_text(char *txt, App *app);
void use_network(LINE *line, App *app, IMAGE *clone);
ERROR read_text(App *app);

#endif
