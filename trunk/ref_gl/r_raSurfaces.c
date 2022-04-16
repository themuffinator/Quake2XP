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

int alphaSurfSort(const msurface_t** a, const msurface_t** b) {
	return	(((*a)->texInfo->image->texnum)) - (((*b)->texInfo->image->texnum));
}
int waterSurfSort(const msurface_t** a, const msurface_t** b) {
	return	(((*a)->texInfo->image->texnum)) - (((*b)->texInfo->image->texnum));
}

msurface_t* interactionTranSurf[MAX_MAP_FACES];
int			numInteractionTransSurfs;

void R_AddAlphaSurceces(msurface_t* s, uint* indeces, qboolean update) {
	int i;
	uint numIndices;
	float scroll = 0.0;
	qboolean scrolling = qfalse;
	int nv = s->polys->numVerts;

	numIndices = *indeces;

	if (s->texInfo->flags & SURF_FLOWING) {
		scroll = -64 * ((r_newrefdef.time / 40.0) - (int)(r_newrefdef.time / 40.0));

		if (scroll == 0.0)
			scroll = -64.0;

		scrolling = qtrue;

		qglUniform1f(U_SCROLL, scroll);
	}
	else
		qglUniform1f(U_SCROLL, 0.0);

	if (update) {

		if (scrolling)
			GL_SetBindlessTexture(U_TMU0, r_DSTTex->handle);
		else
			GL_SetBindlessTexture(U_TMU0, s->texInfo->normalmap->handle);
		GL_SetBindlessTexture(U_TMU1, s->texInfo->image->handle);
	}

	for (i = 0; i < nv - 2; i++) {
		indexArray[numIndices++] = s->baseIndex;
		indexArray[numIndices++] = s->baseIndex + i + 1;
		indexArray[numIndices++] = s->baseIndex + i + 2;
	}
	*indeces = numIndices;
}


void R_DrawAlphaSurfaces() {
	msurface_t* s;
	qboolean	newTex;
	float		ambientScale = max(r_lightmapScale->value, 0.33);
	uint		oldTex = 0;
	uint		numIndices = 0;

	// setup program
	GL_BindProgram(glassProgram);

	GL_SetBindlessTexture(U_TMU2, r_screenTex->handle);
	GL_SetBindlessTexture(U_TMU3, r_depthTex->handle);

	qglUniform1f(U_REFR_DEFORM_MUL, 1.0);
	qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float*)r_newrefdef.modelViewProjectionMatrix);
	qglUniformMatrix4fv(U_MODELVIEW_MATRIX, 1, qfalse, (const float*)r_newrefdef.modelViewMatrix);
	qglUniformMatrix4fv(U_PROJ_MATRIX, 1, qfalse, (const float*)r_newrefdef.projectionMatrix);

	qglUniform1f(U_REFR_THICKNESS0, 150.0);
	qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);
	qglUniform2f(U_DEPTH_PARAMS, r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);
	qglUniform1f(U_AMBIENT_LEVEL, ambientScale);

	float blurScale = 18.88 * ((float)vid.width / 1024.0);
	qglUniform1f(U_PARAM_FLOAT_0, blurScale);

	qsort(r_alphaSurfaces, numAlphaSurfaces, sizeof(msurface_t*), (int(*)(const void*, const void*))alphaSurfSort);

	for (int i = 0; i < numAlphaSurfaces; i++) {

		s = r_alphaSurfaces[i];

		if (s->flags & MSURF_LAVA)
			continue;

		if (s->texInfo->image->texnum != oldTex) {
			if (numIndices) {
				qglDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
				c_brush_polys += numIndices / 3;
				numIndices = 0;
			}
			oldTex = s->texInfo->image->texnum;
			newTex = qtrue;
		}
		else
			newTex = qfalse;

		R_AddAlphaSurceces(s, &numIndices, newTex);

		if (numIndices >= MAX_IDX) { //overflow
			qglDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
			c_brush_polys += numIndices / 3;
			numIndices = 0;
		}
	}
	if (numIndices) {
		qglDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
		c_brush_polys += numIndices / 3;
		numIndices = 0;
	}
	numAlphaSurfaces = 0;
}

