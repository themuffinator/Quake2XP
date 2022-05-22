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
index_t quadIndeces[] = { 0, 1, 2, 0, 2, 3 };
/*
===============
R_LoadFont
===============
*/

void R_Init2D(void)
{
	draw_chars = GL_FindImage("gfx/fonts/engfont.tga", it_pic);

	if (!draw_chars)
		draw_chars = GL_FindImage("pics/conchars.pcx", it_pic);

	if (!draw_chars)
		VID_Error(ERR_FATAL, "couldn't load pics/conchars");

	draw_charsInt = GL_FindImage("gfx/fonts/intfont.tga", it_pic);
	if (!draw_charsInt)
		draw_charsInt = r_notexture;
}

void Draw_CharScaled(int x, int y, float scale_x, float scale_y, unsigned char num)
{
	int row, col;
	float frow, fcol, size;

	num &= 255;

	if ((num & 127) == 32)
		return;					// space

	if (y <= -8 * scale_y)
		return;					// totally off screen

	// shadow offcets
	int x2 = x + 2;
	int y2 = y + 2;

	row = num >> 4;
	col = num & 15;

	frow = row * 0.0625;
	fcol = col * 0.0625;
	size = 0.0625;

	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TEX0);
	qglEnableVertexAttribArray(ATT_COLOR);

	qglVertexAttribPointer(ATT_POSITION, 2 , GL_FLOAT, qfalse, 0, vertCoord);
	qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, texCoord);
	qglVertexAttribPointer(ATT_COLOR, 4, GL_FLOAT, qfalse, 0, colorCoord);

	GL_BindProgram(genericProgram);
	qglUniform1i(U_2D_PICS, 1);
	qglUniform1i(U_CONSOLE_BACK, 0);
	qglUniform1i(U_FRAG_COLOR, 0);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float*)r_newrefdef.orthoMatrix);

	GL_SetBindlessTexture(U_TMU0, draw_chars->handle);

	VA_SetElem2(texCoord[0], fcol, frow);
	VA_SetElem2(texCoord[1], fcol + size, frow);
	VA_SetElem2(texCoord[2], fcol + size, frow + size);
	VA_SetElem2(texCoord[3], fcol, frow + size);

	//====== draw font shadow
	if (num != 129 && num != 18 && num != 19 && num != 20 && num != 24 && num != 25 && num != 26 && r_fontsShadow->integer) { // fields and sliders filter
		VA_SetElem2(vertCoord[0], x2, y2);
		VA_SetElem2(vertCoord[1], x2 + 8 * scale_x, y2);
		VA_SetElem2(vertCoord[2], x2 + 8 * scale_x, y2 + 8 * scale_y);
		VA_SetElem2(vertCoord[3], x2, y + 8 * scale_y);

		for (int i = 0; i < 4; i++)
			VA_SetElem4(colorCoord[i], 0.0, 0.0, 0.0, 1.0);

		qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, quadIndeces);
	}

	//====== draw regular font
	VA_SetElem2(vertCoord[0], x, y);
	VA_SetElem2(vertCoord[1], x + 8 * scale_x, y);
	VA_SetElem2(vertCoord[2], x + 8 * scale_x, y + 8 * scale_y);
	VA_SetElem2(vertCoord[3], x, y + 8 * scale_y);

	for (int i = 0; i < 4; i++)
		VA_SetElem4(colorCoord[i], gl_state.fontColor[0], gl_state.fontColor[1], gl_state.fontColor[2], gl_state.fontColor[3]);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, quadIndeces);

	qglDisableVertexAttribArray(ATT_POSITION);
	qglDisableVertexAttribArray(ATT_TEX0);
	qglDisableVertexAttribArray(ATT_COLOR);
}

