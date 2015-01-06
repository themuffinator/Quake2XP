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
// r_light.c

#include "r_local.h"

int r_dlightframecount;

/*
=============================================================================

LIGHT SAMPLING

=============================================================================
*/

vec3_t pointcolor;
cplane_t *lightplane;			// used as shadow plane
vec3_t lightspot;

int RecursiveLightPoint(mnode_t * node, vec3_t start, vec3_t end)
{
	float front, back, frac;
	int side;
	cplane_t *plane;
	vec3_t mid;
	msurface_t *surf;
	int s, t, ds, dt;
	int i;
	mtexInfo_t *tex;
	byte *lightmap;
	int maps;
	int r;

	if (node->contents != -1)
		return -1;				// didn't hit anything

// calculate mid point

// FIXME: optimize for axial

	plane = node->plane;
	front = DotProduct(start, plane->normal) - plane->dist;
	back = DotProduct(end, plane->normal) - plane->dist;
	side = front < 0;

	if ((back < 0) == side)
		return RecursiveLightPoint(node->children[side], start, end);


	frac = front / (front - back);
	mid[0] = start[0] + (end[0] - start[0]) * frac;
	mid[1] = start[1] + (end[1] - start[1]) * frac;
	mid[2] = start[2] + (end[2] - start[2]) * frac;

// go down front side   
	r = RecursiveLightPoint(node->children[side], start, mid);
	if (r >= 0)
		return r;				// hit something

	if ((back < 0) == side)
		return -1;				// didn't hit anuthing

// check for impact on this node
	VectorCopy(mid, lightspot);
	lightplane = plane;

	surf = r_worldmodel->surfaces + node->firstsurface;

	for (i = 0; i < node->numsurfaces; i++, surf++) {
		if (surf->flags & (SURF_DRAWTURB | SURF_DRAWSKY))
			continue;			// no lightmaps

		tex = surf->texInfo;

		s = DotProduct(mid, tex->vecs[0]) + tex->vecs[0][3];
		t = DotProduct(mid, tex->vecs[1]) + tex->vecs[1][3];;

		if (s < surf->texturemins[0] || t < surf->texturemins[1])
			continue;

		ds = s - surf->texturemins[0];
		dt = t - surf->texturemins[1];

		if (ds > surf->extents[0] || dt > surf->extents[1])
			continue;

		if (!surf->samples)
			return 0;

		ds /= r_worldmodel->lightmap_scale;
		dt /= r_worldmodel->lightmap_scale;

		lightmap = surf->samples;
		VectorCopy(vec3_origin, pointcolor);
		if (lightmap) {
			vec3_t scale;

			lightmap += 3 * (dt * ((surf->extents[0] / r_worldmodel->lightmap_scale) + 1) + ds);
			
			for (maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255; maps++) {
				
				for (i = 0; i < 3; i++)
					scale[i] = r_newrefdef.lightstyles[surf->styles[maps]].rgb[i];

				pointcolor[0] +=
					lightmap[0] * scale[0] *
					0.003921568627450980392156862745098 * r_ambientLevel->value;
				pointcolor[1] +=
					lightmap[1] * scale[1] *
					0.003921568627450980392156862745098 * r_ambientLevel->value;
				pointcolor[2] +=
					lightmap[2] * scale[2] *
					0.003921568627450980392156862745098 * r_ambientLevel->value;
					lightmap +=
					3 * ((surf->extents[0] / r_worldmodel->lightmap_scale) +
						 1) * ((surf->extents[1] / r_worldmodel->lightmap_scale) + 1);
			}
		}

		return 1;
	}

// go down back side
	return RecursiveLightPoint(node->children[!side], mid, end);
}

/*
===============
R_LightPoint
===============
*/

