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

#include "r_local.h"

model_t *loadmodel;
static float s_blocklights[1024 * 1024 * 3];
/*
=============================================================================
LIGHTMAP ALLOCATION
=============================================================================
*/

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

		fminlight = 0;
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
===============
R_BuildLightMap

Combine and scale multiple lightmaps into the floating format in blocklights
===============
*/

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
		
		for (maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255; maps++) {
				style = &r_newrefdef.lightstyles[surf->styles[maps]];
		}
		goto store;
	}
	// count the # of maps
	for (nummaps = 0; nummaps < MAXLIGHTMAPS && surf->styles[nummaps] != 255; nummaps++)
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

	// add all the dynamic lights for non bumped sufaces
	if(!r_bumpWorld->value){

		if (surf->dlightframe == r_framecount)
			R_AddDynamicLights(surf);
	}

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



_inline static void LM_InitBlock(void)
{
	memset(gl_lms.allocated, 0, sizeof(gl_lms.allocated));
}

static void LM_UploadBlock(qboolean dynamic)
{
	int texture;
	int height = 0;

	if (dynamic) {
		texture = 0;
	} else {
		texture = gl_lms.current_lightmap_texture;
	}

	GL_Bind(gl_state.lightmap_textures + texture);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (dynamic) {
		int i;

		for (i = 0; i < LIGHTMAP_SIZE; i++) {
			if (gl_lms.allocated[i] > height)
				height = gl_lms.allocated[i];
		}

		qglTexSubImage2D(GL_TEXTURE_2D,
						 0,
						 0, 0,
						 LIGHTMAP_SIZE, height,
						 GL_LIGHTMAP_FORMAT,
						 GL_UNSIGNED_INT_8_8_8_8_REV, gl_lms.lightmap_buffer);
	} else {
		qglTexImage2D(GL_TEXTURE_2D,
					  0,
					  gl_lms.internal_format,
					  LIGHTMAP_SIZE, LIGHTMAP_SIZE,
					  0,
					  GL_LIGHTMAP_FORMAT,
					  GL_UNSIGNED_INT_8_8_8_8_REV, gl_lms.lightmap_buffer);
		if (++gl_lms.current_lightmap_texture == MAX_LIGHTMAPS)
			VID_Error(ERR_DROP,
					  "LM_UploadBlock() - MAX_LIGHTMAPS exceeded\n");
	}
}

// returns a texture number and the position inside it
static qboolean LM_AllocBlock(int w, int h, int *x, int *y)
{
	int i, j;
	int best, best2;

	best = LIGHTMAP_SIZE;

	for (i = 0; i < LIGHTMAP_SIZE - w; i++) {
		best2 = 0;

		for (j = 0; j < w; j++) {
			if (gl_lms.allocated[i + j] >= best)
				break;
			if (gl_lms.allocated[i + j] > best2)
				best2 = gl_lms.allocated[i + j];
		}
		if (j == w) {			// this is a valid spot
			*x = i;
			*y = best = best2;
		}
	}

	if (best + h > LIGHTMAP_SIZE)
		return false;

	for (i = 0; i < w; i++)
		gl_lms.allocated[*x + i] = best + h;

	return true;
}

