/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2004-2024 Quake2xp Team.

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

/*
===============
R_LoadFont
===============
*/

void R_Init2D(void)
{
	i_menuFont = R_LoadDDS("gfx/fonts/engfont.dds", it_nomips);

	if (!i_menuFont)
		i_menuFont = GL_FindImage("pics/conchars.pcx", it_nomips);

	if (!i_menuFont)
		VID_Error(ERR_DROP, "couldn't load pics/conchars");

	i_consFont = R_LoadDDS("gfx/fonts/intfont.dds", it_nomips);
	if (!i_consFont)
		i_consFont = i_missingTexture;
}

void R_DrawTexturedQuad() {

	GL_BindVAO(vao.tess2d);
	GL_BindVBO(vbo.tess2dVbo);	
	GL_BindVBO(vbo.quadIbo);

	qglInvalidateBufferData(GL_ARRAY_BUFFER);
	qglBufferSubData(GL_ARRAY_BUFFER, 0, QUAD_VERTS * sizeof(vertex2d_t), &tess2d);
	GL_DrawElements(GL_TRIANGLES, QUAD_INDICES, GL_UNSIGNED_BYTE, NULL);
}

void R_Flush2D() {

	if (!tess2dArray.numVerts || tess2dArray.numVerts < 4)
		return;

	GL_BindProgram(picProgram);
	qglUniform1i(U_PARAM_INT_0, PF_COLOREDFONT);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float *)r_newrefdef.orthoMatrix);

	GL_SetBindlessTexture(U_TMU0, tess2dArray.handle);

	GL_BindVAO(vao.tess2dArray);
	GL_BindVBO(vbo.tess2dArrayVbo);
	GL_BindVBO(vbo.quadStringIbo);

	qglInvalidateBufferData(GL_ARRAY_BUFFER);
	qglBufferSubData(GL_ARRAY_BUFFER, 0, tess2dArray.numVerts * sizeof(vertex2d_t), &tess2dArray);
	GL_DrawElements(GL_TRIANGLES, QUAD_INDICES * tess2dArray.numSymbols, GL_UNSIGNED_INT, NULL);

	tess2dArray.numVerts = 0;
	tess2dArray.numSymbols = 0;
}

void R_AddCharsToList(int x, int y, int scale, unsigned char num, image_t *inTex) {
	int row, col, x1, y1;
	float frow, fcol, size;

	if (tess2dArray.numVerts + 4 >= MAX_VERTICES || (tess2dArray.numVerts && tess2dArray.handle != inTex->handle)) {
		R_Flush2D();
	}

	if ((num & 127) == 32)
		return;				// space

	if (y <= -8 * scale)
		return;				// totally off screen
	
	tess2dArray.handle = inTex->handle;

	row		= num >> 4;
	col		= num & 15;
	frow	= row * 0.0625;
	fcol	= col * 0.0625;
	size	= 0.0625;
	x1		= x + 8 * scale;
	y1		= y + 8 * scale;

	VA_SetElem2(tess2dArray.v[tess2dArray.numVerts + 0].tc, fcol, frow);
	VA_SetElem2(tess2dArray.v[tess2dArray.numVerts + 1].tc, fcol +size, frow);
	VA_SetElem2(tess2dArray.v[tess2dArray.numVerts + 2].tc, fcol +size, frow + size);
	VA_SetElem2(tess2dArray.v[tess2dArray.numVerts + 3].tc, fcol, frow + size);

	VA_SetElem2(tess2dArray.v[tess2dArray.numVerts + 0].pos, x,	 y);
	VA_SetElem2(tess2dArray.v[tess2dArray.numVerts + 1].pos, x1, y);
	VA_SetElem2(tess2dArray.v[tess2dArray.numVerts + 2].pos, x1, y1);
	VA_SetElem2(tess2dArray.v[tess2dArray.numVerts + 3].pos, x,	 y1);

	for (int i = 0; i < 4; i++)
		VA_SetElem4(tess2dArray.v[tess2dArray.numVerts + i].color, gl_state.fontColor[0], gl_state.fontColor[1], gl_state.fontColor[2], gl_state.fontColor[3]);

	tess2dArray.numSymbols++;
	tess2dArray.numVerts += 4;
}

