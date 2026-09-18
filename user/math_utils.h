#ifndef MATH_UTILS_H
#define MATH_UTILS_H

typedef float matrix4x4[4][4];

void matrix4x4_identity(matrix4x4 m);
void matrix4x4_multiply(matrix4x4 dst, const matrix4x4 src1, const matrix4x4 src2);
void matrix4x4_init_orthographic(matrix4x4 m, float left, float right, float bottom, float top, float near, float far);

#endif
