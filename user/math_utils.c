#include "math_utils.h"

/*
 * Note: matrices are row-major.
 */

void matrix4x4_identity(matrix4x4 m) {
	m[0][1] = m[0][2] = m[0][3] = 0.0f;
	m[1][0] = m[1][2] = m[1][3] = 0.0f;
	m[2][0] = m[2][1] = m[2][3] = 0.0f;
	m[3][0] = m[3][1] = m[3][2] = 0.0f;
	m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
}

void matrix4x4_multiply(matrix4x4 dst, const matrix4x4 src1, const matrix4x4 src2) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			dst[i][j] = 0.0f;
			for (int k = 0; k < 4; k++) {
				dst[i][j] += src1[i][k] * src2[k][j];
			}
		}
	}
}

void matrix4x4_init_orthographic(matrix4x4 m, float left, float right, float bottom, float top, float near, float far) {
	m[0][0] = 2.0f / (right - left);
	m[0][1] = 0.0f;
	m[0][2] = 0.0f;
	m[0][3] = -(right + left) / (right - left);

	m[1][0] = 0.0f;
	m[1][1] = 2.0f / (top - bottom);
	m[1][2] = 0.0f;
	m[1][3] = -(top + bottom) / (top - bottom);

	m[2][0] = 0.0f;
	m[2][1] = 0.0f;
	m[2][2] = -2.0f / (far - near);
	m[2][3] = -(far + near) / (far - near);

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;
}