/*
=============
Draw_FindPic
=============
*/
image_t* Draw_FindPic(char* name)
{
	image_t* gl;
	char fullname[MAX_QPATH];

	if (name[0] != '/' && name[0] != '\\') {

		Com_sprintf(fullname, sizeof(fullname), "pics/%s.dds", name);
		gl = R_LoadDDS(fullname, it_pic);
		
		if (!gl) {
			Com_sprintf(fullname, sizeof(fullname), "pics/%s.pcx", name);
			gl = GL_FindImage(fullname, it_pic);
		
		}
	}
	else {

		gl = GL_FindImage(name + 1, it_pic);
	}
	if (gl)
		if (gl != i_missingTexture)
			strcpy(gl->bare_name, name);

	return gl;
}

/*
=============
Draw_GetPicSize
=============
*/
void Draw_GetPicSize(int* w, int* h, char* pic)
{
	image_t *gl;

	gl = Draw_FindPic(pic);
	if (!gl) {
		*w = *h = -1;
		return;
	}
	*w = gl->width;
	*h = gl->height;
}


#define WIDTH_FHD 1920.0
#define HEIGHT_FHD 1080.0
#define WIDE_SCREEN_16x9  WIDTH_FHD / HEIGHT_FHD

float loadScreenColorFade;
float loadingLod;

/*
=============
Draw_StretchPic
=============
*/

void R_Draw_StretchPic(int x, int y, int w, int h, int flags, image_t *image, image_t *imageBump)
{
	float		offsX, offsY;
	float		aspect = (float)vid.width / (float)vid.height;

	if (aspect < WIDE_SCREEN_16x9) {  // quad screen
		offsX = (WIDTH_FHD - (HEIGHT_FHD * aspect)) / (WIDTH_FHD * 2.0);
		offsY = 0;
	}
	else if (aspect > WIDE_SCREEN_16x9) {   // super wide screen (21 x 9)
		offsX = 0;
		offsY = (HEIGHT_FHD - (WIDTH_FHD / aspect)) / (HEIGHT_FHD * 2.0);
	}
	else {
		offsX = offsY = 0;
	}
	if (w == h)
		offsX = offsY = 0;

	float	t;
	vec4_t	lPos;
	t = Sys_Milliseconds() * 0.001;
	lPos[0] = sin(t);
	lPos[1] = cos(t);
	lPos[2] = 0.5;
	lPos[3] = ((float)h + (float)y) / (float)h;
	VectorNormalize(lPos);
	lPos[0] = lPos[0] * 0.5 + 0.5;
	lPos[1] = lPos[1] * 0.5 + 0.5;
	lPos[2] = lPos[2] * 0.5 + 0.5;

	GL_BindProgram(picProgram);

	qglUniform1i(U_PARAM_INT_0, flags);
	qglUniform1i(U_PARAM_INT_1, gl_config.useHdrDisplay);
	qglUniform4fv(U_PARAM_VEC4_0, 1, lPos);
	qglUniform1f(U_PARAM_FLOAT_0, loadingLod);
	qglUniform1f(U_PARAM_FLOAT_1, loadScreenColorFade);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float *)r_newrefdef.orthoMatrix);

	GL_SetBindlessTexture(U_TMU0, image->handle);
	GL_SetBindlessTexture(U_TMU1, imageBump->handle);

	VA_SetElem2(tess2d.v[0].pos, x, y);
	VA_SetElem2(tess2d.v[1].pos, x + w, y);
	VA_SetElem2(tess2d.v[2].pos, x + w, y + h);
	VA_SetElem2(tess2d.v[3].pos, x, y + h);

	VA_SetElem2(tess2d.v[0].tc, image->sl + offsX, image->tl + offsY);
	VA_SetElem2(tess2d.v[1].tc, image->sh - offsX, image->tl + offsY);
	VA_SetElem2(tess2d.v[2].tc, image->sh - offsX, image->th - offsY);
	VA_SetElem2(tess2d.v[3].tc, image->sl + offsX, image->th - offsY);

	for (int i = 0; i < 4; i++)
		VA_SetElem4(tess2d.v[i].color, 1.0, 1.0, 1.0, 1.0);

	R_DrawTexturedQuad();
}

