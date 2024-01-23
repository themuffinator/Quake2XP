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

// draw.c   test

#include "r_local.h"

/*
===============
R_LoadFont
===============
*/

void R_Init2D(void)
{
	menuFont = R_LoadDDS("gfx/fonts/engfont.dds", it_nomips);

	if (!menuFont)
		menuFont = GL_FindImage("pics/conchars.pcx", it_nomips);

	if (!menuFont)
		VID_Error(ERR_FATAL, "couldn't load pics/conchars");

	consFont = R_LoadDDS("gfx/fonts/intfont.dds", it_nomips);
	if (!consFont)
		consFont = r_missingTexture;
}

void R_DrawTexturedQuad() {

	GL_BindVAO(vao.tess2d);
	GL_BindVBO(vbo.tess2dVbo);
	qglInvalidateBufferData(GL_ARRAY_BUFFER);
	qglBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(tess2d), &tess2d);
	GL_DrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
	GL_BindNullVAO();
}

void R_Flush2D() {

	if (!tess2dArray.numVerts)
		return;

	GL_BindProgram(genericProgram);
	qglUniform1i(U_2D_PICS, 1);
	qglUniform1i(U_CONSOLE_BACK, 0);
	qglUniform1i(U_FRAG_COLOR, 0);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	GL_SetBindlessTexture(U_TMU0, tess2dArray.handle);

	GL_BindVAO(vao.tess2dArray);
	GL_BindVBO(vbo.tess2dArrayVbo);

	qglInvalidateBufferData(GL_ARRAY_BUFFER);
	qglBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(tess2dArray), &tess2dArray);

	GL_DrawElements(GL_TRIANGLES, 6 * tess2dArray.numSymbols, GL_UNSIGNED_INT, NULL);

	tess2dArray.numVerts = 0;
	tess2dArray.numSymbols = 0;

	GL_BindNullVAO();
}