void R_LightPoint(vec3_t p, vec3_t color)
{
	vec3_t end;
	float r;
	int i;

	if ((r_worldmodel && !r_worldmodel->lightData) || !r_worldmodel)
	{
          color[0] = color[1] = color[2] = 1.0;
          return;
     }
	
	if (!r_newrefdef.areabits)
		return;

	end[0] = p[0];
	end[1] = p[1];
	end[2] = p[2] - 8192;

	r = RecursiveLightPoint(r_worldmodel->nodes, p, end);

	if (r == -1) {
		VectorCopy(vec3_origin, color);
	} else {
		VectorCopy(pointcolor, color);
	}

	// this catches too bright modulated color
	for (i = 0; i < 3; i++)
		if (color[i] > 1)
			color[i] = 1;
}


#define LIGHTGRID_STEP 128
#define LIGHTGRID_NUM_STEPS (8192/LIGHTGRID_STEP)	// 64

byte r_lightgrid[LIGHTGRID_NUM_STEPS * LIGHTGRID_NUM_STEPS *
				 LIGHTGRID_NUM_STEPS][3];


void R_InitLightgrid(void)
{
	int i, x, y, z;
	vec3_t p, end;
	float r;
	byte *b;

	memset(r_lightgrid, 0, sizeof(r_lightgrid));

	if (!r_worldmodel->lightData)
		return;

	b = &r_lightgrid[0][0];
	// Huh ?
	for (x = 0; x < 8192; x += LIGHTGRID_STEP)
		for (y = 0; y < 8192; y += LIGHTGRID_STEP)
			for (z = 0; z < 8192; z += LIGHTGRID_STEP) {
				end[0] = p[0] = x - 4096;
				end[1] = p[1] = y - 4096;
				end[2] = (p[2] = z - 4096) - 2048;
				r = RecursiveLightPoint(r_worldmodel->nodes, p, end);
				if (r != -1) {
					for (i = 0; i < 3; i++) {
						float mu = pointcolor[i];
						float f = mu * (2 * (1 - mu) + mu);
						*b++ = 255.0 * f;
					}
				} else {
					b += 3;
				}
			}
}

//___________________________________________________________
void R_LightColor(vec3_t org, vec3_t color)
{
	byte *b[8];
	int i;
	float f;
	float x = (4096 + org[0]) / LIGHTGRID_STEP;
	float y = (4096 + org[1]) / LIGHTGRID_STEP;
	float z = (4096 + org[2]) / LIGHTGRID_STEP;
	int s = x;
	int t = y;
	int r = z;

	x = x - s;
	y = y - t;
	z = z - r;

	b[0] =
		&r_lightgrid[(((s * LIGHTGRID_NUM_STEPS) +
					   t) * LIGHTGRID_NUM_STEPS) + r][0];
	b[1] =
		&r_lightgrid[(((s * LIGHTGRID_NUM_STEPS) +
					   t) * LIGHTGRID_NUM_STEPS) + r + 1][0];
	b[2] =
		&r_lightgrid[(((s * LIGHTGRID_NUM_STEPS) + t +
					   1) * LIGHTGRID_NUM_STEPS) + r][0];
	b[3] =
		&r_lightgrid[(((s * LIGHTGRID_NUM_STEPS) + t +
					   1) * LIGHTGRID_NUM_STEPS) + r + 1][0];
	b[4] =
		&r_lightgrid[((((s + 1) * LIGHTGRID_NUM_STEPS) +
					   t) * LIGHTGRID_NUM_STEPS) + r][0];
	b[5] =
		&r_lightgrid[((((s + 1) * LIGHTGRID_NUM_STEPS) +
					   t) * LIGHTGRID_NUM_STEPS) + r + 1][0];
	b[6] =
		&r_lightgrid[((((s + 1) * LIGHTGRID_NUM_STEPS) + t +
					   1) * LIGHTGRID_NUM_STEPS) + r][0];
	b[7] =
		&r_lightgrid[((((s + 1) * LIGHTGRID_NUM_STEPS) + t +
					   1) * LIGHTGRID_NUM_STEPS) + r + 1][0];

	f = ((float) 4 / (float) (255));

	if (!(b[0][0] && b[0][1] && b[0][2]))
		f -= (1.0 / (510.0)) * (1 - x) * (1 - y) * (1 - z);
	if (!(b[1][0] && b[1][1] && b[1][2]))
		f -= (1.0 / (510.0)) * (1 - x) * (1 - y) * (z);
	if (!(b[2][0] && b[2][1] && b[2][2]))
		f -= (1.0 / (510.0)) * (1 - x) * (y) * (1 - z);
	if (!(b[3][0] && b[3][1] && b[3][2]))
		f -= (1.0 / (510.0)) * (1 - x) * (y) * (z);
	if (!(b[4][0] && b[4][1] && b[4][2]))
		f -= (1.0 / (510.0)) * (x) * (1 - y) * (1 - z);
	if (!(b[5][0] && b[5][1] && b[5][2]))
		f -= (1.0 / (510.0)) * (x) * (1 - y) * (z);
	if (!(b[6][0] && b[6][1] && b[6][2]))
		f -= (1.0 / (510.0)) * (x) * (y) * (1 - z);
	if (!(b[7][0] && b[7][1] && b[7][2]))
		f -= (1.0 / (510.0)) * (x) * (y) * (z);

	for (i = 0; i < 3; i++) {	// hahaha slap me silly
		color[i] =
			(1 - x) * ((1 - y) * ((1 - z) * b[0][i] + (z) * b[1][i]) +
		y * ((1 - z) * b[2][i] + (z) * b[3][i]))
	  + x * ((1 - y) * ((1 - z) * b[4][i] + (z) * b[5][i]) +
	    y * ((1 - z) * b[6][i] + (z) * b[7][i]));
		color[i] *= f;

	}
}


