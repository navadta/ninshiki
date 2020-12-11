#include "images/segmentation.h"

#include <stdio.h>
#include <stdlib.h>

#include "images/colors.h"
#include "images/conversions.h"
#include "images/image.h"
#include "utils/error.h"

typedef struct node {
    int start;
    int end;
    struct node *next;
} NODE;

//  Take a binary image and return an histogram

ERROR histo_height(unsigned int *hist, IMAGE *image) {
    if (image->type != COLOR_BINARY) {
        return NOT_HANDLED;
    }
    for (unsigned int y = 0; y < image->height; y++) {
        hist[y] = 0;
        for (unsigned int x = 0; x < image->width; x++) {
            hist[y] += !(image->pixels.binary[y * image->width + x].binary);
        }
    }
    return SUCCESS;
}

ERROR histo_line(unsigned int *hist, IMAGE *image, LINE *line) {
    if (image->type != COLOR_BINARY) {
        return NOT_HANDLED;
    }
    for (unsigned int x = 0; x < image->width; x++) {
        hist[x] = 0;
        for (unsigned int y = line->lower; y < line->higher; y++) {
            hist[x] += !(image->pixels.binary[y * image->width + x].binary);
        }
    }
    return SUCCESS;
}

ERROR histo_word(unsigned int *hist, IMAGE *image, WORD *word, LINE *line) {
    if (image->type != COLOR_BINARY) {
        return NOT_HANDLED;
    }
    for (unsigned int x = 0; x < word->right - word->left + 1; x++) {
        hist[x] = 0;
        for (unsigned int y = line->lower; y < line->higher; y++) {
            hist[x] += !(
                image->pixels.binary[y * image->width + word->left + x].binary);
        }
    }
    return SUCCESS;
}

CHARACTER *character_segmentation(IMAGE *image, WORD *word, LINE *line) {
    unsigned int *histo =
        malloc((word->right - word->left + 1) * sizeof(unsigned int));
    histo_word(histo, image, word, line);
    unsigned char in_character = 0;
    CHARACTER *first_character = calloc(1, sizeof(CHARACTER));
    CHARACTER *character = first_character;
    CHARACTER *previous = character;

    for (unsigned int x = 0; x < word->right - word->left + 1; x++) {
        if (histo[x] >= 1 && in_character == 0) {
            character->left = x - 1;  // Distance par rapport au mot->left
            in_character = 1;
        } else if ((histo[x] < 1 || x == word->right - word->left) &&
                   in_character == 1) {
            character->right = x;
            in_character = 0;
            previous = character;
            CHARACTER *new_character = calloc(1, sizeof(CHARACTER));
            character->next = new_character;
            character = new_character;
        }
    }

    previous->next = NULL;
    free(character);
    free(histo);
    return first_character;
}

void character_free(CHARACTER *character) {
    while (character) {
        CHARACTER *current = character;
        character = character->next;
        free(current);
    }
}

WORD *word_segmentation(IMAGE *image, LINE *line) {
    unsigned int *histo = malloc(image->width * sizeof(unsigned int));
    histo_line(histo, image, line);
    unsigned char in_word = 0;
    WORD *first_word = calloc(1, sizeof(WORD));
    WORD *word = first_word;

    for (unsigned int x = 0; x < image->width; x++) {
        if (histo[x] >= 2) {
            if (in_word == 0) word->left = x - 1;
            in_word = 5;
        } else {
            if (in_word > 0) in_word--;
            if (in_word == 1 || (in_word > 1 && x == image->width - 1)) {
                word->right = x - 2;
                in_word = 0;
                WORD *new_word = calloc(1, sizeof(WORD));
                word->next = new_word;
                word = new_word;
            }
        }
    }

    word = first_word;
    WORD *previous = word;

    while (word->next) {
        word->characters = character_segmentation(image, word, line);
        previous = word;
        word = word->next;
    }
    previous->next = NULL;
    free(word);
    free(histo);
    return first_word;
}

void word_free(WORD *word) {
    while (word) {
        WORD *current = word;
        word = word->next;
        character_free(current->characters);
        free(current);
    }
}

LINE *line_segmentation(IMAGE *image) {
    unsigned int *histo = malloc(image->height * sizeof(unsigned int));
    histo_height(histo, image);
    unsigned char in_line = 0;
    LINE *first_line = calloc(1, sizeof(LINE));
    LINE *line = first_line;

    for (unsigned int y = 0; y < image->height; y++) {
        if (histo[y] >= 2 && in_line == 0) {
            line->lower = y;
            in_line = 1;
        } else if (in_line == 1 && (histo[y] < 2 || y == image->height - 1)) {
            line->higher = y;
            in_line = 0;
            LINE *new_line = calloc(1, sizeof(LINE));
            line->next = new_line;
            line = new_line;
        }
    }

    line = first_line;
    LINE *previous = line;

    while (line->next) {
        line->words = word_segmentation(image, line);
        previous = line;
        line = line->next;
    }

    previous->next = NULL;
    free(line);
    free(histo);
    return first_line;
}

void line_free(LINE *line) {
    while (line) {
        LINE *current = line;
        line = line->next;
        word_free(current->words);
        free(current);
    }
}
