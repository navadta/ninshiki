#include "utils/matrix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/error.h"
#include "utils/random.h"

double random_init(void *context, unsigned int row, unsigned int column) {
    (void) context;
    (void) row;
    (void) column;
    return random_double(-1.0, 1.0);
}

double matrix_init_zeros(void *context, unsigned int row, unsigned int column) {
    (void) row;
    (void) column;
    (void) context;
    return 0.0;
}

ERROR matrix_init(MATRIX *matrix, unsigned int rows, unsigned int columns,
                  double (*function)(void *context, unsigned int, unsigned int),
                  void *context) {
    matrix->rows = rows;
    matrix->columns = columns;
    matrix->values = calloc(rows * columns, sizeof(double));
    if (function != NULL)
        for (unsigned int i = 0; i < rows; i++)
            for (unsigned int j = 0; j < columns; j++)
                matrix->values[i * columns + j] = function(context, i, j);
    return SUCCESS;
}

void matrix_free(MATRIX *matrix) {
    if (matrix->values != NULL) {
        free(matrix->values);  // TODO
        matrix->values = NULL;
    }
}

unsigned int matrix_size(MATRIX *matrix) {
    return matrix->rows * matrix->columns;
}

ERROR matrix_clone(MATRIX *dest, MATRIX *source) {
    matrix_init(dest, source->rows, source->columns, NULL, NULL);
    for (unsigned int i = 0; i < matrix_size(source); i++)
        dest->values[i] = source->values[i];
    return SUCCESS;
}

double matrix_get(MATRIX *matrix, unsigned int row, unsigned int column) {
    return matrix->values[row * matrix->columns + column];
}

void matrix_set(MATRIX *matrix, unsigned int row, unsigned int column,
                double value) {
    matrix->values[row * matrix->columns + column] = value;
}

void matrix_min(MATRIX *matrix, unsigned int *row, unsigned int *column) {
    *row = 0;
    *column = 0;
    for (unsigned int i = 0; i < matrix->rows; i++)
        for (unsigned int j = 0; j < matrix->columns; j++)
            if (matrix->values[i * matrix->columns + j] <
                matrix->values[*row * matrix->columns + *column]) {
                *row = i;
                *column = j;
            }
}

void matrix_max(MATRIX *matrix, unsigned int *row, unsigned int *column) {
    *row = 0;
    *column = 0;
    for (unsigned int i = 0; i < matrix->rows; i++)
        for (unsigned int j = 0; j < matrix->columns; j++)
            if (matrix->values[i * matrix->columns + j] >
                matrix->values[*row * matrix->columns + *column]) {
                *row = i;
                *column = j;
            }
}

ERROR matrix_add(MATRIX *a, MATRIX *b) {
    if (a->rows != b->rows || a->columns != b->columns) {
        char buf[128];
        sprintf(
            buf,
            "Can't add two matrices of different sizes: a(%u, %u) + b(%u, %u)",
            a->rows, a->columns, b->rows, b->columns);
        set_last_error_message(NOT_HANDLED, buf);
        return NOT_HANDLED;
    }
    for (unsigned int i = 0; i < a->rows * a->columns; i++)
        b->values[i] = b->values[i] + a->values[i];
    return SUCCESS;
}

ERROR matrix_sub(MATRIX *a, MATRIX *b) {
    if (a->rows != b->rows || a->columns != b->columns) {
        char buf[128];
        sprintf(buf,
                "Can't substract two matrices of different sizes: a(%u, %u) - "
                "b(%u, %u)",
                a->rows, a->columns, b->rows, b->columns);
        set_last_error_message(NOT_HANDLED, buf);
        return NOT_HANDLED;
    }
    for (unsigned int i = 0; i < a->rows * a->columns; i++)
        b->values[i] = a->values[i] - b->values[i];
    return SUCCESS;
}

ERROR matrix_scalar(MATRIX *matrix, double scalar) {
    for (unsigned int i = 0; i < matrix->rows * matrix->columns; i++)
        matrix->values[i] = matrix->values[i] * scalar;
    return SUCCESS;
}

ERROR matrix_mul(MATRIX *a, MATRIX *b) {
    if (a->columns != b->rows) {
        char buf[128];
        sprintf(buf,
                "Can't multiply two matrices where a.columns != b.rows: a(%u, "
                "%u) * "
                "b(%u, %u)",
                a->rows, a->columns, b->rows, b->columns);
        set_last_error_message(NOT_HANDLED, buf);
        return NOT_HANDLED;
    }
    double *values = calloc(a->rows * b->columns, sizeof(double));
    for (unsigned int r = 0; r < a->rows; r++)
        for (unsigned int c = 0; c < b->columns; c++)
            for (unsigned int s = 0; s < a->columns; s++)
                values[r * b->columns + c] = values[r * b->columns + c] +
                                             a->values[r * a->columns + s] *
                                                 b->values[s * b->columns + c];
    free(b->values);
    b->values = values;
    b->rows = a->rows;
    return SUCCESS;
}

