/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
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

extern model_t *loadmodel;

//
// 3-vector radiosity basis for normal mapping
// need high precision to lower error on further bounces
//
const vec3_t r_xplmBasisVecs[XPLM_NUMVECS] = {
	{ 0.81649658092772603273242802490196f, 0.f, 0.57735026918962576450914878050195f },
	{ -0.40824829046386301636621401245098f, 0.70710678118654752440084436210485f, 0.57735026918962576450914878050195f },
	{ -0.40824829046386301636621401245098f, -0.70710678118654752440084436210485f, 0.57735026918962576450914878050195f }
};


/*
===============
R_BuildLightMap

Combine & scale multiple lightmaps into the floating-point format in s_blocklights,
then normalize into GL format in gl_lms.lightmap_buffer.
===============
*/
//extern int *mod_xplmOffsets;		// face light offsets from .xplm file, freed after level is loaded

void R_BuildLightMap (msurface_t *surf, int stride) {
	static float s_blocklights[LIGHTMAP_BLOCKLIGHTS_SIZE];		// intermediate RGB buffer for combining multiple lightmaps
	const int numVecs = loadmodel->useXPLM ? 3 : 1;
	int smax, tmax;
	int r, g, b, cmax;
	int i, j, k, size;
	vec3_t scale;
	float *bl;
	byte *lm, *dest;
	int numMaps;

	if (surf->texInfo->flags & (SURF_SKY | SURF_TRANS33 | SURF_TRANS66 | SURF_WARP))
		VID_Error (ERR_DROP, "R_BuildLightMap(): called for non-lit surface.");

	surf->lightmapTexNum = gl_lms.texnum;

	// no more dynamic lightmaps, so only loadmodel is used
	smax = (surf->extents[0] / (int)loadmodel->lightmap_scale) + 1;
	tmax = (surf->extents[1] / (int)loadmodel->lightmap_scale) + 1;

	size = smax * tmax;
	stride -= smax * 3;

	if (size > ((sizeof(float) * LIGHTMAP_BLOCKLIGHTS_SIZE) / (int)loadmodel->lightmap_scale))
		VID_Error(ERR_DROP, "R_BuildLightMap(): bad gl_lms.blocklights size.");

	// count maps
	if (surf->samples)
	for (numMaps = 0; numMaps < MAXLIGHTMAPS && surf->styles[numMaps] != 255; numMaps++);

	for (k = 0; k < numVecs; k++) {
		if (surf->samples) {
			lm = surf->samples + k * size * 3;

			// KRIGS:
			// we don't refresh lightmaps at runtime anymore, so load only the first map
			// use real-time lights for styling instead
			// 0 or 1
			if (1 || numMaps == 1) {
				// copy the lightmap
				bl = s_blocklights;

				for (i = 0; i < 3; i++)
					scale[i] = r_newrefdef.lightstyles[surf->styles[0]].rgb[i];

				if (scale[0] == 1.f && scale[1] == 1.f && scale[2] == 1.f) {
					for (i = 0; i < size; i++, bl += 3, lm += 3) {
						bl[0] = lm[0];
						bl[1] = lm[1];
						bl[2] = lm[2];
					}
				}
				else {
					for (i = 0; i < size; i++, bl += 3, lm += 3) {
						bl[0] = lm[0] * scale[0];
						bl[1] = lm[1] * scale[1];
						bl[2] = lm[2] * scale[2];
					}
				}
			}
			else {
				// add all the lightmaps
				memset (s_blocklights, 0, sizeof(s_blocklights[0]) * size * 3);

				for (j = 0; j < numMaps; j++) {
					bl = s_blocklights;

					for (i = 0; i < 3; i++)
						scale[i] = r_newrefdef.lightstyles[surf->styles[j]].rgb[i];

					if (scale[0] == 1.f && scale[1] == 1.f && scale[2] == 1.f) {
						for (i = 0; i < size; i++, bl += 3, lm += 3) {
							bl[0] += lm[0];
							bl[1] += lm[1];
							bl[2] += lm[2];
						}
					}
					else {
						for (i = 0; i < size; i++, bl += 3, lm += 3) {
							bl[0] += lm[0] * scale[0];
							bl[1] += lm[1] * scale[1];
							bl[2] += lm[2] * scale[2];
						}
					}

					if (loadmodel->useXPLM)
						lm += size * 6;	// skip the rest 6/9 to the next style's lightmap
				}
			}
		}
		else {
			// set to full bright if no light data
			for (i = 0; i < size * 3; i++)
				s_blocklights[i] = 255.f;
		}

		//
		// put into texture format
		//

		bl = s_blocklights;

		dest = gl_lms.lightmap_buffer[k];
		dest += (surf->light_t * LIGHTMAP_SIZE + surf->light_s) * 3;

		for (i = 0; i < tmax; i++, dest += stride) {
			for (j = 0; j < smax; j++, bl += 3, dest += 3) {
				r = Q_ftol (bl[0]);
				g = Q_ftol (bl[1]);
				b = Q_ftol (bl[2]);

				// catch negative lights
				r = max (r, 0);
				g = max (g, 0);
				b = max (b, 0);

				// determine the brightest of the three color components
				cmax = max (r, max (g, b));

				// rescale all the color components if the intensity of the greatest channel exceeds 1.0
				if (cmax > 255) {
					float t = 255.f / cmax;

					r *= t;
					g *= t;
					b *= t;
				}

				dest[0] = r;
				dest[1] = g;
				dest[2] = b;
			}
		}
	}	// for k

}


