/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2004-2023 Quake2xp Team.

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

char skyname[MAX_QPATH];
float skyrotate;
vec3_t skyaxis;

/*
============
R_SetSky
============
*/
void R_SetSky(char *name, float rotate, vec3_t axis) {

	skyrotate = rotate;
	VectorCopy(axis, skyaxis);
}

/*
==============
R_DrawSkyBox
==============
*/

void R_AddSkyPolys(msurface_t *surf, unsigned *indeces) {
	unsigned	numIndices;
	int			i, nv = surf->numEdges;

	numIndices = *indeces;

	for (i = 0; i < nv - 2; i++) {
		indexArray[numIndices++] = surf->baseIndex;
		indexArray[numIndices++] = surf->baseIndex + i + 1;
		indexArray[numIndices++] = surf->baseIndex + i + 2;
	}
	*indeces = numIndices;
}

#define MAX_SKY_IDX 12288 // 4096*3

void R_DrawSkyBox(){
	msurface_t *s;
	int			i;
	uint		numIndices = 0;

	// setup program
	GL_BindProgram(skyProgram);

	qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float *)r_newrefdef.modelViewProjectionMatrix);
	qglUniformMatrix4fv(U_TEXTURE0_MATRIX, 1, qfalse, (const float *)r_newrefdef.skyMatrix);

	GL_SetBindlessTexture(U_TMU0, skyCube_handle);

	for (i = 0; i < numSkySurfaces; i++) {
		s = skySurfaces[i];

		R_AddSkyPolys(s, &numIndices);

		if (numIndices >= MAX_SKY_IDX) {
			GL_DrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
			c_brush_polys += numIndices / 3;

			R_ShowTrisBSP(qfalse, numIndices, 1.0, 0.7, 0.0, skyProgram);
			numIndices = 0;
		}
	}
	if (numIndices) {
		GL_DrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
		c_brush_polys += numIndices / 3;

		R_ShowTrisBSP(qfalse, numIndices, 1.0, 0.7, 0.0, skyProgram);
		numIndices = 0;
	}
	numSkySurfaces = 0;
}

uint	transi[4096 * 4096];
float	transf[1024 * 1024];

void R_FlipImageFloat(int i, hdri_t* hdri, float* dst) {
	float* from;
	float* src = hdri->data;
	int	width = hdri->width;
	int	height = hdri->height;
	int	x, y;

	if (i == 1)		// bk
	{
		for (y = height - 1; y >= 0; y--) {
			for (x = width - 1; x >= 0; x--) {	// copy rgb components
				from = src + (x * height + y) * 3;
				dst[0] = from[0];
				dst[1] = from[1];
				dst[2] = from[2];
				dst += 3;
			}
		}
		return;
	}

	if (i == 2)		// lf
	{
		for (y = height - 1; y >= 0; y--) {
			for (x = 0; x < width; x++) {	// copy rgb components
				from = src + (y * width + x) * 3;
				dst[0] = from[0];
				dst[1] = from[1];
				dst[2] = from[2];
				dst += 3;
			}
		}
		return;
	}

	if (i == 3)		// rt
	{
		for (y = 0; y < height; y++) {
			for (x = width - 1; x >= 0; x--) {	// copy rgb components
				from = src + (y * width + x) * 3;
				dst[0] = from[0];
				dst[1] = from[1];
				dst[2] = from[2];
				dst += 3;
			}
		}
		return;
	}

	// ft, up, dn
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {	// copy rgb components
			from = src + (x * height + y) * 3;
			dst[0] = from[0];
			dst[1] = from[1];
			dst[2] = from[2];
			dst += 3;
		}
	}
}

// q2 skybox sides to ogl cubemap faces - flip back and left sides 
char *cubeSide[6] = { "rt", "lf", "bk", "ft", "up", "dn" };

void R_GenSkyCubeMap(char* name) {
	int			i, numMips;
	char		ldrName[MAX_QPATH], hdrName[MAX_QPATH];
	img_t		pix[6];
	hdri_t		hdri[6];
	qboolean	hdr = qfalse;

	strncpy(skyname, name, sizeof(skyname) - 1);
	
	if (skyCube) {
		glMakeTextureHandleNonResidentARB(skyCube_handle);
		qglDeleteTextures(1, &skyCube);
	}
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &skyCube);

	for (i = 0; i < 6; i++) {
		pix[i].pixels = NULL;
		pix[i].width = pix[i].height = 0;
		
		hdri[i].width = hdri[i].height = 0;
		hdri[i].data = NULL;
		
		Com_sprintf(hdrName, sizeof(hdrName), "env/hdr/%s%s.hdr", skyname, cubeSide[i]);

		if (STB_LoadHdr(hdrName, &hdri[i].data, &hdri[i].width, &hdri[i].height)) {
			numMips = CalcMipmapCount(hdri[0].width, hdri[0].height);
			glTextureStorage2D(skyCube, numMips, GL_RGB32F, hdri[0].width, hdri[0].height);
			hdr = qtrue;
		}
		else {
			Com_sprintf(ldrName, sizeof(ldrName), "env/%s%s.tga", skyname, cubeSide[i]);
			STB_LoadLdr(ldrName, &pix[i].pixels, &pix[i].width, &pix[i].height);

			numMips = CalcMipmapCount(pix[0].width, pix[0].height);
			glTextureStorage2D(skyCube, numMips, GL_RGB8, pix[0].width, pix[0].height);
		}
	}

	for (i = 0; i < 6; i++) {

		if (!hdr) {
			R_FlipImage(i, &pix[i], (byte*)transi);
			free(pix[i].pixels);
			glTextureSubImage3D(skyCube, 0, 0, 0, i, pix[i].width, pix[i].height, 1, GL_RGB, GL_UNSIGNED_BYTE, transi);
		}
		else {
			R_FlipImageFloat(i, &hdri[i], transf);
			free(hdri[i].data);
			glTextureSubImage3D(skyCube, 0, 0, 0, i, hdri[i].width, hdri[i].height, 1, GL_RGB, GL_FLOAT,transf);
		}
	}

	glTextureParameteri(skyCube, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(skyCube, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(skyCube, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(skyCube, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(skyCube, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(skyCube, GL_TEXTURE_BASE_LEVEL, 0);
	glTextureParameteri(skyCube, GL_TEXTURE_MAX_LEVEL, numMips-1);
	glGenerateTextureMipmap(skyCube);

	skyCube_handle = glGetTextureHandleARB(skyCube);
	glMakeTextureHandleResidentARB(skyCube_handle);
}