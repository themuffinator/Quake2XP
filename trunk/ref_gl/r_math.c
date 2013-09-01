/*
Copyright (C) 1997-2001 Quake2xp Team.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "r_local.h"

/*
===========
Q_sincos

===========
*/

#if defined _WIN32 && defined ASM_X86
__forceinline void Q_sincos(float a, float *s, float *c) {
	__asm {
		fld	a
		mov	ecx, s
		mov	edx, c
		fsincos
		fstp	dword ptr [edx]
		fstp	dword ptr [ecx]
	}
}
#else
__inline void Q_sincos(float a, float *s, float *c) {
	*s = (float)sin(a);
	*c = (float)cos(a);
}
#endif

/*
=================
AnglesToMat3

=================
*/
void AnglesToMat3(const vec3_t angles, mat3_t m) {
     float     sp, sy, sr, cp, cy, cr;

     Q_sincos(DEG2RAD(angles[PITCH]), &sp, &cp);
     Q_sincos(DEG2RAD(angles[YAW]), &sy, &cy);
     Q_sincos(DEG2RAD(angles[ROLL]), &sr, &cr);

     m[0][0] = cp * cy;
     m[0][1] = cp * sy;
     m[0][2] = -sp;
     m[1][0] = sr * sp * cy - cr * sy;
     m[1][1] = sr * sp * sy + cr * cy;
     m[1][2] = sr * cp;
     m[2][0] = cr * sp * cy + sr * sy;
     m[2][1] = cr * sp * sy - sr * cy;
     m[2][2] = cr * cp;
}


/*
================
Mat3_TransposeMultiplyVector

Need this to receive local space coords in one quick operation.
================
*/
void Mat3_TransposeMultiplyVector (const mat3_t m, const vec3_t in, vec3_t out) {
	out[0] = m[0][0] * in[0] + m[0][1] * in[1] + m[0][2] * in[2];
	out[1] = m[1][0] * in[0] + m[1][1] * in[1] + m[1][2] * in[2];
	out[2] = m[2][0] * in[0] + m[2][1] * in[1] + m[2][2] * in[2];
}


/*
===========
Mat4_Multiply

===========
*/
void Mat4_Multiply(const mat4_t a, const mat4_t b, mat4_t out) {
#if 0
	int		i, j;

	for (i=0; i<4; i++)
		for (j=0; j<4; j++)
			out[i][j] = a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j] + a[i][3] * b[3][j];
#else
	const float	a00 = a[0][0], a01 = a[0][1], a02 = a[0][2], a03 = a[0][3];
	const float	a10 = a[1][0], a11 = a[1][1], a12 = a[1][2], a13 = a[1][3];
	const float	a20 = a[2][0], a21 = a[2][1], a22 = a[2][2], a23 = a[2][3];
	const float	a30 = a[3][0], a31 = a[3][1], a32 = a[3][2], a33 = a[3][3];

	const float	b00 = b[0][0], b01 = b[0][1], b02 = b[0][2], b03 = b[0][3];
	const float	b10 = b[1][0], b11 = b[1][1], b12 = b[1][2], b13 = b[1][3];
	const float	b20 = b[2][0], b21 = b[2][1], b22 = b[2][2], b23 = b[2][3];
	const float	b30 = b[3][0], b31 = b[3][1], b32 = b[3][2], b33 = b[3][3];

	out[0][0] = a00 * b00 + a01 * b10 + a02 * b20 + a03 * b30;
	out[0][1] = a00 * b01 + a01 * b11 + a02 * b21 + a03 * b31;
	out[0][2] = a00 * b02 + a01 * b12 + a02 * b22 + a03 * b32;
	out[0][3] = a00 * b03 + a01 * b13 + a02 * b23 + a03 * b33;

	out[1][0] = a10 * b00 + a11 * b10 + a12 * b20 + a13 * b30;
	out[1][1] = a10 * b01 + a11 * b11 + a12 * b21 + a13 * b31;
	out[1][2] = a10 * b02 + a11 * b12 + a12 * b22 + a13 * b32;
	out[1][3] = a10 * b03 + a11 * b13 + a12 * b23 + a13 * b33;

	out[2][0] = a20 * b00 + a21 * b10 + a22 * b20 + a23 * b30;
	out[2][1] = a20 * b01 + a21 * b11 + a22 * b21 + a23 * b31;
	out[2][2] = a20 * b02 + a21 * b12 + a22 * b22 + a23 * b32;
	out[2][3] = a20 * b03 + a21 * b13 + a22 * b23 + a23 * b33;

	out[3][0] = a30 * b00 + a31 * b10 + a32 * b20 + a33 * b30;
	out[3][1] = a30 * b01 + a31 * b11 + a32 * b21 + a33 * b31;
	out[3][2] = a30 * b02 + a31 * b12 + a32 * b22 + a33 * b32;
	out[3][3] = a30 * b03 + a31 * b13 + a32 * b23 + a33 * b33;
#endif
}

/*
=================
Mat4_Copy

=================
*/
void Mat4_Copy(const mat4_t in, mat4_t out) {
	out[0][0] = in[0][0];
	out[0][1] = in[0][1];
	out[0][2] = in[0][2];
	out[0][3] = in[0][3];
	out[1][0] = in[1][0];
	out[1][1] = in[1][1];
	out[1][2] = in[1][2];
	out[1][3] = in[1][3];
	out[2][0] = in[2][0];
	out[2][1] = in[2][1];
	out[2][2] = in[2][2];
	out[2][3] = in[2][3];
	out[3][0] = in[3][0];
	out[3][1] = in[3][1];
	out[3][2] = in[3][2];
	out[3][3] = in[3][3];
}

/*
=================
Mat4_Transpose

=================
*/
void Mat4_Transpose(const mat4_t in, mat4_t out) {
	out[0][0] = in[0][0];
	out[0][1] = in[1][0];
	out[0][2] = in[2][0];
	out[0][3] = in[3][0];
	out[1][0] = in[0][1];
	out[1][1] = in[1][1];
	out[1][2] = in[2][1];
	out[1][3] = in[3][1];
	out[2][0] = in[0][2];
	out[2][1] = in[1][2];
	out[2][2] = in[2][2];
	out[2][3] = in[3][2];
	out[3][0] = in[0][3];
	out[3][1] = in[1][3];
	out[3][2] = in[2][3];
	out[3][3] = in[3][3];
}

/*
===============
Mat4_MultiplyVector

===============
*/
void Mat4_MultiplyVector(const mat4_t m, const vec3_t in, vec3_t out) {
	float	s;

	s = m[3][0] * in[0] + m[3][1] * in[1] + m[3][2] * in[2] + m[3][3];

	if (s == 0.0)
		VectorClear(out);
	else if (s == 1.0) {
		out[0] = m[0][0] * in[0] + m[0][1] * in[1] + m[0][2] * in[2] + m[0][3];
		out[1] = m[1][0] * in[0] + m[1][1] * in[1] + m[1][2] * in[2] + m[1][3];
		out[2] = m[2][0] * in[0] + m[2][1] * in[1] + m[2][2] * in[2] + m[2][3];
	}
	else {
		s = 1.0 / s;

		out[0] = (m[0][0] * in[0] + m[0][1] * in[1] + m[0][2] * in[2] + m[0][3]) * s;
		out[1] = (m[1][0] * in[0] + m[1][1] * in[1] + m[1][2] * in[2] + m[1][3]) * s;
		out[2] = (m[2][0] * in[0] + m[2][1] * in[1] + m[2][2] * in[2] + m[2][3]) * s;
	}
}