void Draw_StretchPic(int x, int y, int w, int h, int flags, char *imageName, char *imageName2){
	image_t *image, *bumpImage;

	if (flags & PF_LOADSCREEN) {
		image = GL_FindImage(imageName + 1, it_mipmap);
	}else
		image = Draw_FindPic(imageName);
	
	if (strstr(imageName2, "null"))
		bumpImage = i_defBump;
	else
		bumpImage	= Draw_FindPic(imageName2);

	if (!image)
		image = i_missingTexture;
	if (!bumpImage)
		bumpImage = i_defBump;

	R_Draw_StretchPic(x, y, w, h, flags, image, bumpImage);
}


/*
=============
Draw_Pic
=============
*/
void Draw_ScaledPic(int x, int y, float scaleX, float scaleY, int flags, image_t *image, image_t *imageBump){
	int w, h;

	w = image->width * scaleX * image->picScale_w;
	h = image->height * scaleY * image->picScale_h;

	if (!image->has_alpha)
		GL_Disable(GL_BLEND);

	if (flags & PF_CROSSHAIR) { 
		GL_Enable(GL_BLEND);
		GL_BlendFunc(GL_ONE, GL_ONE);
		w = image->width * scaleX;
		h = image->height * scaleY;
	}

	if (!r_drawPicBump->integer)
		flags &= ~PF_LIGHT;

	float	t;
	vec4_t	lPos;
	t = Sys_Milliseconds() * 0.001;
	lPos[0] = sin(t);
	lPos[1] = cos(t);
	lPos[2] = 0.5;
	lPos[3] = ((float)h + (float)y) / (float)h;
	VectorNormalize(lPos);
	lPos[0] = lPos[0] * 0.5 + 0.5;
	lPos[1] = lPos[1] * 0.5 + 0.5;
	lPos[2] = lPos[2] * 0.5 + 0.5;

	GL_BindProgram(picProgram);

	qglUniform1i(U_PARAM_INT_0, flags);
	qglUniform4fv(U_PARAM_VEC4_0, 1, lPos);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float *)r_newrefdef.orthoMatrix);

	GL_SetBindlessTexture(U_TMU0, image->handle);
	GL_SetBindlessTexture(U_TMU1, imageBump->handle);

	VA_SetElem2(tess2d.v[0].tc, image->sl, image->tl);
	VA_SetElem2(tess2d.v[1].tc, image->sh, image->tl);
	VA_SetElem2(tess2d.v[2].tc, image->sh, image->th);
	VA_SetElem2(tess2d.v[3].tc, image->sl, image->th);

	VA_SetElem2(tess2d.v[0].pos, x, y);
	VA_SetElem2(tess2d.v[1].pos, x + w, y);
	VA_SetElem2(tess2d.v[2].pos, x + w, y + h);
	VA_SetElem2(tess2d.v[3].pos, x, y + h);


	for (int i = 0; i < 4; i++) {
		if (flags & PF_CROSSHAIR)
			VA_SetElem4(tess2d.v[i].color, hColor[0], hColor[1], hColor[2], 1.0);
		else
			VA_SetElem4(tess2d.v[i].color, 1.0, 1.0, 1.0, 1.0);
	}

	R_DrawTexturedQuad();

	if (!image->has_alpha)
		GL_Enable(GL_BLEND);

	if (flags & PF_CROSSHAIR)
		GL_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void Draw_PicScaled(int x, int y, float scaleX, float scaleY, int flags, char *imageName, char *imageName2)
{
	image_t *image, *bumpImage;

	image = Draw_FindPic(imageName);

	if (strstr(imageName2, "null"))
		bumpImage = i_defBump;
	else
		bumpImage = Draw_FindPic(imageName2);

	if (!image)
		image = i_missingTexture;
	if (!bumpImage)
		bumpImage = i_defBump;

	Draw_ScaledPic(x, y, scaleX, scaleY, flags, image, bumpImage);
}