void Draw_CharScaledInt(int x, int y, float scale_x, float scale_y, unsigned char num)
{
	int row, col;
	float frow, fcol, size;

	num &= 255;

	if ((num & 127) == 32)
		return;					// space

	if (y <= -8 * scale_y)
		return;					// totally off screen

	// shadow offcets
	int x2 = x + 2;
	int y2 = y + 2;

	row = num >> 4;
	col = num & 15;

	frow = row * 0.0625;
	fcol = col * 0.0625;
	size = 0.0625;

	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TEX0);
	qglEnableVertexAttribArray(ATT_COLOR);

	qglVertexAttribPointer(ATT_POSITION, 2 , GL_FLOAT, qfalse, 0, vertCoord);
	qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, texCoord);
	qglVertexAttribPointer(ATT_COLOR, 4, GL_FLOAT, qfalse, 0, colorCoord);

	GL_BindProgram(genericProgram);
	qglUniform1i(U_2D_PICS, 1);
	qglUniform1i(U_CONSOLE_BACK, 0);
	qglUniform1i(U_FRAG_COLOR, 0);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float*)r_newrefdef.orthoMatrix);

	GL_SetBindlessTexture(U_TMU0, draw_charsInt->handle);

	VA_SetElem2(texCoord[0], fcol, frow);
	VA_SetElem2(texCoord[1], fcol + size, frow);
	VA_SetElem2(texCoord[2], fcol + size, frow + size);
	VA_SetElem2(texCoord[3], fcol, frow + size);

	//====== draw font shadow
	if (num != 129 && num != 18 && num != 19 && num != 20 && num != 24 && num != 25 && num != 26 && r_fontsShadow->integer) { // fields and sliders filter
		VA_SetElem2(vertCoord[0], x2, y2);
		VA_SetElem2(vertCoord[1], x2 + 8 * scale_x, y2);
		VA_SetElem2(vertCoord[2], x2 + 8 * scale_x, y2 + 8 * scale_y);
		VA_SetElem2(vertCoord[3], x2, y + 8 * scale_y);

		for (int i = 0; i < 4; i++)
			VA_SetElem4(colorCoord[i], 0.0, 0.0, 0.0, 1.0);

		qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, quadIndeces);
	}

	//====== draw regular font
	VA_SetElem2(vertCoord[0], x, y);
	VA_SetElem2(vertCoord[1], x + 8 * scale_x, y);
	VA_SetElem2(vertCoord[2], x + 8 * scale_x, y + 8 * scale_y);
	VA_SetElem2(vertCoord[3], x, y + 8 * scale_y);

	for (int i = 0; i < 4; i++)
		VA_SetElem4(colorCoord[i], gl_state.fontColor[0], gl_state.fontColor[1], gl_state.fontColor[2], gl_state.fontColor[3]);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, quadIndeces);

	qglDisableVertexAttribArray(ATT_POSITION);
	qglDisableVertexAttribArray(ATT_TEX0);
	qglDisableVertexAttribArray(ATT_COLOR);
}

void Draw_StringShadow(int x, int y, float scale_x, float scale_y, unsigned char* s)
{
	int px, py, row, col, num, counter, quadCounter;
	float frow, fcol, size;

	px = x + 2;
	py = y + 2;

	size = 0.0625;
	counter = 0;

	while (*s) {
		num = *s++;

		if ((num & 127) == 32) {  // space
			px += 6 * scale_x;
			continue;
		}

		if (y <= -6) {			// totally off screen
			px += 6 * scale_x;
			continue;
		}

		row = num >> 4;
		col = num & 15;

		frow = row * 0.0625;
		fcol = col * 0.0625;

		quadCounter = counter << 2;

		VA_SetElem2(texCoord[quadCounter + 0], fcol, frow);
		VA_SetElem2(texCoord[quadCounter + 1], fcol + size, frow);
		VA_SetElem2(texCoord[quadCounter + 2], fcol + size, frow + size);
		VA_SetElem2(texCoord[quadCounter + 3], fcol, frow + size);

		VA_SetElem2(vertCoord[quadCounter + 0], px, py);
		VA_SetElem2(vertCoord[quadCounter + 1], px + 8 * scale_x, py);
		VA_SetElem2(vertCoord[quadCounter + 2], px + 8 * scale_x, py + 8 * scale_y);
		VA_SetElem2(vertCoord[quadCounter + 3], px, py + 8 * scale_y);

		for (int i = 0; i < 4; i++)
			VA_SetElem4(colorCoord[quadCounter + i], 0.0, 0.0, 0.0, 1.0);

		px += 6 * scale_x;
		counter++;

		if (counter == MAX_DRAW_STRING_LENGTH) {
			qglDrawElements(GL_TRIANGLES, 6 * counter, GL_UNSIGNED_SHORT, NULL);
			counter = 0;
		}
	}

	if (counter)
		qglDrawElements(GL_TRIANGLES, 6 * counter, GL_UNSIGNED_SHORT, NULL);
}


