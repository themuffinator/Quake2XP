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

#include "cmdlib.h"
#include "mathlib.h"
#include "bspfile.h"
#include "polylib.h"
#include "threads.h"
#include "lbmlib.h"
#include "xplit.h"
#include "xplm.h"
#include "lightmap.h"
#include "sh.h"

#ifdef WIN32
#include <windows.h>
#endif

#define	MAX_PATCHES	65000			// larger will cause 32 bit overflows

// the sum of all transfer->transfer[0] values (for vanilla)
// or sum[i=0 to 3](transfer->transfer[i] * xplm_basisVecs[i][2]) values (for XP)
// for a given patch should equal exactly 0x10000,
// showing that all radiance reaches other patches
typedef struct {
	unsigned short	patch;
	unsigned short	transfer[XPLM_NUMVECS];
} transfer_t;

typedef struct patch_s {
	struct patch_s		*next;		// next in face
	winding_t	*winding;

	int			numtransfers;
	transfer_t	*transfers;

	int			cluster;			// for pvs checking
	vec3_t		origin;

	dplane_t	*plane;
	vec3_t		tangents[2];				// for XPLM

	float		area;

	// accumulated by radiosity
	// does NOT include light accounted for by direct lighting
	vec3_t			totallight[3];

	// illuminance * reflectivity = radiosity
	vec3_t			reflectivity;

	// emissivity only
	vec3_t			baselight;

	// each style 0 lightmap sample in the patch will be
	// added up to get the average illuminance of the entire patch
	vec3_t			samplelight;

	int			samples;		// for averaging direct light

	qboolean	sky;
} patch_t;

extern	patch_t		*face_patches[MAX_MAP_FACES];
extern	entity_t	*face_entity[MAX_MAP_FACES];
extern	vec3_t		face_offset[MAX_MAP_FACES];		// for rotating bmodels
extern	patch_t		patches[MAX_PATCHES];
extern	unsigned	num_patches;

extern	float	lightscale;

extern int	lightmap_scale;
extern qboolean deluxeMapping;

void MakeShadowSplits (void);

//==============================================


void BuildVisMatrix (void);
qboolean CheckVisBit (unsigned p1, unsigned p2);

//==============================================

extern	float ambient, maxlight;

void LinkPlaneFaces (void);

extern qboolean	extrasamples;
extern int extrasamplesvalue;

extern int numbounce;

extern qboolean	qrad_xplit;
extern qboolean	qrad_xplm;
extern int qrad_numBasisVecs;
extern int qrad_dlMode;

extern	byte	nodehit[MAX_MAP_NODES];

void BuildLightmaps (void);

void BuildFacelights (int facenum);

void FinalLightFace (int facenum);

qboolean PvsForOrigin (const vec3_t org, byte *pvs);

int TestLine_r (int node, const vec3_t start, const vec3_t stop);

void CreateDirectLights (void);

dleaf_t		*PointInLeaf (const vec3_t point);


extern dplane_t	backplanes[MAX_MAP_PLANES];
extern int			fakeplanes;					// created planes for origin offset 

extern float	subdiv;
extern qboolean	nopvs;

extern float	direct_scale;
extern float	entity_scale;

// for MakeTransfers
extern int	total_transfer;

void MakeBackplanes (void);
void MakeParents (int nodenum, int parent);
int PointInLeafnum (const vec3_t point);
void MakeTnodes (dmodel_t *bm);
void MakePatches (void);
void SubdividePatches (void);
void PairEdges (void);
void CalcTextureReflectivity (void);