void R_AddCharsToList(int x, int y, int scale, unsigned char num, image_t *inTex) {
	int row, col, x1, y1;
	float frow, fcol, size;

	if (tess2dArray.numVerts >= MAX_VERTICES_2D || (tess2dArray.numVerts && tess2dArray.handle != inTex->handle)) {
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

	tess2dArray.numVerts = tess2dArray.numSymbols << 2;

	VA_SetElem2(tess2dArray.v[tess2dArray.numVerts + 0].tc, fcol, frow);
	VA_SetElem2(tess2dArray.v[tess2dArray.numVerts + 1].tc, fcol +size, frow);
	VA_SetElem2(tess2dArray.v[tess2dArray.numVerts + 2].tc, fcol +size, frow + size);
	VA_SetElem2(tess2dArray.v[tess2dArray.numVerts + 3].tc, fcol, frow + size);

	VA_SetElem2(tess2dArray.v[tess2dArray.numVerts + 0].pos, x,	y);
	VA_SetElem2(tess2dArray.v[tess2dArray.numVerts + 1].pos, x1, y);
	VA_SetElem2(tess2dArray.v[tess2dArray.numVerts + 2].pos, x1, y1);
	VA_SetElem2(tess2dArray.v[tess2dArray.numVerts + 3].pos, x,	y1);

	for (int i = 0; i < 4; i++)
		VA_SetElem4(tess2dArray.v[tess2dArray.numVerts + i].color, gl_state.fontColor[0], gl_state.fontColor[1], gl_state.fontColor[2], gl_state.fontColor[3]);

	tess2dArray.numSymbols++;
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
		if (gl != r_missingTexture)
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


/*
=============
Draw_StretchPic
=============
*/

void Draw_StretchPic2(int x, int y, int w, int h, image_t* gl)
{
	float		offsX, offsY;
	float		woh = (float)vid.width / (float)vid.height;
	qboolean	console, menu;

	if (!gl) {
		Com_Printf("NULL pic in Draw_StretchPic\n");
		return;
	}
	if (strstr(gl->name, "conback"))
		console = qtrue;
	else
		console = qfalse;
	if (strstr(gl->name, "menuback"))
		menu = qtrue;
	else
		menu = qfalse;

	GL_BindProgram(genericProgram);

	qglUniform1i(U_CONSOLE_BACK, 0);
	qglUniform1i(U_FRAG_COLOR, 0);

	if (woh < WIDE_SCREEN_16x9) {  // quad screen
		offsX = (WIDTH_FHD - (HEIGHT_FHD * woh)) / (WIDTH_FHD * 2.0);
		offsY = 0;
	}
	else if (woh > WIDE_SCREEN_16x9) {   // super wide screen (21 x 9)
		offsX = 0;
		offsY = (HEIGHT_FHD - (WIDTH_FHD / woh)) / (HEIGHT_FHD * 2.0);
	}
	else {
		offsX = offsY = 0;
	}
	if(w==h)
		offsX = offsY = 0;

	if (console) {
		qglUniform1i(U_CONSOLE_BACK, 1);

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

		qglUniform4fv(U_PARAM_VEC4_0, 1, lPos);
	}
	else {
		qglUniform1i(U_2D_PICS, 1);
	}
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float*)r_newrefdef.orthoMatrix);
	qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);

	GL_SetBindlessTexture(U_TMU0, gl->handle);
	GL_SetBindlessTexture(U_TMU1, r_conBump->handle);

	VA_SetElem2(tess2d.v[0].pos, x, y);
	VA_SetElem2(tess2d.v[1].pos, x + w, y);
	VA_SetElem2(tess2d.v[2].pos, x + w, y + h);
	VA_SetElem2(tess2d.v[3].pos, x, y + h);
		
	VA_SetElem2(tess2d.v[0].tc, gl->sl + offsX, gl->tl + offsY);
	VA_SetElem2(tess2d.v[1].tc, gl->sh - offsX, gl->tl + offsY);
	VA_SetElem2(tess2d.v[2].tc, gl->sh - offsX, gl->th - offsY);
	VA_SetElem2(tess2d.v[3].tc, gl->sl + offsX, gl->th - offsY);

	for (int i = 0; i < 4; i++)
		VA_SetElem4(tess2d.v[i].color, 1.0, 1.0, 1.0, 1.0);

	R_DrawTexturedQuad();
}



void Draw_StretchPic(int x, int y, int w, int h, char* pic)
{
	qboolean cons = 0;
	image_t* gl;

	gl = Draw_FindPic(pic);

	if (!gl) {
		Com_Printf("Can't find pic: %s\n", pic);
		return;
	}
	Draw_StretchPic2(x, y, w, h, gl);
}

float loadScreenColorFade;
float loadingLod;

void Draw_LoadingScreen2(int x, int y, int w, int h, image_t* gl)
{
	float offsX, offsY;
	float woh = (float)vid.width / (float)vid.height;

	if (!gl) {
		Com_Printf("NULL pic in Draw_LoadingScreen2\n");
		return;
	}

	if (woh < WIDE_SCREEN_16x9) {  // quad screen
		offsX = (WIDTH_FHD - (HEIGHT_FHD * woh)) / (WIDTH_FHD * 2.0);
		offsY = 0;
	}
	else if (woh > WIDE_SCREEN_16x9) {   // super wide screen (21 x 9)
		offsX = 0;
		offsY = (HEIGHT_FHD - (WIDTH_FHD / woh)) / (HEIGHT_FHD * 2.0);
	}
	else {
		offsX = offsY = 0;
	}

	GL_BindProgram(loadingProgram);

	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float*)r_newrefdef.orthoMatrix);
	qglUniform1f(U_PARAM_FLOAT_0, loadingLod);
	qglUniform1f(U_PARAM_FLOAT_1, loadScreenColorFade);
	qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);

	GL_SetBindlessTexture(U_TMU0, gl->handle);

	VA_SetElem2(tess2d.v[0].pos, x, y);
	VA_SetElem2(tess2d.v[1].pos, x + w, y);
	VA_SetElem2(tess2d.v[2].pos, x + w, y + h);
	VA_SetElem2(tess2d.v[3].pos, x, y + h);

	VA_SetElem2(tess2d.v[0].tc, gl->sl + offsX, gl->tl + offsY);
	VA_SetElem2(tess2d.v[1].tc, gl->sh - offsX, gl->tl + offsY);
	VA_SetElem2(tess2d.v[2].tc, gl->sh - offsX, gl->th - offsY);
	VA_SetElem2(tess2d.v[3].tc, gl->sl + offsX, gl->th - offsY);

	R_DrawTexturedQuad();
}