void Draw_StringScaled(int x, int y, float scale_x, float scale_y, const char* str, qboolean international)
{
	int px, py, row, col, num, counter, quadCounter, i;
	float frow, fcol, size;
	unsigned char* s = (unsigned char*)str;

	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadString);
	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TEX0);
	qglEnableVertexAttribArray(ATT_COLOR);

	qglVertexAttribPointer(ATT_POSITION, 2 , GL_FLOAT, qfalse, 0, vertCoord);
	qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, texCoord);
	qglVertexAttribPointer(ATT_COLOR, 4, GL_FLOAT, qfalse, 0, colorCoord);

	GL_BindProgram(genericProgram);
	qglUniform1i(U_2D_PICS, 1);
	qglUniform1i(U_CONSOLE_BACK, 0);
	qglUniform1i(U_FRAG_COLOR, 0);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float*)r_newrefdef.orthoMatrix);

	if(international)
		GL_SetBindlessTexture(U_TMU0, draw_charsInt->handle);
	else 
		GL_SetBindlessTexture(U_TMU0, draw_chars->handle);

	if(r_fontsShadow->integer)
		Draw_StringShadow(x, y, scale_x, scale_y, s);

	px = x;
	py = y;

	size = 0.0625;
	counter = 0;

	while (*s) {
		num = *s++;

		if ((num & 127) == 32) {        // space
			px += 6 * scale_x;
			continue;
		}

		if (y <= -6) {                  // totally off screen
			px += 6 * scale_x;
			continue;
		}

		row = num >> 4;
		col = num & 15;

		frow = row * 0.0625;
		fcol = col * 0.0625;

		quadCounter = counter << 2;

		VA_SetElem2(texCoord[quadCounter + 0], fcol, frow);
		VA_SetElem2(texCoord[quadCounter + 1], fcol + size, frow);
		VA_SetElem2(texCoord[quadCounter + 2], fcol + size, frow + size);
		VA_SetElem2(texCoord[quadCounter + 3], fcol, frow + size);

		VA_SetElem2(vertCoord[quadCounter + 0], px, py);
		VA_SetElem2(vertCoord[quadCounter + 1], px + 8 * scale_x, py);
		VA_SetElem2(vertCoord[quadCounter + 2], px + 8 * scale_x, py + 8 * scale_y);
		VA_SetElem2(vertCoord[quadCounter + 3], px, py + 8 * scale_y);

		for (i = 0; i < 4; i++)
			VA_SetElem4(colorCoord[quadCounter + i], gl_state.fontColor[0], gl_state.fontColor[1], gl_state.fontColor[2], gl_state.fontColor[3]);

		px += 6 * scale_x;
		counter++;

		if (counter == MAX_DRAW_STRING_LENGTH) {
			qglDrawElements(GL_TRIANGLES, 6 * counter, GL_UNSIGNED_SHORT, NULL);
			counter = 0;
		}
	}

	if (counter)
		qglDrawElements(GL_TRIANGLES, 6 * counter, GL_UNSIGNED_SHORT, NULL);

	qglDisableVertexAttribArray(ATT_POSITION);
	qglDisableVertexAttribArray(ATT_TEX0);
	qglDisableVertexAttribArray(ATT_COLOR);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
		Com_sprintf(fullname, sizeof(fullname), "pics/%s.pcx", name);
		gl = GL_FindImage(fullname, it_pic);
	}
	else
		gl = GL_FindImage(name + 1, it_pic);

	if (gl)
		if (gl != r_notexture)
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
	image_t* gl;

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

	qglVertexAttribPointer(ATT_POSITION, 2, GL_FLOAT, qfalse, 0, vertCoord);
	qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, texCoord);
	qglVertexAttribPointer(ATT_COLOR, 4, GL_FLOAT, qfalse, 0, colorCoord);

	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TEX0);
	qglEnableVertexAttribArray(ATT_COLOR);

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

	VA_SetElem2(vertCoord[0], x, y);
	VA_SetElem2(vertCoord[1], x + w, y);
	VA_SetElem2(vertCoord[2], x + w, y + h);
	VA_SetElem2(vertCoord[3], x, y + h);
		
	VA_SetElem2(texCoord[0], gl->sl + offsX, gl->tl + offsY);
	VA_SetElem2(texCoord[1], gl->sh - offsX, gl->tl + offsY);
	VA_SetElem2(texCoord[2], gl->sh - offsX, gl->th - offsY);
	VA_SetElem2(texCoord[3], gl->sl + offsX, gl->th - offsY);

	for (int i = 0; i < 4; i++)
		VA_SetElem4(colorCoord[i], 1.0, 1.0, 1.0, 1.0);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, quadIndeces);

	qglDisableVertexAttribArray(ATT_POSITION);
	qglDisableVertexAttribArray(ATT_TEX0);
	qglDisableVertexAttribArray(ATT_COLOR);
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

	VA_SetElem2(tess2d.data[0].pos, x, y);
	VA_SetElem2(tess2d.data[1].pos, x + w, y);
	VA_SetElem2(tess2d.data[2].pos, x + w, y + h);
	VA_SetElem2(tess2d.data[3].pos, x, y + h);

	VA_SetElem2(tess2d.data[0].texCoord, gl->sl + offsX, gl->tl + offsY);
	VA_SetElem2(tess2d.data[1].texCoord, gl->sh - offsX, gl->tl + offsY);
	VA_SetElem2(tess2d.data[2].texCoord, gl->sh - offsX, gl->th - offsY);
	VA_SetElem2(tess2d.data[3].texCoord, gl->sl + offsX, gl->th - offsY);

	glBindVertexArray(vao.draw2d);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_draw2d);

	qglBufferData(GL_ARRAY_BUFFER, sizeof(tess2d), NULL, GL_STREAM_DRAW); // buffer orphaning
	qglBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(tess2d), &tess2d);
	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

	glBindVertexArray(0);
	qglBindBuffer(GL_ARRAY_BUFFER, 0);
}


