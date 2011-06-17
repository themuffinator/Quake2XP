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
=============================================================================

DYNAMIC LIGHTS BLEND RENDERING

=============================================================================
*/


void R_RenderDlight(dlight_t * light)
{
	int i, j;
	float a;
	vec3_t v;
	float rad;


	rad = light->intensity * 0.35;

	VectorSubtract(light->origin, r_origin, v);


	qglBegin(GL_TRIANGLE_FAN);
	qglColor3f(light->color[0] * 0.2, light->color[1] * 0.2,
			   light->color[2] * 0.2);
	for (i = 0; i < 3; i++)
		v[i] = light->origin[i] - vpn[i] * rad;
	qglVertex3fv(v);
	qglColor3f(0, 0, 0);
	for (i = 16; i >= 0; i--) {
		a = i * 0.39269875;
		for (j = 0; j < 3; j++)
			v[j] = light->origin[j] + vright[j] * cos(a) * rad
				+ vup[j] * sin(a) * rad;
		qglVertex3fv(v);
	}
	qglEnd();
}

/*
=============
R_RenderDlights
=============
*/
void R_RenderDlights(void)
{
	int i;
	dlight_t *l;


	if (!r_flashBlend->value)
		return;

	r_dlightframecount = r_framecount + 1;	// because the count hasn't
	// advanced yet for this frame
	qglDepthMask(0);
	qglDisable(GL_TEXTURE_2D);
	qglShadeModel(GL_SMOOTH);
	GL_Blend(true, GL_ONE, GL_ONE);

	l = r_newrefdef.dlights;
	for (i = 0; i < r_newrefdef.num_dlights; i++, l++)
		R_RenderDlight(l);

	qglColor3f(1, 1, 1);
	qglEnable(GL_TEXTURE_2D);
	GL_Blend(false, 0, 0);
	qglDepthMask(1);



}




/*
=============================================================================

DYNAMIC LIGHTS

=============================================================================
*/
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

	if (r_flashBlend->value)
		return;

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

	if (!TracePointForRender(p, NULL, NULL, dl->origin))
				continue;		// dlight behind the wall

	add = (dl->intensity - dst) * (1.0/255);
	VectorMA(color, add, dl->color, color);
	}
	}
}


static float s_blocklights[1024 * 1024 * 3];
/*
===============
R_AddDynamicLights
===============
*/
void R_AddDynamicLights(msurface_t * surf)
{
	int lnum;
	int sd, td;
	float fdist, frad, fminlight;
	vec3_t impact, local;
	int s, t;
	int i;
	int smax, tmax;
	mtexinfo_t *tex;
	dlight_t *dl;
	float *pfBL;
	float fsacc, ftacc;


	smax = (surf->extents[0] / r_worldmodel->lightmap_scale) + 1;
	tmax = (surf->extents[1] / r_worldmodel->lightmap_scale) + 1;
	tex = surf->texinfo;

	for (lnum = 0; lnum < r_newrefdef.num_dlights; lnum++) {
		if (!(surf->dlightbits & (1 << lnum)))
			continue;			// not lit by this light

		dl = &r_newrefdef.dlights[lnum];
		frad = dl->intensity;
		fdist = DotProduct(dl->origin, surf->plane->normal) -
			surf->plane->dist;
		frad -= fabs(fdist);
		// rad is now the highest intensity on the plane

		fminlight = DLIGHT_CUTOFF;	// FIXME: make configurable?
		if (frad < fminlight)
			continue;
		fminlight = frad - fminlight;

		for (i = 0; i < 3; i++) {
			impact[i] = dl->origin[i] - surf->plane->normal[i] * fdist;
		}

		local[0] =
			DotProduct(impact,
					   tex->vecs[0]) + tex->vecs[0][3] -
			surf->texturemins[0];
		local[1] =
			DotProduct(impact,
					   tex->vecs[1]) + tex->vecs[1][3] -
			surf->texturemins[1];

		pfBL = s_blocklights;
		for (t = 0, ftacc = 0; t < tmax; t++, ftacc += r_worldmodel->lightmap_scale) {
			td = local[1] - ftacc;
			if (td < 0)
				td = -td;

			for (s = 0, fsacc = 0; s < smax; s++, fsacc += r_worldmodel->lightmap_scale, pfBL += 3) {
				sd = Q_ftol(local[0] - fsacc);

				if (sd < 0)
					sd = -sd;

				if (sd > td)
					fdist = sd + (td >> 1);
				else
					fdist = td + (sd >> 1);

				if (fdist < fminlight) {
					pfBL[0] += (fminlight - fdist) * dl->color[0];
					pfBL[1] += (fminlight - fdist) * dl->color[1];
					pfBL[2] += (fminlight - fdist) * dl->color[2];
				}
			}
		}
	}
}


/*
** R_SetCacheState
*/
void R_SetCacheState(msurface_t * surf)
{
	int maps;

	for (maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255;
		 maps++) {
		surf->cached_light[maps] =
			r_newrefdef.lightstyles[surf->styles[maps]].white;
	}
}


/*
===============
R_BuildLightMap

Combine and scale multiple lightmaps into the floating format in blocklights
===============
*/
model_t *loadmodel;

