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

#define Q_INLINE		__forceinline

Q_INLINE void Q_sincos(float a, float *s, float *c) {
#if defined _WIN32 && defined ASM_X86
	__asm {
		fld	a
		mov	ecx, s
		mov	edx, c
		fsincos
		fstp	dword ptr [edx]
		fstp	dword ptr [ecx]
	}
#else
	*s = (float)sin(a);
	*c = (float)cos(a);
#endif
}

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

