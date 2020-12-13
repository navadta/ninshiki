#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>

#include "utils/error.h"

typedef struct matrix {
    unsigned int rows;
    unsigned int columns;
    double *values;
} MATRIX;

double random_init(void *context, unsigned int row, unsigned int column);
double matrix_init_zeros(void *context, unsigned int row, unsigned int column);

ERROR matrix_init(MATRIX *matrix, unsigned int rows, unsigned int columns,
                  double (*function)(void *context, unsigned int, unsigned int),
                  void *context);
void matrix_free(MATRIX *matrix);

unsigned int matrix_size(MATRIX *matrix);
ERROR matrix_clone(MATRIX *dest, MATRIX *source);

double matrix_get(MATRIX *matrix, unsigned int row, unsigned int column);
void matrix_set(MATRIX *matrix, unsigned int row, unsigned int column,
                double value);
void matrix_min(MATRIX *matrix, unsigned int *row, unsigned int *column);
void matrix_max(MATRIX *matrix, unsigned int *row, unsigned int *column);

// B is the result holder
ERROR matrix_add(MATRIX *a, MATRIX *b);
ERROR matrix_sub(MATRIX *a, MATRIX *b);
ERROR matrix_scalar(MATRIX *matrix, double scalar);
ERROR matrix_mul(MATRIX *a, MATRIX *b);
ERROR matrix_mul_2(MATRIX *a, MATRIX *b);
ERROR matrix_hadamard_mul(MATRIX *a, MATRIX *b);
ERROR matrix_transpose(MATRIX *matrix);
ERROR matrix_flatten_row(MATRIX *matrix);
ERROR matrix_flatten_column(MATRIX *matrix);

void matrix_apply(MATRIX *matrix, double (*function)(void *context, double),
                  void *context);

int matrix_save(FILE *file, MATRIX *matrix);
ERROR matrix_load(FILE *file, MATRIX *matrix);
#endif
