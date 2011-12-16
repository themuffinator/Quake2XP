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

#define	DLIGHT_CUTOFF	0

/*
 =================
 BoundsAndSphereIntersect
 =================
*/
qboolean BoundsAndSphereIntersect (const vec3_t mins, const vec3_t maxs, const vec3_t origin, float radius){

	if (mins[0] > origin[0] + radius || mins[1] > origin[1] + radius || mins[2] > origin[2] + radius)
		return false;
	if (maxs[0] < origin[0] - radius || maxs[1] < origin[1] - radius || maxs[2] < origin[2] - radius)
		return false;

	return true;
}

/*
=============
R_MarkLights
=============
*/

void R_MarkLights(dlight_t * light, int bit, mnode_t * node)
{
	cplane_t *splitplane;
	float dist;
	msurface_t *surf;
	int i, sidebit;

	if (node->contents != -1)
		return;

	splitplane = node->plane;
	dist =
		DotProduct(light->origin, splitplane->normal) - splitplane->dist;

	if (dist > light->intensity - DLIGHT_CUTOFF) {
		R_MarkLights(light, bit, node->children[0]);
		return;
	}
	if (dist < -light->intensity + DLIGHT_CUTOFF) {
		R_MarkLights(light, bit, node->children[1]);
		return;
	}
// mark the polygons
	surf = r_worldmodel->surfaces + node->firstsurface;
	for (i = 0; i < node->numsurfaces; i++, surf++) {
	
	if (!BoundsAndSphereIntersect(surf->mins, surf->maxs, light->origin, light->intensity))
			continue;		// No intersection


		dist = DotProduct(light->origin, surf->plane->normal) - surf->plane->dist;
		if (dist >= 0)
			sidebit = 0;
		else
		sidebit = SURF_PLANEBACK;

		if ((surf->flags & SURF_PLANEBACK) != sidebit)	// Discoloda
			continue;			

		if (surf->dlightframe != r_dlightframecount) {
			surf->dlightbits = 0;
			surf->dlightframe = r_dlightframecount;
		}
		surf->dlightbits |= bit;
				
	}

	R_MarkLights(light, bit, node->children[0]);
	R_MarkLights(light, bit, node->children[1]);

}




/*
=============
R_PushDlights
=============
*/
void R_PushDlights(void)
{
	int i;
	dlight_t *l;

	r_dlightframecount = r_framecount + 1;	// because the count hasn't
	// advanced yet for this frame
	l = r_newrefdef.dlights;
	for (i = 0; i < r_newrefdef.num_dlights; i++, l++)
		R_MarkLights(l, 1 << i, r_worldmodel->nodes);
}


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
	mtexinfo_t *tex;
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

		tex = surf->texinfo;

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
			for (maps = 0;
				 maps < MAXLIGHTMAPS && surf->styles[maps] != 255;
				 maps++) {
				for (i = 0; i < 3; i++)
					scale[i] = r_newrefdef.lightstyles[surf->styles[maps]].rgb[i];

				pointcolor[0] +=
					lightmap[0] * scale[0] *
					0.003921568627450980392156862745098;
				pointcolor[1] +=
					lightmap[1] * scale[1] *
					0.003921568627450980392156862745098;
				pointcolor[2] +=
					lightmap[2] * scale[2] *
					0.003921568627450980392156862745098;
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

void R_LightPoint(vec3_t p, vec3_t color, qboolean bump)
{
	vec3_t end;
	float r;
	int i;
	dlight_t *dl;
	vec3_t dir;
	float add, dst;
	trace_t trace;

	if ((r_worldmodel && !r_worldmodel->lightdata) || !r_worldmodel)
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
	
	if(!bump){
	
	dl = r_newrefdef.dlights;
	for (i=0; i<r_newrefdef.num_dlights; i++, dl++){
	
	VectorSubtract(dl->origin, p, dir);
	dst = VectorLength(dir);
	
	if (!dst || dst > dl->intensity)
		continue;

	// dlight behind the wall
	if (r_newrefdef.areabits){
			trace = CM_BoxTrace(dl->origin, p, vec3_origin, vec3_origin, r_worldmodel->firstnode, MASK_OPAQUE);
			if(trace.fraction != 1.0)
			continue;
	}

	add = (dl->intensity - dst) * (1.0/255);
	VectorMA(color, add, dl->color, color);
	}
	}
}


#define LIGHTGRID_STEP 128
#define LIGHTGRID_NUM_STEPS (8192/LIGHTGRID_STEP)	// 64

byte r_lightgrid[LIGHTGRID_NUM_STEPS * LIGHTGRID_NUM_STEPS *
				 LIGHTGRID_NUM_STEPS][3];

//___________________________________________________________
void R_InitLightgrid(void)
{
	int i, x, y, z;
	vec3_t p, end;
	float r;
	byte *b;

	memset(r_lightgrid, 0, sizeof(r_lightgrid));

	if (!r_worldmodel->lightdata)
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
	int i, lnum;
	dlight_t *dl;
	float f, light, add;
	vec3_t dist;
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

		if(color[i] <= 0.35)
			color[i] = 0.35;


	}

	// add dynamic light
	light = 0;
	dl = r_newrefdef.dlights;
	for (lnum = 0; lnum < r_newrefdef.num_dlights; lnum++, dl++) {
		VectorSubtract(org, dl->origin, dist);
		f = ((dl->intensity * 2) -
			 VectorLength(dist)) / (dl->intensity * 2);
		if (f <= 0)
			continue;
		add = f * (0.2 + (0.8 * f));
		if (add > 0.01) {
			VectorMA(color, add, dl->color, color);
		}
	}
}