#define LIGHTGRID_STEP_PPL 64
#define LIGHTGRID_NUM_STEPS_PPL (8192/LIGHTGRID_STEP_PPL)	// 128
byte r_lightgridPpl[LIGHTGRID_NUM_STEPS_PPL * LIGHTGRID_NUM_STEPS_PPL *
LIGHTGRID_NUM_STEPS_PPL][3];
//___________________________________________________________

void R_InitLightgrid2(void)
{
	worldShadowLight_t *light;
	trace_t trace;
	int i, x, y, z;
	vec3_t point, dist;
	byte *b;
	float add;

	memset(r_lightgridPpl, 0, sizeof(r_lightgridPpl));

	for (light = shadowLight_static; light; light = light->s_next) {
		
		float tmp, color;
		float rad = (light->radius[0] + light->radius[1], + light->radius[2]) / 3;

		VectorSubtract(point, light->origin, dist);
		tmp = ((rad * 2) - VectorLength(dist)) / (rad * 2);
		add = tmp * (0.2 + (0.8 * tmp));

		b = &r_lightgridPpl[0][0];

		// Huh ?
		for (x = 0; x < 8192; x += LIGHTGRID_STEP_PPL)
			for (y = 0; y < 8192; y += LIGHTGRID_STEP_PPL)
				for (z = 0; z < 8192; z += LIGHTGRID_STEP_PPL) {

			point[0] = x - 4096;
			point[1] = y - 4096;
			point[2] = z - 4096;
			
			if (CL_PMpointcontents(point) & MASK_SOLID)
				goto next;

			if (r_newrefdef.areabits){
				trace = CM_BoxTrace(light->origin, point, vec3_origin, vec3_origin, r_worldmodel->firstNode, MASK_OPAQUE);

				if (trace.fraction != 1.0)
					goto next;
			}

			if (BoundsIntersectsPoint(light->mins, light->maxs, point)) {
				
				for (i = 0; i < 3; i++) {
					
					color = light->startColor[i];

					if (add > 0.01)
						color = light->startColor[i] + add * light->startColor[i];

					color *= 255.0;
					color += *b;
					if (color > 255)
						color = 255;
					*b++ = color;
				}
			}
			else {
				next:
				b += 3;
			}
		}
	}
}
void R_AmbientColor(vec3_t org, vec3_t color)
{
	byte *b[8];
	int i;
	float f;
	float x = (4096 + org[0]) / LIGHTGRID_STEP_PPL;
	float y = (4096 + org[1]) / LIGHTGRID_STEP_PPL;
	float z = (4096 + org[2]) / LIGHTGRID_STEP_PPL;
	int s = x;
	int t = y;
	int r = z;

	x = x - s;
	y = y - t;
	z = z - r;

	b[0] =
		&r_lightgridPpl[(((s * LIGHTGRID_NUM_STEPS_PPL) +
		t) * LIGHTGRID_NUM_STEPS_PPL) + r][0];
	b[1] =
		&r_lightgridPpl[(((s * LIGHTGRID_NUM_STEPS_PPL) +
		t) * LIGHTGRID_NUM_STEPS_PPL) + r + 1][0];
	b[2] =
		&r_lightgridPpl[(((s * LIGHTGRID_NUM_STEPS_PPL) + t +
		1) * LIGHTGRID_NUM_STEPS_PPL) + r][0];
	b[3] =
		&r_lightgridPpl[(((s * LIGHTGRID_NUM_STEPS_PPL) + t +
		1) * LIGHTGRID_NUM_STEPS_PPL) + r + 1][0];
	b[4] =
		&r_lightgridPpl[((((s + 1) * LIGHTGRID_NUM_STEPS_PPL) +
		t) * LIGHTGRID_NUM_STEPS_PPL) + r][0];
	b[5] =
		&r_lightgridPpl[((((s + 1) * LIGHTGRID_NUM_STEPS_PPL) +
		t) * LIGHTGRID_NUM_STEPS_PPL) + r + 1][0];
	b[6] =
		&r_lightgridPpl[((((s + 1) * LIGHTGRID_NUM_STEPS_PPL) + t +
		1) * LIGHTGRID_NUM_STEPS_PPL) + r][0];
	b[7] =
		&r_lightgridPpl[((((s + 1) * LIGHTGRID_NUM_STEPS_PPL) + t +
		1) * LIGHTGRID_NUM_STEPS_PPL) + r + 1][0];

	f = ((float)4 / (float)(255));

	if (!(b[0][0] && b[0][1] && b[0][2]))
		f -= (1.0 / (510.0)) * (1 - x) * (1 - y) * (1 - z);
	if (!(b[1][0] && b[1][1] && b[1][2]))
		f -= (1.0 / (510.0)) * (1 - x) * (1 - y) * (z);
	if (!(b[2][0] && b[2][1] && b[2][2]))
		f -= (1.0 / (510.0)) * (1 - x) * (y)* (1 - z);
	if (!(b[3][0] && b[3][1] && b[3][2]))
		f -= (1.0 / (510.0)) * (1 - x) * (y)* (z);
	if (!(b[4][0] && b[4][1] && b[4][2]))
		f -= (1.0 / (510.0)) * (x)* (1 - y) * (1 - z);
	if (!(b[5][0] && b[5][1] && b[5][2]))
		f -= (1.0 / (510.0)) * (x)* (1 - y) * (z);
	if (!(b[6][0] && b[6][1] && b[6][2]))
		f -= (1.0 / (510.0)) * (x)* (y)* (1 - z);
	if (!(b[7][0] && b[7][1] && b[7][2]))
		f -= (1.0 / (510.0)) * (x)* (y)* (z);

	for (i = 0; i < 3; i++) {	// hahaha slap me silly
		color[i] =
			(1 - x) * ((1 - y) * ((1 - z) * b[0][i] + (z)* b[1][i]) +
			y * ((1 - z) * b[2][i] + (z)* b[3][i]))
			+ x * ((1 - y) * ((1 - z) * b[4][i] + (z)* b[5][i]) +
			y * ((1 - z) * b[6][i] + (z)* b[7][i]));
		color[i] *= f;
		
		clamp(color[i], 0.0, 1.0);

	}

}