#include "images/image.h"
#include "utils/error.h"

#ifndef H_SEGMENTATION
#define H_SEGMENTATION

typedef struct character {
    unsigned int left;
    unsigned int right;
    struct character *next;
} CHARACTER;

typedef struct word {
    unsigned int left;
    unsigned int right;
    CHARACTER *characters;
    struct word *next;
} WORD;

typedef struct line {
    unsigned int lower;
    unsigned int higher;
    WORD *words;
    struct line *next;
} LINE;

typedef struct paragraph {
    unsigned int lower;
    unsigned int higher;
    LINE *lines;
    struct paragraph *next;
} PARAGRAPH;

LINE *line_segmentation(IMAGE *image);
void line_free(LINE *line);

WORD *word_segmentation(IMAGE *image, LINE *line);
void word_free(WORD *word);

CHARACTER *character_segmentation(IMAGE *image, WORD *word, LINE *line);
void character_free(CHARACTER *character);

#endif