void R_BuildLightMap(msurface_t * surf, byte * dest, int stride, qboolean loadModel)
{
	int smax, tmax;
	int r, g, b, a, max;
	int i, j, size;
	byte *lightmap;
	float scale[4];
	int nummaps;
	float *bl;
	lightstyle_t *style;
	
	if (surf->texinfo->
		flags & (SURF_SKY | SURF_TRANS33 | SURF_TRANS66 | SURF_WARP))
		VID_Error(ERR_DROP, "R_BuildLightMap called for non-lit surface");


	if(loadModel){
	smax = (surf->extents[0] / (int)loadmodel->lightmap_scale) + 1;
	tmax = (surf->extents[1] / (int)loadmodel->lightmap_scale) + 1;
	size = smax * tmax;
	if (size > (sizeof(s_blocklights) / (int)loadmodel->lightmap_scale))
		VID_Error(ERR_DROP, "Bad s_blocklights size");
	}
	else
	{
	smax = (surf->extents[0] / (int)r_worldmodel->lightmap_scale) + 1;
	tmax = (surf->extents[1] / (int)r_worldmodel->lightmap_scale) + 1;
	size = smax * tmax;
	if (size > (sizeof(s_blocklights) / (int)r_worldmodel->lightmap_scale))
		VID_Error(ERR_DROP, "Bad s_blocklights size");
	}
	
// set to full bright if no light data
	if (!surf->samples) {
		int maps;

		for (i = 0; i < size * 3; i++)
			s_blocklights[i] = 255;
		for (maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255;
			 maps++) {
			style = &r_newrefdef.lightstyles[surf->styles[maps]];
		}
		goto store;
	}
	// count the # of maps
	for (nummaps = 0;
		 nummaps < MAXLIGHTMAPS && surf->styles[nummaps] != 255;
		 nummaps++);

	lightmap = surf->samples;
	
	
	// add all the lightmaps
	if (nummaps == 1) {
		int maps;

		for (maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255;
			 maps++) {
			bl = s_blocklights;

			for (i = 0; i < 3; i++)
				scale[i] = r_newrefdef.lightstyles[surf->styles[maps]].rgb[i];

			if (scale[0] == 1.0F && scale[1] == 1.0F && scale[2] == 1.0F) {
				for (i = 0; i < size; i++, bl += 3) {
					bl[0] = lightmap[i * 3 + 0];
					bl[1] = lightmap[i * 3 + 1];
					bl[2] = lightmap[i * 3 + 2];
				}
			} else {
				for (i = 0; i < size; i++, bl += 3) {
					bl[0] = lightmap[i * 3 + 0] * scale[0];
					bl[1] = lightmap[i * 3 + 1] * scale[1];
					bl[2] = lightmap[i * 3 + 2] * scale[2];
				}
			}
			lightmap += size * 3;	// skip to next lightmap
		}

	} else {
		int maps;

		memset(s_blocklights, 0, sizeof(s_blocklights[0]) * size * 3);
		
		

		for (maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255;
			 maps++) {
			bl = s_blocklights;

			for (i = 0; i < 3; i++)
				scale[i] = r_newrefdef.lightstyles[surf->styles[maps]].rgb[i];

			if (scale[0] == 1.0F && scale[1] == 1.0F && scale[2] == 1.0F) {
				for (i = 0; i < size; i++, bl += 3) {
					bl[0] += lightmap[i * 3 + 0];
					bl[1] += lightmap[i * 3 + 1];
					bl[2] += lightmap[i * 3 + 2];
				}
			} else {
				for (i = 0; i < size; i++, bl += 3) {
					bl[0] += lightmap[i * 3 + 0] * scale[0];
					bl[1] += lightmap[i * 3 + 1] * scale[1];
					bl[2] += lightmap[i * 3 + 2] * scale[2];
				}
			}
			lightmap += size * 3;	// skip to next lightmap
		}
	
	}



// add all the dynamic lights
	if (surf->dlightframe == r_framecount)
		R_AddDynamicLights(surf);



// put into texture format
  store:
	stride -= (smax << 2);
	bl = s_blocklights;
		
	for (i = 0; i < tmax; i++, dest += stride) {
			for (j = 0; j < smax; j++) {

				/*
				r = Q_ftol(bl[0]);
				g = Q_ftol(bl[1]);
				b = Q_ftol(bl[2]);
				*/

				// swap color data to GL_BGRA for fast lightmaps upload
				r = Q_ftol(bl[2]);
				g = Q_ftol(bl[1]);
				b = Q_ftol(bl[0]);

				// catch negative lights
				if (r < 0)
					r = 0;
				if (g < 0)
					g = 0;
				if (b < 0)
					b = 0;

				/* 
				 ** determine the brightest of the three color components
				 */
				if (r > g)
					max = r;
				else
					max = g;
				if (b > max)
					max = b;

				/* 
				 ** alpha is ONLY used for the mono lightmap case.  For this reason
				 ** we set it to the brightest of the color components so that 
				 ** things don't get too dim.
				 */
				a = max;

				/* 
				 ** rescale all the color components if the intensity of the greatest
				 ** channel exceeds 1.0
				 */
				if (max > 255) {
					float t = 255.0F / max;

					r = r * t;
					g = g * t;
					b = b * t;
					a = a * t;
				}

				dest[0] = r;
				dest[1] = g;
				dest[2] = b;
				dest[3] = a;

				bl += 3;
				dest += 4;
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