/*
================
GL_BuildPolygonFromSurface
================
*/
void GL_BuildPolygonFromSurface(msurface_t * fa)
{
	int i, lindex, lnumverts;
	medge_t *pedges, *r_pedge;
	int vertpage;
	float *vec;
	float s, t;
	glpoly_t *poly;
	vec3_t total;
	
	fa->numVertices = fa->numedges;
    fa->numIndices = (fa->numVertices - 2) * 3;

	// reconstruct the polygon
	pedges = currentmodel->edges;
	lnumverts = fa->numedges;
	vertpage = 0;

	VectorClear(total);
	//
	// draw texture
	//
	poly = (glpoly_t*)Hunk_Alloc(sizeof(glpoly_t) + (lnumverts - 4) * VERTEXSIZE * sizeof(float));
	poly->next = fa->polys;
	poly->flags = fa->flags;
	fa->polys = poly;
	poly->numverts = lnumverts;

	currentmodel->memorySize += sizeof(glpoly_t) + (lnumverts - 4) * VERTEXSIZE * sizeof(float);
	
	for (i = 0; i < lnumverts; i++) {
		lindex = currentmodel->surfedges[fa->firstedge + i];

		if (lindex > 0) {
			r_pedge = &pedges[lindex];
			vec = currentmodel->vertexes[r_pedge->v[0]].position;
		} else {
			r_pedge = &pedges[-lindex];
			vec = currentmodel->vertexes[r_pedge->v[1]].position;
		}
		s = DotProduct(vec,
					   fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
		s /= fa->texinfo->image->width;

		t = DotProduct(vec,
					   fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
		t /= fa->texinfo->image->height;

		VectorAdd(total, vec, total);
		VectorCopy(vec, poly->verts[i]);
		poly->verts[i][3] = s;
		poly->verts[i][4] = t;

		//
		// lightmap texture coordinates
		//
		  s = DotProduct(vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
          s -= fa->texturemins[0];
          s += fa->light_s * loadmodel->lightmap_scale;
          s += loadmodel->lightmap_scale / 2;
          s /= LIGHTMAP_SIZE * loadmodel->lightmap_scale;

          t = DotProduct(vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
          t -= fa->texturemins[1];
          t += fa->light_t * loadmodel->lightmap_scale;
          t += loadmodel->lightmap_scale / 2;
          t /= LIGHTMAP_SIZE * loadmodel->lightmap_scale;

		poly->verts[i][5] = s;
		poly->verts[i][6] = t;

	}

	poly->numverts = lnumverts;

	VectorScale(total, 1.0f / (float) lnumverts, total);

	fa->c_s =
		(DotProduct(total, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3])
		/ fa->texinfo->image->width;
	fa->c_t =
		(DotProduct(total, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3])
		/ fa->texinfo->image->height;
}

/*
========================
GL_CreateSurfaceLightmap
========================
*/
void GL_CreateSurfaceLightmap(msurface_t * surf)
{
	int smax, tmax;
	byte *base;

	if (surf->flags & (SURF_DRAWSKY | SURF_DRAWTURB))
		return;

	smax = (surf->extents[0] / loadmodel->lightmap_scale) + 1; 
	tmax = (surf->extents[1] / loadmodel->lightmap_scale) + 1;
	
	if (!LM_AllocBlock(smax, tmax, &surf->light_s, &surf->light_t)) {
		LM_UploadBlock(false);
		LM_InitBlock();
		if (!LM_AllocBlock(smax, tmax, &surf->light_s, &surf->light_t)) {
			VID_Error(ERR_FATAL,
					  "Consecutive calls to LM_AllocBlock(%d,%d) failed\n",
					  smax, tmax);
		}
	}

	surf->lightmaptexturenum = gl_lms.current_lightmap_texture;

	base = gl_lms.lightmap_buffer;
	base += (surf->light_t * LIGHTMAP_SIZE + surf->light_s) * LIGHTMAP_BYTES;

	R_SetCacheState(surf);
	R_BuildLightMap(surf, base, LIGHTMAP_SIZE * LIGHTMAP_BYTES, true);
}


/*
==================
GL_BeginBuildingLightmaps
==================
*/
void GL_BeginBuildingLightmaps(model_t * m)
{
	static lightstyle_t lightstyles[MAX_LIGHTSTYLES];
	int i;
	byte *dummy;

	memset(gl_lms.allocated, 0, sizeof(gl_lms.allocated));
	
	dummy = (byte*)Z_Malloc(LIGHTMAP_BYTES * LIGHTMAP_SIZE * LIGHTMAP_SIZE);

	r_framecount = 1;			// no dlightcache

	GL_SelectTexture( GL_TEXTURE1 );
	qglEnable( GL_TEXTURE_2D );

	/*
	 ** setup the base lightstyles so the lightmaps won't have to be regenerated
	 ** the first time they're seen
	 */
	for (i = 0; i < MAX_LIGHTSTYLES; i++) {
		lightstyles[i].rgb[0] = 1;
		lightstyles[i].rgb[1] = 1;
		lightstyles[i].rgb[2] = 1;
		lightstyles[i].white = 3;
	}
	r_newrefdef.lightstyles = lightstyles;

	if (!gl_state.lightmap_textures)
		gl_state.lightmap_textures = TEXNUM_LIGHTMAPS;


	gl_lms.current_lightmap_texture = 1;
	gl_lms.internal_format = gl_tex_solid_format;

	/*
	 ** initialize the dynamic lightmap texture
	 */
	GL_Bind(gl_state.lightmap_textures + 0);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	qglTexImage2D(GL_TEXTURE_2D,
				  0,
				  gl_lms.internal_format,
				  LIGHTMAP_SIZE, LIGHTMAP_SIZE,
				  0, GL_LIGHTMAP_FORMAT, GL_UNSIGNED_INT_8_8_8_8_REV, dummy);
	
	Z_Free(dummy);
}

/*
=======================
GL_EndBuildingLightmaps
=======================
*/
void GL_EndBuildingLightmaps(void)
{
	LM_UploadBlock(false);

	GL_SelectTexture( GL_TEXTURE1 );
	qglDisable( GL_TEXTURE_2D );
	GL_SelectTexture( GL_TEXTURE0 );
}

