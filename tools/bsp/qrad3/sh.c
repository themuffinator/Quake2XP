#include "qrad.h"

void SHS_Clear (shSpectralCoeffs_t c) {
	int i, j;

	for (i = 0; i < 3; i++)
	for (j = 0; j < 4; j++)
		c[i][j] = 0.f;
}

void SHS_Copy (const shSpectralCoeffs_t a, shSpectralCoeffs_t b) {
	int i, j;

	for (i = 0; i < 3; i++)
	for (j = 0; j < 4; j++)
		b[i][j] = a[i][j];
}

void SHS_Add (const shSpectralCoeffs_t a, const shSpectralCoeffs_t b, shSpectralCoeffs_t out) {
	int i, j;

	for (i = 0; i < 3; i++)
	for (j = 0; j < 4; j++)
		out[i][j] = a[i][j] + b[i][j];
}

void SHS_Sub (const shSpectralCoeffs_t a, const shSpectralCoeffs_t b, shSpectralCoeffs_t out) {
	int i, j;

	for (i = 0; i < 3; i++)
	for (j = 0; j < 4; j++)
		out[i][j] = a[i][j] - b[i][j];
}

void SH_MulFloat (const shCoeffs_t c, const float f, shSpectralCoeffs_t out) {
	int i, j;

	for (i = 0; i < 3; i++)
	for (j = 0; j < 4; j++)
		out[i][j] = c[j] * f;
}

void SH_MulVec3 (const shCoeffs_t c, const vec3_t vec, shSpectralCoeffs_t out) {
	int i, j;

	for (i = 0; i < 3; i++)
	for (j = 0; j < 4; j++)
		out[i][j] = c[j] * vec[i];
}

void SHS_MulFloat (const shSpectralCoeffs_t sc, const float f, shSpectralCoeffs_t out) {
	int i, j;

	for (i = 0; i < 3; i++)
	for (j = 0; j < 4; j++)
		out[i][j] = sc[i][j] * f;
}

void SHS_MulVec3 (const shSpectralCoeffs_t sc, const vec3_t vec, shSpectralCoeffs_t out) {
	int i, j;

	for (i = 0; i < 3; i++)
	for (j = 0; j < 4; j++)
		out[i][j] = sc[i][j] * vec[i];
}

void SHS_Dot (const shSpectralCoeffs_t sc, const shCoeffs_t c, vec3_t out) {
	int i;

	for (i = 0; i < 3; i++)
		out[i] = sc[i][0] * c[0] + sc[i][1] * c[1] + sc[i][2] * c[2] + sc[i][3] * c[3];
}

void SH_Dir (const vec3_t dir, shCoeffs_t out) {
	out[0] = 0.282094792f;
	out[1] = -0.488602512f * dir[1];
	out[2] = 0.488602512f * dir[2];
	out[3] = -0.488602512f * dir[0];
}

void SH_Cone (const vec3_t dir, const float halfAngle, shCoeffs_t out) {
	const vec2_t zhCoeffs = { 0.5f * (1.f - cosf (halfAngle)), 0.75f * sinf (halfAngle) * sinf (halfAngle) };

	out[0] = zhCoeffs[0];
	out[1] = zhCoeffs[1] * -dir[1];
	out[2] = zhCoeffs[1] * dir[2];
	out[3] = zhCoeffs[1] * -dir[0];
}

void SH_Cone60 (const vec3_t dir, shCoeffs_t out) {
	// (1/2 (1 - Sqrt[3]/2), 3 / 16)
	const vec2_t zhCoeffs = { 0.0669873f, 0.1875f };

	out[0] = zhCoeffs[0];
	out[1] = zhCoeffs[1] * -dir[1];
	out[2] = zhCoeffs[1] * dir[2];
	out[3] = zhCoeffs[1] * -dir[0];
}

void SH_Cone90 (const vec3_t dir, shCoeffs_t out) {
	// (1/2 (1 - 1/Sqrt[2]), 3 / 8)
	const vec2_t zhCoeffs = { 0.146447f, 0.375f };

	out[0] = zhCoeffs[0];
	out[1] = zhCoeffs[1] * -dir[1];
	out[2] = zhCoeffs[1] * dir[2];
	out[3] = zhCoeffs[1] * -dir[0];
}

void SH_Cosine (const vec3_t dir, shCoeffs_t out) {
	out[0] = 0.886227f;
	out[1] = -1.02333f * dir[1];
	out[2] = 1.02333f * dir[2];
	out[3] = -1.02333f * dir[0];
}
