/*
===========================================================================
Copyright (C) 1997-2006 Id Software, Inc.

This file is part of Quake 2 Tools source code.

Quake 2 Tools source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake 2 Tools source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake 2 Tools source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#ifndef __MATHLIB__
#define __MATHLIB__

// mathlib.h

#include <math.h>

#ifdef DOUBLEVEC_T
typedef double vec_t;
#else
typedef float vec_t;
#endif
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];

#define	SIDE_FRONT		0
#define	SIDE_ON			2
#define	SIDE_BACK		1
#define	SIDE_CROSS		-2

#define	Q_PI	3.14159265358979323846

#ifndef min 
#define min(a,b)        (((a) < (b)) ? (a) : (b)) 
#endif 
#ifndef max 
#define max(a,b)        (((a) > (b)) ? (a) : (b)) 
#endif 

#define Q_clamp(a,b,c)	((a) < (b) ? (b) : (a) > (c) ? (c) : (a))

extern vec2_t vec2_origin;
extern vec3_t vec3_origin;
extern vec4_t vec4_origin;

#define	EQUAL_EPSILON	0.001

qboolean VectorCompare (vec3_t v1, vec3_t v2);

#define DotProduct(x,y)		((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define VectorSubtract(a,b,c)	((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2])
#define VectorAdd(a,b,c)	((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2])
#define VectorCopy(a,b)		((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])
#define VectorClear(a)		((a)[0]=(a)[1]=(a)[2]=0)
#define VectorNegate(a,b)	((b)[0]=-(a)[0],(b)[1]=-(a)[1],b[2]=-(a)[2])
#define VectorSet(v,x,y,z)	((v)[0]=(x), (v)[1]=(y), (v)[2]=(z))
#define VectorInvert(v)		((v)[0]=-(v)[0], (v)[1]=-(v)[1], (v)[2]=-(v)[2])
#define VectorScale(a,b,c)	((c)[0]=(a)[0]*(b), (c)[1]=(a)[1]*(b), (c)[2]=(a)[2]*(b))

#define VectorMA(a,b,c,d)	((d)[0]=(a)[0]+(c)[0]*(b),(d)[1]=(a)[1]+(c)[1]*(b),(d)[2]=(a)[2]+(c)[2]*(b))

//#define VectorMin(a,b,o)	((o)[0]=((a)[0]<(b)[0])?(a)[0]:(b)[0],(o)[1]=((a)[1]<(b)[1])?(a)[1]:(b)[1],(o)[2]=((a)[2]<(b)[2])?(a)[2]:(b)[2])
//#define VectorMax(a,b,o)	((o)[0]=((a)[0]>(b)[0])?(a)[0]:(b)[0],(o)[1]=((a)[1]>(b)[1])?(a)[1]:(b)[1],(o)[2]=((a)[2]>(b)[2])?(a)[2]:(b)[2])
//#define VectorMul(in1, in2, out)		((out)[0] = (in1)[0] * (in2)[0], (out)[1] = (in1)[1] * (in2)[1], (out)[2] = (in1)[2] * (in2)[2])
//#define VectorAverage(a,b,o)	((o)[0]=((a)[0]+(b)[0])*0.5f,(o)[1]=((a)[1]+(b)[1])*0.5f,(o)[2]=((a)[2]+(b)[2])*0.5f)

vec_t Q_rint (vec_t in);
vec_t _DotProduct (vec3_t v1, vec3_t v2);
void _VectorSubtract (vec3_t va, vec3_t vb, vec3_t out);
void _VectorAdd (vec3_t va, vec3_t vb, vec3_t out);
void _VectorCopy (vec3_t in, vec3_t out);
void _VectorScale (vec3_t v, vec_t scale, vec3_t out);

double VectorLength (vec3_t v);

void _VectorMA (vec3_t va, double scale, vec3_t vb, vec3_t vc);

void CrossProduct (vec3_t v1, vec3_t v2, vec3_t cross);
vec_t VectorNormalize (vec3_t in, vec3_t out);
vec_t ColorNormalize (vec3_t in, vec3_t out);
void VectorInverse (vec3_t v);

void ClearBounds (vec3_t mins, vec3_t maxs);
void AddPointToBounds (vec3_t v, vec3_t mins, vec3_t maxs);

#endif
