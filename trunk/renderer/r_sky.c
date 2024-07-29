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

void R_DrawSkyBox(){
	
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
	// setup program
	GL_BindProgram(skyProgram);
	qglUniformMatrix4fv(U_MVP_MATRIX, 1, false, (const float *)r_newrefdef.modelViewProjectionMatrix);
	qglUniformMatrix4fv(U_TEXTURE0_MATRIX, 1, false, (const float *)r_newrefdef.skyMatrix);

	GL_SetBindlessTexture(U_TMU0, gi.levelSkyBox->handle);

	GL_BindVAO(vao.sky);
	GL_DrawElements(GL_TRIANGLES, CUBE_INDICES, GL_UNSIGNED_BYTE, NULL);
}

// convert q2 skybox sides to ogl cubemap faces
char *cubeSide[6] = { "rt", "lf", "bk", "ft", "up", "dn" };
//cubemap face names   x+    x-    y+    y-    z+    z-

//photoshop helper
//rt  -   rot90cc flip y
//lf  -   rot90c flip y
//bk  -   flip y
//ft  -   flip x 
//up-dn - rot90c flip x

uint	trans[2048 * 2048];
image_t *R_MakeLegacySkyCubeMap(char *name) {
	int			i, side;
	char		pname[MAX_QPATH];
	img_t		pix[6];
	image_t		*image;
	uint		hash = Com_HashKey(name);

	if (!name)
		return NULL;

	int len = strlen(name);

	if (len < 5)
		return NULL;

	for (i = 0, image = gi.r_textures; i < gi.r_numTextures; i++, image++) {

		if (image->hash == hash) {

			if (!b_stricmp(image->name, name)) {

				image->registration_sequence = registration_sequence;
				return image;
			}
		}
	}
	
	// find a free image_t
	for (i = 0, image = gi.r_textures; i < gi.r_numTextures; i++, image++) {
		if (!image->texnum)
			break;
	}
	if (i == gi.r_numTextures) {
		if (gi.r_numTextures == MAX_GLTEXTURES)
			VID_Error(ERR_FATAL, "MAX_GLTEXTURES");

		gi.r_numTextures++;
	}

	image = &gi.r_textures[i];
	strcpy(image->name, name);
	image->type = it_sky;
	image->hash = hash;
	image->compressed = false;
	image->has_alpha = true;
	image->paletted = false;
	image->legacySky = true;

	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &image->texnum);

	for (side = 0; side < 6; side++) {

		pix[side].pixels = NULL;
		pix[side].width = pix[side].height = 0;

		Com_sprintf(pname, sizeof(pname), "env/%s%s.tga", skyname, cubeSide[side]);
		STB_LoadTexture(pname, &pix[side].pixels, &pix[side].width, &pix[side].height);

		glTextureStorage2D(image->texnum, 1, GL_SRGB8, pix[0].width, pix[0].height);

		R_FlipImage(side, &pix[side], (byte *)trans);
		free(pix[side].pixels);
		glTextureSubImage3D(image->texnum, 0, 0, 0, side, pix[side].width, pix[side].height, 1, GL_RGB, GL_UNSIGNED_BYTE, trans);
	}

	image->width = pix[0].width * 6;
	image->height = pix[0].height;
	image->upload_width = pix[0].width * 6;
	image->upload_height = pix[0].height;
	image->numMips = 1;

	image->dataType = GL_UNSIGNED_BYTE;
	image->texType = GL_TEXTURE_CUBE_MAP;
	image->intFormat = GL_SRGB8;

	glTextureParameteri(image->texnum, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(image->texnum, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(image->texnum, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTextureParameteri(image->texnum, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

	image->handle = glGetTextureHandleARB(image->texnum);
	glMakeTextureHandleResidentARB(image->handle);

	return image;
}


void R_GenSkyCubeMap(char* name) {
	char		ddsName[MAX_QPATH];

	strncpy(skyname, name, sizeof(skyname) - 1);
	
	Com_sprintf(ddsName, sizeof(ddsName), "env/dds/%s.dds", skyname);
	gi.levelSkyBox = R_LoadDDS(ddsName, it_sky);

	if (!gi.levelSkyBox)
		gi.levelSkyBox = R_MakeLegacySkyCubeMap(skyname);
	
	if (!gi.levelSkyBox)
		gi.levelSkyBox = gi.missingTexture;
}