#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H

#include <stdint.h>

#define MATRIX_SIZE 4
#define RULE_BYTES 64

typedef uint8_t Matrix[MATRIX_SIZE][MATRIX_SIZE];

typedef struct {
    uint8_t table[RULE_BYTES];
} Rule512;

void copy_matrix(Matrix dst, const Matrix src);
int matrix_equals(const Matrix a, const Matrix b);
uint64_t matrix_hash(const Matrix m);
void flat_to_matrix(Matrix out, const uint32_t* flat);
void matrix_to_flat(uint32_t* flat, const Matrix in);
void print_matrix(const Matrix m);

#endif