image_t* GL_FindImage2(char* name, imagetype_t type);

image_t* GL_FindPic(char* name)
{
	image_t* gl;

	gl = GL_FindImage2(name + 1, it_mipmap);

	if (gl) {
		if (gl != r_notexture)
			strcpy(gl->bare_name, name);
	}
	return gl;
}

void Draw_LoadingScreen(int x, int y, int w, int h, char* pic)
{
	image_t* gl;
	gl = GL_FindPic(pic);

	if (!gl) {
		Com_Printf("Can't find pic: %s\n", pic);
		return;
	}

	Draw_LoadingScreen2(x, y, w, h, gl);
}

/*
=============
Draw_Pic
=============
*/

void Draw_Pic2(int x, int y, image_t* gl)
{
	int w, h;

	if (!gl) {
		Com_Printf("NULL pic in Draw_Pic\n");
		return;
	}

	w = gl->width;
	h = gl->height;

	if (!gl->has_alpha)
		GL_Disable(GL_BLEND);

	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TEX0);
	qglEnableVertexAttribArray(ATT_COLOR);

	qglVertexAttribPointer(ATT_POSITION, 2 , GL_FLOAT, qfalse, 0, vertCoord);
	qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, texCoord);
	qglVertexAttribPointer(ATT_COLOR, 4, GL_FLOAT, qfalse, 0, colorCoord);

	GL_BindProgram(genericProgram);
	qglUniform1i(U_2D_PICS, 1);
	qglUniform1i(U_CONSOLE_BACK, 0);
	qglUniform1i(U_FRAG_COLOR, 0);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float*)r_newrefdef.orthoMatrix);

	GL_SetBindlessTexture(U_TMU0, gl->handle);

	VA_SetElem2(texCoord[0], gl->sl, gl->tl);
	VA_SetElem2(texCoord[1], gl->sh, gl->tl);
	VA_SetElem2(texCoord[2], gl->sh, gl->th);
	VA_SetElem2(texCoord[3], gl->sl, gl->th);

	VA_SetElem2(vertCoord[0], x, y);
	VA_SetElem2(vertCoord[1], x + gl->width, y);
	VA_SetElem2(vertCoord[2], x + gl->width, y + gl->height);
	VA_SetElem2(vertCoord[3], x, y + gl->height);

	for (int i = 0; i < 4; i++)
		VA_SetElem4(colorCoord[0], 1.0, 1.0, 1.0, 1.0);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, quadIndeces);


	if (!gl->has_alpha)
		GL_Enable(GL_BLEND);

	qglDisableVertexAttribArray(ATT_POSITION);
	qglDisableVertexAttribArray(ATT_TEX0);
	qglDisableVertexAttribArray(ATT_COLOR);
}

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


	if (strstr(gl->name, "chxp")) { // crosshair hack
		GL_Enable(GL_BLEND);
		GL_BlendFunc(GL_ONE, GL_ONE);
		w = gl->width * sX;
		h = gl->height * sY;
	}

	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TEX0);
	qglEnableVertexAttribArray(ATT_COLOR);

	qglVertexAttribPointer(ATT_POSITION, 2 , GL_FLOAT, qfalse, 0, vertCoord);
	qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, texCoord);
	qglVertexAttribPointer(ATT_COLOR, 4, GL_FLOAT, qfalse, 0, colorCoord);

	GL_BindProgram(genericProgram);
	qglUniform1i(U_2D_PICS, 1);
	qglUniform1i(U_CONSOLE_BACK, 0);
	qglUniform1i(U_FRAG_COLOR, 0);

	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float*)r_newrefdef.orthoMatrix);

	GL_SetBindlessTexture(U_TMU0, gl->handle);

	VA_SetElem2(texCoord[0], gl->sl, gl->tl);
	VA_SetElem2(texCoord[1], gl->sh, gl->tl);
	VA_SetElem2(texCoord[2], gl->sh, gl->th);
	VA_SetElem2(texCoord[3], gl->sl, gl->th);

	VA_SetElem2(vertCoord[0], x, y);
	VA_SetElem2(vertCoord[1], x + w, y);
	VA_SetElem2(vertCoord[2], x + w, y + h);
	VA_SetElem2(vertCoord[3], x, y + h);

	for (int i = 0; i < 4; i++)
		VA_SetElem4(colorCoord[i], 1.0, 1.0, 1.0, 1.0);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, quadIndeces);


	if (!gl->has_alpha)
		GL_Enable(GL_BLEND);

	if (strstr(gl->name, "chxp"))
		GL_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	qglDisableVertexAttribArray(ATT_POSITION);
	qglDisableVertexAttribArray(ATT_TEX0);
	qglDisableVertexAttribArray(ATT_COLOR);
}