ERROR matrix_mul_2(MATRIX *a, MATRIX *b) {
    if (a->columns != b->rows) {
        char buf[128];
        sprintf(buf,
                "Can't multiply two matrices where a.columns != b.rows: a(%u, "
                "%u) * "
                "b(%u, %u)",
                a->rows, a->columns, b->rows, b->columns);
        set_last_error_message(NOT_HANDLED, buf);
        return NOT_HANDLED;
    }
    double *values = calloc(a->rows * b->columns, sizeof(double));
    for (unsigned int i = 0; i < a->rows; i++)
        for (unsigned int j = 0; j < b->columns; j++)
            for (unsigned int k = 0; k < b->rows; k++)
                values[i * b->columns + j] = values[i * b->columns + j] +
                                             a->values[i * a->columns + k] *
                                                 b->values[k * b->columns + j];
    free(a->values);
    a->values = values;
    a->columns = b->columns;
    return SUCCESS;
}

ERROR matrix_hadamard_mul(MATRIX *a, MATRIX *b) {
    if (a->rows != b->rows || a->columns != b->columns) {
        char buf[128];
        sprintf(buf,
                "Can't process Hadamard multiplication on two different size "
                "matrices: a(%u, %u) * b(%u, %u)",
                a->rows, a->columns, b->rows, b->columns);
        set_last_error_message(NOT_HANDLED, buf);
        return NOT_HANDLED;
    }
    for (unsigned int i = 0; i < a->rows * a->columns; i++)
        b->values[i] = a->values[i] * b->values[i];
    return SUCCESS;
}

ERROR matrix_transpose(MATRIX *matrix) {
    double *values = malloc(matrix->rows * matrix->columns * sizeof(double));
    for (unsigned int i = 0; i < matrix->rows; i++)
        for (unsigned int j = 0; j < matrix->columns; j++)
            values[j * matrix->rows + i] =
                matrix->values[i * matrix->columns + j];
    free(matrix->values);
    matrix->values = values;
    unsigned int columns = matrix->columns;
    matrix->columns = matrix->rows;
    matrix->rows = columns;
    return SUCCESS;
}

ERROR matrix_flatten_row(MATRIX *matrix) {
    matrix->columns = matrix->rows * matrix->columns;
    matrix->rows = 1;
    return SUCCESS;
}

ERROR matrix_flatten_column(MATRIX *matrix) {
    matrix->rows = matrix->rows * matrix->columns;
    matrix->columns = 1;
    return SUCCESS;
}

void matrix_apply(MATRIX *matrix, double (*function)(void *, double),
                  void *context) {
    for (unsigned int i = 0; i < matrix->rows * matrix->columns; i++)
        matrix->values[i] = function(context, matrix->values[i]);
}

int matrix_save(FILE *file, MATRIX *matrix) {
    if (file == NULL) return 1;
    char str[10];
    sprintf(str, "%u,", matrix->rows);
    fputs(str, file);
    sprintf(str, "%u,", matrix->columns);
    fputs(str, file);
    fputs("[", file);

    for (unsigned int i = 0; i < matrix->rows * matrix->columns - 1; i++) {
        snprintf(str, 10, "%f", matrix->values[i]);
        fputs(str, file);
        fputs(",", file);
    }
    snprintf(str, 10, "%f", matrix->values[matrix->rows * matrix->columns - 1]);
    fputs(str, file);
    fputs("]\n", file);

    return 0;
}

ERROR matrix_load(FILE *file, MATRIX *matrix) {
    unsigned int rows;
    unsigned int columns;
    if (fscanf(file, "%u,%u,[", &rows, &columns) == 0) return IO_ERROR;
    ERROR err = matrix_init(matrix, rows, columns, NULL, NULL);
    if (err) return err;
    // unsigned int *number;
    // unsigned int current = fgetc(file);

    unsigned int i = 0;
    while (i != rows * columns - 1) {
        // current = fgetc(file);
        if (fscanf(file, "%lf,", matrix->values + i) == 0) return IO_ERROR;
        i++;
    }

    if (fscanf(file, "%lf]", matrix->values + i) == 0) return IO_ERROR;
    return SUCCESS;
}