shadowlight_t* AllocShadowLight ()
{
	if (numShadowLights >= MAXSHADOWLIGHTS)
		return NULL;

	numShadowLights++;
	return &shadowlights[numShadowLights-1];
}


void R_SpawnSLight(shadowlight_t *sl)
{
	int				i;
	shadowlight_t	*l;

	l = AllocShadowLight();
	if (!l)
		return;

	VectorCopy(sl->color, l->color);
	VectorCopy(sl->origin, l->origin);
	l->radius = sl->radius;
	l->style = sl->style;
	VectorCopy(sl->angles, l->angles);
	l->isStatic = true;
	numStaticShadowLights++;
}

qboolean SpawnStaticLight (shadowlight_t *sl)
{
	if (numStaticShadowLights >= MAXSHADOWLIGHTS)
	{
		Com_DPrintf ("SpawnStaticLight: overflow\n");
		return false;
	}

	if (sl->radius == 1.0)
		return false;

	//Create a light and make it static
	R_SpawnSLight(sl);
///	numStaticShadowLights++;
	return true;
}

void R_ClearSLights()
{
	numShadowLights = 0;
	numStaticShadowLights = 0;
}

int CL_PMpointcontents(vec3_t point);

/*
===============
R_MarkLightLeaves

Marks nodes from the light, this is used for
gross culling during svbsp creation.
===============
*/
qboolean R_MarkLightLeaves ()
{
	int		contents, leafnum, cluster;
	int		leafs[MAX_MAP_LEAFS];
	int		i, count;
	vec3_t	mins, maxs;
	byte	vis[MAX_MAP_LEAFS/8];

	contents = CL_PMpointcontents(currentshadowlight->origin);
	if (contents & CONTENTS_SOLID)
		goto skip;

	leafnum = CM_PointLeafnum (currentshadowlight->origin);
	cluster = CM_LeafCluster (leafnum);
	currentshadowlight->area = CM_LeafArea (leafnum);

	if(!currentshadowlight->area)
	{
skip:	Com_DPrintf("Out of BSP, rejected light at %f %f %f\n", currentshadowlight->origin[0], currentshadowlight->origin[1], currentshadowlight->origin[2]);
		return false;
	}

	// строим vis-data
	memcpy (&currentshadowlight->vis, CM_ClusterPVS(cluster), (((CM_NumClusters()+31)>>5)<<2));

	for (i=0 ; i<3 ; i++)
	{
		mins[i] = currentshadowlight->origin[i] - currentshadowlight->radius;
		maxs[i] = currentshadowlight->origin[i] + currentshadowlight->radius;
	}

	count = CM_BoxLeafnums (mins, maxs, leafs, r_worldmodel->numleafs/*MAX_MAP_LEAFS*/, NULL);
	if (count < 1)
		Com_Error (ERR_FATAL, "R_MarkLightLeaves: count < 1");
///	longs = (CM_NumClusters()+31)>>5;

	// convert leafs to clusters
	for (i=0 ; i<count ; i++)
		leafs[i] = CM_LeafCluster(leafs[i]);

	memset(&vis, 0, (((r_worldmodel->numleafs+31)>>5)<<2)/*MAX_MAP_LEAFS/8*/);
	for (i=0 ; i<count ; i++)
		vis[leafs[i]>>3] |= (1<<(leafs[i]&7));
	// вырезаем кластеры, которых нет в списке leafs
	for (i=0 ; i<((r_worldmodel->numleafs+31)>>5)/*MAX_MAP_LEAFS/32*/ ; i++)
		((long *)currentshadowlight->vis)[i] &= ((long *)vis)[i];

	return true;
}

