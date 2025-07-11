#include <string.h>
#include <stdio.h>
#include "matrix_utils.h"

void copy_matrix(Matrix dst, const Matrix src) {
    memcpy(dst, src, sizeof(Matrix));
}

int matrix_equals(const Matrix a, const Matrix b) {
    for (int i = 0; i < MATRIX_SIZE; ++i)
        for (int j = 0; j < MATRIX_SIZE; ++j)
            if (a[i][j] != b[i][j]) return 0;
    return 1;
}

uint64_t matrix_hash(const Matrix m) {
    uint64_t h = 0;
    for (int i = 0; i < MATRIX_SIZE; ++i)
        for (int j = 0; j < MATRIX_SIZE; ++j)
            h = (h << 1) | (m[i][j] & 1);
    return h;
}

void flat_to_matrix(Matrix out, const uint32_t* flat) {
    for (int i = 0; i < MATRIX_SIZE; ++i)
        for (int j = 0; j < MATRIX_SIZE; ++j)
            out[i][j] = flat[i * MATRIX_SIZE + j];
}

void matrix_to_flat(uint32_t* flat, const Matrix in) {
    for (int i = 0; i < MATRIX_SIZE; ++i)
        for (int j = 0; j < MATRIX_SIZE; ++j)
            flat[i * MATRIX_SIZE + j] = in[i][j];
}

void print_matrix(const Matrix m) {
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            printf("%d", m[i][j]);
        }
        printf("\n");
    }
}