void Draw_LoadingScreen(int x, int y, int w, int h, char* pic)
{
	image_t* gl;

	gl = GL_FindImage(pic + 1, it_mipmap);
	Draw_LoadingScreen2(x, y, w, h, gl);
}

/*
=============
Draw_Pic
=============
*/


void Draw_ScaledPic(int x, int y, float sX, float sY, image_t* gl)
{
	int w, h;

	if (!gl) {
		Com_Printf("NULL pic in Draw_Pic\n");
		return;
	}

	w = gl->width * sX * gl->picScale_w;
	h = gl->height * sY * gl->picScale_h;

	if (!gl->has_alpha)
		GL_Disable(GL_BLEND);

	if (strstr(gl->name, "chx")) { // crosshair hack
		GL_Enable(GL_BLEND);
		GL_BlendFunc(GL_ONE, GL_ONE);
		w = gl->width * sX;
		h = gl->height * sY;
	}

	GL_BindProgram(genericProgram);
	qglUniform1i(U_2D_PICS, 1);
	qglUniform1i(U_CONSOLE_BACK, 0);
	qglUniform1i(U_FRAG_COLOR, 0);

	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float*)r_newrefdef.orthoMatrix);

	GL_SetBindlessTexture(U_TMU0, gl->handle);

	VA_SetElem2(tess2d.v[0].tc, gl->sl, gl->tl);
	VA_SetElem2(tess2d.v[1].tc, gl->sh, gl->tl);
	VA_SetElem2(tess2d.v[2].tc, gl->sh, gl->th);
	VA_SetElem2(tess2d.v[3].tc, gl->sl, gl->th);

	VA_SetElem2(tess2d.v[0].pos, x, y);
	VA_SetElem2(tess2d.v[1].pos, x + w, y);
	VA_SetElem2(tess2d.v[2].pos, x + w, y + h);
	VA_SetElem2(tess2d.v[3].pos, x, y + h);


	for (int i = 0; i < 4; i++) {
		if (strstr(gl->name, "chx"))
			VA_SetElem4(tess2d.v[i].color, hColor[0], hColor[1], hColor[2], 1.0);
		else
			VA_SetElem4(tess2d.v[i].color, 1.0, 1.0, 1.0, 1.0);
	}

	R_DrawTexturedQuad();

	if (!gl->has_alpha)
		GL_Enable(GL_BLEND);

	if (strstr(gl->name, "chx"))
		GL_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Draw_ScaledBumpPic(int x, int y, float sX, float sY, image_t* gl, image_t* gl2)
{
	int w, h;
	
	if (!r_bump2D->integer)
		return;

	if (!gl2)
		gl2 = r_defBump;

	if (strstr(gl->name, "chx"))
		return;

	w = gl->width	* sX * gl->picScale_w;
	h = gl->height	* sY * gl->picScale_h;

	GL_BlendFunc(GL_ONE, GL_ONE); // use addative alpha blending

	GL_BindProgram(light2dProgram);

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
	lPos[3] = lPos[3] * 0.5 + 0.5;

	qglUniform4fv(U_PARAM_VEC4_0, 1, lPos);

	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float*)r_newrefdef.orthoMatrix);

	GL_SetBindlessTexture(U_TMU0, gl->handle);
	GL_SetBindlessTexture(U_TMU1, gl2->handle);

	VA_SetElem2(tess2d.v[0].tc, gl->sl, gl->tl);
	VA_SetElem2(tess2d.v[1].tc, gl->sh, gl->tl);
	VA_SetElem2(tess2d.v[2].tc, gl->sh, gl->th);
	VA_SetElem2(tess2d.v[3].tc, gl->sl, gl->th);

	VA_SetElem2(tess2d.v[0].pos, x, y);
	VA_SetElem2(tess2d.v[1].pos, x + w, y);
	VA_SetElem2(tess2d.v[2].pos, x + w, y + h);
	VA_SetElem2(tess2d.v[3].pos, x, y + h);

	R_DrawTexturedQuad();

	GL_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Draw_PicScaled(int x, int y, float scale_x, float scale_y, char* pic)
{
	image_t* gl;

	gl = Draw_FindPic(pic);
	if (!gl) {
		gl = r_missingTexture;
	}
	Draw_ScaledPic(x, y, scale_x, scale_y, gl);
}