void R_AddWaterSurceces(msurface_t* s, uint* indeces, qboolean update) {
	int i;
	uint numIndices;
	float scroll = 0.0;
	qboolean scrolling = qfalse;
	int nv = s->polys->numVerts;

	numIndices = *indeces;

	if (s->texInfo->flags & (SURF_TRANS33 | SURF_TRANS66))
		qglUniform1i(U_WATER_TRANS, 1);
	else
		qglUniform1i(U_WATER_TRANS, 0);

	if (update)
		GL_SetBindlessTexture(U_TMU0, s->texInfo->image->handle);

	for (i = 0; i < nv - 2; i++) {
		indexArray[numIndices++] = s->baseIndex;
		indexArray[numIndices++] = s->baseIndex + i + 1;
		indexArray[numIndices++] = s->baseIndex + i + 2;
	}
	*indeces = numIndices;
}

void R_DrawWaterSurfaces(qboolean bmodel) {
	msurface_t* s;
	float		ambientScale = max(r_lightmapScale->value, 0.33);
	qboolean	newTex;
	uint		oldTex = 0;
	uint		numIndices = 0;

	GL_BindProgram(waterProgram);

	GL_SetBindlessTexture(U_TMU1, r_waterNormals[((int)(r_newrefdef.time * 15)) & (MAX_WATER_NORMALS - 1)]->handle);
	GL_SetBindlessTexture(U_TMU2, r_screenTex->handle);
	GL_SetBindlessTexture(U_TMU3, r_depthTex->handle);

	qglUniform1f(U_WATER_DEFORM_MUL, 1.0);
	qglUniform1f(U_AMBIENT_LEVEL, ambientScale);
	qglUniform1f(U_WATHER_THICKNESS, 150.0);
	qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);
	qglUniform2f(U_DEPTH_PARAMS, r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);

	if (!bmodel)
		qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float*)r_newrefdef.modelViewProjectionMatrix);
	else
		qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float*)currententity->orMatrix);

	if (r_newrefdef.rdflags & RDF_UNDERWATER)
		qglUniform1i(U_WATER_MIRROR, 0);
	else
		qglUniform1i(U_WATER_MIRROR, 1);

	qglUniformMatrix4fv(U_MODELVIEW_MATRIX, 1, qfalse, (const float*)r_newrefdef.modelViewMatrix);
	qglUniformMatrix4fv(U_PROJ_MATRIX, 1, qfalse, (const float*)r_newrefdef.projectionMatrix);

	qsort(r_reflectiveSurfaces, numReflectiveSurfaces, sizeof(msurface_t*), (int(*)(const void*, const void*))waterSurfSort);

	for (int i = 0; i < numReflectiveSurfaces; i++) {
		s = r_reflectiveSurfaces[i];

		if (s->flags & MSURF_LAVA)
			continue;

		if (s->texInfo->image->texnum != oldTex) {
			if (numIndices) {
				qglDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
				c_brush_polys += numIndices / 3;
				numIndices = 0;
			}
			oldTex = s->texInfo->image->texnum;
			newTex = qtrue;
		}
		else
			newTex = qfalse;

		R_AddWaterSurceces(s, &numIndices, newTex);

		if (numIndices >= MAX_IDX) { //overflow
			qglDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
			c_brush_polys += numIndices / 3;
			numIndices = 0;
		}
	}
	if (numIndices) {
		qglDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
		c_brush_polys += numIndices / 3;
		numIndices = 0;
	}
	numReflectiveSurfaces = 0;
}




void R_DrawSurfacesRA(qboolean bmodel) {

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	glBindVertexArray(vao.bsp);

	R_CaptureColorBuffer();
	R_DrawAlphaSurfaces();

	R_CaptureColorBuffer();
	R_DrawWaterSurfaces(bmodel);

	glBindVertexArray(0);
}