/*
Copyright (C) 1997-2001 Id Software, Inc.

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
//Decals, base code from EGL rewrite to q2xp specifics

#include "client.h"

decals_t *active_decals, *free_decals;
decals_t decals[MAX_DECALS];
int cl_maxDecals = MAX_DECALS;

int r_numDecals;


float Clamp_Value(float value, float min, float max)
{
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}

/*
=================
CL_ClearDecals
=================
*/

void CL_ClearDecals(void)
{
	int i;

	r_numDecals = 0;

	free_decals = &decals[0];
	active_decals = NULL;

	for (i = 0; i < cl_maxDecals; i++)
		decals[i].next = &decals[i + 1];

	decals[cl_maxDecals - 1].next = NULL;

}


/*
===============
CL_AddDecalToScene
===============
*/

void CL_AddDecalToScene(vec3_t origin, vec3_t dir,
						float red, float green, float blue, float alpha,
						float endRed, float endGreen, float endBlue,
						float endAlpha, float size,
						float endTime, int type, int flags, float angle,
						int blendD, int blendS)
{
	int i, j, numfragments;
	vec3_t verts[MAX_DECAL_VERTS];
	fragment_t *fr, fragments[MAX_DECAL_FRAGMENTS];
	mat3_t axis;
	decals_t *d = NULL;
	int nv;
	float *v;

	if (!cl_decals->value)
		return;

	// invalid decal
	if (size <= 0 || VectorCompare(dir, vec3_origin))
		return;

	// calculate orientation matrix
	VectorNormalize2(dir, axis[0]);
	PerpendicularVector(axis[1], axis[0]);
	RotatePointAroundVector(axis[2], axis[0], axis[1], angle);
	CrossProduct(axis[0], axis[2], axis[1]);

	numfragments = R_GetClippedFragments(origin, size, axis,	// clip it
										 MAX_DECAL_VERTS, verts,
										 MAX_DECAL_FRAGMENTS, fragments);

	// no valid fragments
	if (!numfragments)
		return;

	VectorScale(axis[1], 0.5f / size, axis[1]);
	VectorScale(axis[2], 0.5f / size, axis[2]);

	for (i = 0, fr = fragments; i < numfragments; i++, fr++) {
	
	
		if (fr->numverts > MAX_DECAL_VERTS)
			fr->numverts = MAX_DECAL_VERTS;
		else if (fr->numverts <= 0)
			continue;


		if (!free_decals)
			break;
		
		nv = fr->surf->polys->numverts;
		v = fr->surf->polys->verts[0];

		d = free_decals;
		free_decals = d->next;
		d->next = active_decals;
		active_decals = d;
		
		d->numverts = fr->numverts;
		d->node = fr->node;
		
	
		
		VectorCopy(fr->surf->plane->normal, d->direction);
		// reverse direction
		if (!(fr->surf->flags & SURF_PLANEBACK)) {
			VectorNegate(d->direction, d->direction);

		}
		VectorCopy(origin, d->org);

		VectorSet(d->color, red, green, blue);
		VectorSet(d->endColor, endRed, endGreen, endBlue);

		d->time = cl.time;
		d->endTime = cl.time + endTime;

		d->alpha = alpha;
		d->endAlpha = endAlpha;
		d->size = size;
		d->type = type;
		d->flags = flags;
		d->blendD = blendD;
		d->blendS = blendS;
		
		for (j = 0; j < fr->numverts; j++) {
			vec3_t v;

			VectorCopy(verts[fr->firstvert + j], d->verts[j]);
			VectorSubtract(d->verts[j], origin, v);
			d->stcoords[j][0] = DotProduct(v, axis[1]) + 0.5f;
			d->stcoords[j][1] = DotProduct(v, axis[2]) + 0.5f;

		}


	}
}



void AddDecals(void)
{
	decals_t *d, *next;
	vec3_t color, temp, org, dir;
	decals_t *active, *tail;
	float alpha;
	float time;
	float endLerp;
	float size;
	int i, blendD, blendS;


	active = NULL;
	tail = NULL;

	for (d = active_decals; d; d = next) {
		next = d->next;

		endLerp = (float) (cl.time - d->time) / (float) (d->endTime - d->time);

		time = (cl.time - d->time) * 0.001;
		alpha = d->alpha + (d->endAlpha - d->alpha) * endLerp;

		if (d->endTime <= cl.time) {
			d->next = free_decals;
			free_decals = d;
			continue;
		}

		if (alpha <= 0) {		// faded out
			d->next = free_decals;
			free_decals = d;
			continue;
		}
		
		d->next = NULL;
		if (!tail)
			active = tail = d;
		else {
			tail->next = d;
			tail = d;
		}


		if (alpha > 1.0)
			alpha = 1;

		for (i = 0; i < 3; i++)
			color[i] = d->color[i] + (d->endColor[i] - d->color[i]) * endLerp;

		blendD = d->blendD;
		blendS = d->blendS;
		
		size = d->size;
		VectorCopy(d->org, org);
		VectorCopy(d->direction, dir);

		VectorSubtract (cl.refdef.vieworg, d->org, temp);
		
		if (DotProduct(temp, temp)/15000 > 100*d->size)
			goto nextDecal;

		V_AddDecal(d->stcoords[0], d->verts[0], d->numverts, d->node,
				   color, alpha, d->type, d->flags, d->blendD,
				   d->blendS, org, dir, size);


	nextDecal:
		for (i = 0; i < 3; i++)
			color[i] = 0;
		alpha = 0;

	}
	active_decals = active;
	
}