/*
=============
Draw_Fill

Fills a box of pixels with a single color
=============
*/
void Draw_Fill(int x, int y, int w, int h, float r, float g, float b, float a, bool loading) {

	GL_BindProgram(picProgram);
	qglUniform1i(U_PARAM_INT_0, PF_VERTEXCOLOR);

	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float*)r_newrefdef.orthoMatrix);

	VA_SetElem2(tess2d.v[0].pos, x, y);
	VA_SetElem2(tess2d.v[1].pos, x + w, y);
	VA_SetElem2(tess2d.v[2].pos, x + w, y + h);
	VA_SetElem2(tess2d.v[3].pos, x, y + h);

	if (!loading) {
		VA_SetElem4(tess2d.v[0].color, r, g, b, a);
		VA_SetElem4(tess2d.v[1].color, r, g, b, a);
		VA_SetElem4(tess2d.v[2].color, r, g, b, a);
		VA_SetElem4(tess2d.v[3].color, r, g, b, a);
	}
	else {
		VA_SetElem4(tess2d.v[0].color, 0.5, 0.0, 0.0, 0.25);
		VA_SetElem4(tess2d.v[1].color, 0.0, 0.5, 0.0, 0.75);
		VA_SetElem4(tess2d.v[2].color, 0.0, 0.5, 0.0, 0.75);
		VA_SetElem4(tess2d.v[3].color, 0.5, 0.0, 0.0, 0.25);
	}
	R_DrawTexturedQuad();
}

/*
=================================
Draw_StretchRaw - draw cinematics
with scanline postprocessing
=================================
*/
extern unsigned r_rawpalette[256];

void Draw_StretchRaw(int x, int y, int w, int h, int rawWidth, int rawHeight, byte* data)
{
	static uint	image32[256 * 256];
	int			i, j, trows, tex = 0;
	byte		*source;
	int			frac, fracstep;
	float		hscale;
	int			row;
	unsigned	*dest;

	qglClearBufferfv(GL_COLOR, 0, clearColor);

	memset(image32, 0, sizeof(image32));

	hscale = rawHeight / 256.0;
	trows = 256;

	for (i = 0; i < trows; i++)
	{
		row = (int)(i * hscale);

		if (row > rawHeight)
			break;

		source = data + rawWidth * row;
		dest = &image32[i * 256];
		fracstep = rawWidth * 256;
		frac = fracstep >> 1;

		for (j = 0; j < 256; j++)
		{
			dest[j] = r_rawpalette[source[frac >> 16]];
			frac += fracstep;
		}
	}

	// update cin texture
	glTextureSubImage2D(i_cinematic->texnum, 0, 0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, image32);

	// setup program
	GL_BindProgram(picProgram);
	qglUniform1i(U_PARAM_INT_0, PF_TECHCOLOR | PF_MEDIANFILTER | PF_SCANLINE);
	GL_SetBindlessTexture(U_TMU0, i_cinematic->handle);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float*)r_newrefdef.orthoMatrix);

	VA_SetElem2(tess2d.v[0].pos, x, y);
	VA_SetElem2(tess2d.v[1].pos, x + w, y);
	VA_SetElem2(tess2d.v[2].pos, x + w, y + h);
	VA_SetElem2(tess2d.v[3].pos, x, y + h);

	VA_SetElem2(tess2d.v[0].tc, 0.0, 0.0);
	VA_SetElem2(tess2d.v[1].tc, 1.0, 0.0);
	VA_SetElem2(tess2d.v[2].tc, 1.0, 1.0);
	VA_SetElem2(tess2d.v[3].tc, 0.0, 1.0);

	R_DrawTexturedQuad();
}