/// from Tenebrae, asm by Berserker
qboolean HasSharedLeafs(byte *v1, byte *v2)
{

	int numleafs__ = r_worldmodel->numleafs;
	_asm
	{
		mov edx, numleafs__
		mov esi, v1
		mov edi, v2
		shr edx, 5
		jz short l4
l0:		mov eax, [esi]
		add esi, 4
		test eax, [edi]
		jnz short l3
		add edi, 4
		dec edx
		jnz short l0
l4:		mov edx, numleafs__
		mov esi, v1
		and edx, ~0x1f
		mov edi, v2
		cmp edx, numleafs__
		jz short l5
l1:		mov eax, edx
		mov ecx, edx
		mov ebx, 1
		and ecx, 7
		shr eax, 3
		shl ebx, cl
		test byte ptr [esi+eax], bl
		jz short l2
		test byte ptr [edi+eax], bl
		jnz short l3
l2:		inc edx
		cmp edx, numleafs__
		jc short l1
	}
l5:	return false;
	_asm
	{
l3:
	}
	return true;

}

float SphereInFrustum(vec3_t o, float radius);

/*
=============
R_ContributeFrame

Returns true if the light should be rendered.
=============
*/
qboolean R_ContributeFrame ()
{
	
	
		//frustum scissor testing
		float d = SphereInFrustum(currentshadowlight->origin, currentshadowlight->radius);
		if (d == 0)
			return false;	//whole sphere is out ouf frustum so cut it.
	

	if(!currentshadowlight->isStatic)
		if(!R_MarkLightLeaves())
			return false;

	//not in a visible area => skip it
	if (!(r_newrefdef.areabits[currentshadowlight->area>>3] & (1<<(currentshadowlight->area&7))))
		return false;
		return false;	/// Это вызывает баг при присоединении к удаленному серверу!

	if (!HasSharedLeafs (currentshadowlight->vis, viewvis))
		return false;



	return true;
}

void R_SpawnDlight(dlight_t *dl)
{
	int				i;
	shadowlight_t	*l;

	l = AllocShadowLight();
	if (!l)
		return;

	VectorCopy(dl->color, l->color);
	VectorCopy(dl->origin, l->origin);
	l->radius =  dl->intensity;
	VectorCopy(dl->angles, l->angles);
	l->isStatic = false;

	for (i=0 ; i<3 ; i++)
	{
		l->mins[i] = -l->radius;
		l->maxs[i] = l->radius;
	}

}


/*
=============
R_MarkDLights

Adds dynamic lights to the shadow light list
=============
*/
void R_AddDlights ()
{
	int		i;
	dlight_t	*l;

	l = r_newrefdef.dlights;
	for (i=0 ; i<r_newrefdef.num_dlights ; i++, l++)
	{
		R_SpawnDlight(l);
	}
}



/*
=============
R_InitShadowsForFrame

Do per frame intitialization for the shadows
=============
*/
void R_InitShadowsForFrame ()
{
	int		i;
	vec3_t	temp;

	numShadowLights = numStaticShadowLights;
	R_AddDlights (); //add dynamic lights to the list
	numUsedShadowLights = 0;

	for (i=0; i<numShadowLights; i++)
	{
		currentshadowlight = &shadowlights[i];
		currentshadowlight->visible = false;

		if(currentshadowlight->isStatic)
			currentshadowlight->visible = false;
		else
		{
			if (R_ContributeFrame())
			{
				if (numUsedShadowLights < MAXUSEDSHADOWLIGHS)
				{
					currentshadowlight->visible = true;
					usedshadowlights[numUsedShadowLights] = currentshadowlight;
					numUsedShadowLights++;
				}
				
			}
		}
	}
}