void Draw_ScaledBumpPic(int x, int y, float sX, float sY, image_t* gl, image_t* gl2)
{
	int w, h;
	float lightShift;

	w = gl->width * sX * gl->picScale_w;
	h = gl->height * sY * gl->picScale_h;

	GL_BlendFunc(GL_ONE, GL_ONE); // use addative alpha blending

	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TEX0);

	qglVertexAttribPointer(ATT_POSITION, 2 , GL_FLOAT, qfalse, 0, vertCoord);
	qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, texCoord);

	GL_BindProgram(light2dProgram);

	lightShift = 66.6 * sin(Sys_Milliseconds() * 0.001f);
	qglUniform2f(U_PARAM_VEC2_0, lightShift, r_hudLighting->value);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float*)r_newrefdef.orthoMatrix);

	GL_SetBindlessTexture(U_TMU0, gl->handle);
	GL_SetBindlessTexture(U_TMU1, gl2->handle);

	VA_SetElem2(texCoord[0], gl->sl, gl->tl);
	VA_SetElem2(texCoord[1], gl->sh, gl->tl);
	VA_SetElem2(texCoord[2], gl->sh, gl->th);
	VA_SetElem2(texCoord[3], gl->sl, gl->th);

	VA_SetElem3(vertCoord[0], x, y, 1.0);
	VA_SetElem3(vertCoord[1], x + w, y, 1.0);
	VA_SetElem3(vertCoord[2], x + w, y + h, 1.0);
	VA_SetElem3(vertCoord[3], x, y + h, 1.0);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, quadIndeces);


	GL_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	qglDisableVertexAttribArray(ATT_POSITION);
	qglDisableVertexAttribArray(ATT_TEX0);
}

void Draw_Pic(int x, int y, char* pic)
{
	image_t* gl;

	gl = Draw_FindPic(pic);
	if (!gl) {
		Com_Printf("Can't find pic: %s\n", pic);
		return;
	}
	Draw_Pic2(x, y, gl);

}