void Draw_PicBumpScaled(int x, int y, float scale_x, float scale_y, char* pic, char* pic2)
{
	image_t* gl;
	image_t* gl2;

	if (!r_bump2D->integer)
		return;

	gl = Draw_FindPic(pic);
	if (!gl) {
		gl = r_missingTexture;
	}

	gl2 = Draw_FindPic(pic2);
	if (!gl2) {
		gl2 = r_defBump;
	}
	Draw_ScaledBumpPic(x, y, scale_x, scale_y, gl, gl2);
}

/*
=============
Draw_TileClear

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
void Draw_TileClear2(int x, int y, int w, int h, image_t* image)
{
	if (!image) {
		Com_Printf("NULL pic in Draw_TileClear\n");
		return;
	}

	GL_BindProgram(genericProgram);
	qglUniform1i(U_2D_PICS, 1);
	qglUniform1i(U_CONSOLE_BACK, 0);
	qglUniform1i(U_FRAG_COLOR, 0);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float*)r_newrefdef.orthoMatrix);

	GL_SetBindlessTexture(U_TMU0, image->handle);

	VA_SetElem2(tess2d.v[0].tc, x / 64.0, y / 64.0);
	VA_SetElem2(tess2d.v[1].tc, (x + w) / 64.0, y / 64.0);
	VA_SetElem2(tess2d.v[2].tc, (x + w) / 64.0, y / 64.0);
	VA_SetElem2(tess2d.v[3].tc, x / 64.0, (y + h) / 64.0);

	VA_SetElem2(tess2d.v[0].pos, x, y);
	VA_SetElem2(tess2d.v[1].pos, x + w, y);
	VA_SetElem2(tess2d.v[2].pos, x + w, y + h);
	VA_SetElem2(tess2d.v[3].pos, x, y + h);

	for (int i = 0; i < 4; i++)
		VA_SetElem4(tess2d.v[i].color, 1.0, 1.0, 1.0, 1.0);

	R_DrawTexturedQuad();
}

void Draw_TileClear(int x, int y, int w, int h, char* pic)
{
	image_t* image;
	image = Draw_FindPic(pic);

	if (!image) {
		Com_Printf("Can't find pic: %s\n", pic);
		return;
	}
	Draw_TileClear2(x, y, w, h, image);
}


/*
=============
Draw_Fill

Fills a box of pixels with a single color
=============
*/
void Draw_Fill(int x, int y, int w, int h, float r, float g, float b, float a, qboolean loading) {

	GL_BindProgram(genericProgram);
	qglUniform1i(U_2D_PICS, 0);
	qglUniform1i(U_CONSOLE_BACK, 0);
	qglUniform1i(U_FRAG_COLOR, 1);

	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float*)r_newrefdef.orthoMatrix);

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

	qglClear(GL_COLOR_BUFFER_BIT);

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
	glTextureSubImage2D(r_cinImage->texnum, 0, 0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, image32);

	// setup program
	GL_BindProgram(cinProgram);

	GL_SetBindlessTexture(U_TMU0, r_cinImage->handle);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float*)r_newrefdef.orthoMatrix);
	qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);

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