// FIXME: remove dynamic completely
static void LM_UploadBlock () {

	const int	numLm = loadmodel->useXPLM ? 3 : 1;
	int			texture = gl_lms.texnum;

	qglGenTextures(1, &gl_lms.texnum);	

	// upload the finished atlas
	for (int i = 0; i < numLm; i++) {
		
		GL_Bind (gl_state.lightmapOffcet + texture + i * MAX_LIGHTMAPS);

		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		qglTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, LIGHTMAP_SIZE, LIGHTMAP_SIZE);
		qglTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, LIGHTMAP_SIZE, LIGHTMAP_SIZE, GL_RGB, GL_UNSIGNED_BYTE, gl_lms.lightmap_buffer[i]);

	}
}

// returns a texture number and the position inside it
static qboolean LM_AllocBlock (int w, int h, int *x, int *y) {
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
		return qfalse;

	for (i = 0; i < w; i++)
		gl_lms.allocated[*x + i] = best + h;

	return qtrue;
}


/*
========================
GL_CreateSurfaceLightmap

========================
*/
void GL_CreateSurfaceLightmap (msurface_t * surf) {
	int smax, tmax;

	if (surf->flags & (MSURF_DRAWSKY | MSURF_DRAWTURB))
		return;

	smax = (surf->extents[0] / loadmodel->lightmap_scale) + 1;
	tmax = (surf->extents[1] / loadmodel->lightmap_scale) + 1;

	if (!LM_AllocBlock (smax, tmax, &surf->light_s, &surf->light_t))
		VID_Error (ERR_FATAL, "GL_CreateSurfaceLightmap(): consecutive calls to LM_AllocBlock(%d, %d) failed.\n", smax, tmax);

	R_BuildLightMap (surf, LIGHTMAP_SIZE * 3);
}

extern int occ_framecount;

/*
==================
GL_BeginBuildingLightmaps

==================
*/
void GL_BeginBuildingLightmaps (model_t *m) {
	static lightstyle_t lightstyles[MAX_LIGHTSTYLES];
	int i;

	memset (gl_lms.allocated, 0, sizeof(gl_lms.allocated));

	occ_framecount = r_framecount = 1;

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

	if (!gl_state.lightmapOffcet)
		gl_state.lightmapOffcet = TEXNUM_LIGHTMAPS;
}

/*
=======================
GL_EndBuildingLightmaps
=======================
*/
void GL_EndBuildingLightmaps (void) {
	LM_UploadBlock ();
}