void Draw_PicScaled(int x, int y, float scale_x, float scale_y, char* pic)
{
	image_t* gl;

	gl = Draw_FindPic(pic);
	if (!gl) {
		Com_Printf("Can't find pic: %s\n", pic);
		return;
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
		Com_Printf("Can't find pic: %s\n", pic);
		return;
	}

	gl2 = Draw_FindPic(pic2);
	if (!gl2) {
		Com_Printf("Can't find pic: %s\n", pic2);
		return;
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

	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TEX0);
	qglEnableVertexAttribArray(ATT_COLOR);

	qglVertexAttribPointer(ATT_POSITION, 2 , GL_FLOAT, qfalse, 0, vertCoord);
	qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, texCoord);
	qglVertexAttribPointer(ATT_COLOR, 4, GL_FLOAT, qfalse, 0, colorCoord);

	GL_BindProgram(genericProgram);
	qglUniform1i(U_2D_PICS, 1);
	qglUniform1i(U_CONSOLE_BACK, 0);
	qglUniform1i(U_FRAG_COLOR, 0);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float*)r_newrefdef.orthoMatrix);

	GL_SetBindlessTexture(U_TMU0, image->handle);

	VA_SetElem2(texCoord[0], x / 64.0, y / 64.0);
	VA_SetElem2(texCoord[1], (x + w) / 64.0, y / 64.0);
	VA_SetElem2(texCoord[2], (x + w) / 64.0, y / 64.0);
	VA_SetElem2(texCoord[3], x / 64.0, (y + h) / 64.0);

	VA_SetElem2(vertCoord[0], x, y);
	VA_SetElem2(vertCoord[1], x + w, y);
	VA_SetElem2(vertCoord[2], x + w, y + h);
	VA_SetElem2(vertCoord[3], x, y + h);

	for (int i = 0; i < 4; i++)
		VA_SetElem4(colorCoord[i], 1.0, 1.0, 1.0, 1.0);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, quadIndeces);

	qglDisableVertexAttribArray(ATT_POSITION);
	qglDisableVertexAttribArray(ATT_TEX0);
	qglDisableVertexAttribArray(ATT_COLOR);


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

	glBindVertexArray(vao.draw2d);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_draw2d);

	VA_SetElem2(tess2d.data[0].pos, x, y);
	VA_SetElem2(tess2d.data[1].pos, x + w, y);
	VA_SetElem2(tess2d.data[2].pos, x + w, y + h);
	VA_SetElem2(tess2d.data[3].pos, x, y + h);

	if (!loading) {
		VA_SetElem4(tess2d.data[0].colorCoord, r, g, b, a);
		VA_SetElem4(tess2d.data[1].colorCoord, r, g, b, a);
		VA_SetElem4(tess2d.data[2].colorCoord, r, g, b, a);
		VA_SetElem4(tess2d.data[3].colorCoord, r, g, b, a);
	}
	else {
		VA_SetElem4(tess2d.data[0].colorCoord, 0.5, 0.0, 0.0, 0.25);
		VA_SetElem4(tess2d.data[1].colorCoord, 0.0, 0.5, 0.0, 0.75);
		VA_SetElem4(tess2d.data[2].colorCoord, 0.0, 0.5, 0.0, 0.75);
		VA_SetElem4(tess2d.data[3].colorCoord, 0.5, 0.0, 0.0, 0.25);
	}

	qglBufferData(GL_ARRAY_BUFFER, sizeof(tess2d), NULL, GL_STREAM_DRAW); // buffer orphaning
	qglBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(tess2d), &tess2d);
	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

	glBindVertexArray(0);
	qglBindBuffer(GL_ARRAY_BUFFER, 0);
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
	byte* source;
	int			frac, fracstep;
	float		hscale;
	int			row;
	unsigned* dest;

	qglClearColor(0.0, 0.0, 0.0, 0.0);

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

	VA_SetElem2(tess2d.data[0].pos, x, y);
	VA_SetElem2(tess2d.data[1].pos, x + w, y);
	VA_SetElem2(tess2d.data[2].pos, x + w, y + h);
	VA_SetElem2(tess2d.data[3].pos, x, y + h);

	VA_SetElem2(tess2d.data[0].texCoord, 0.0, 0.0);
	VA_SetElem2(tess2d.data[1].texCoord, 1.0, 0.0);
	VA_SetElem2(tess2d.data[2].texCoord, 1.0, 1.0);
	VA_SetElem2(tess2d.data[3].texCoord, 0.0, 1.0);

	glBindVertexArray(vao.draw2d);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_draw2d);

	qglBufferData(GL_ARRAY_BUFFER, sizeof(tess2d), NULL, GL_STREAM_DRAW); // buffer orphaning
	qglBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(tess2d), &tess2d);
	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

	glBindVertexArray(0);
	qglBindBuffer(GL_ARRAY_BUFFER, 0);
}