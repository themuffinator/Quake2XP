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

// draw.c

#include "r_local.h"

image_t *draw_chars;

extern qboolean scrap_dirty;
void Scrap_Upload(void);

vec2_t	texCoord[MAX_VERTEX_ARRAY];
vec2_t	texCoord1[MAX_VERTEX_ARRAY];
vec3_t	vertCoord[MAX_VERTEX_ARRAY];
vec4_t	colorCoord[MAX_VERTEX_ARRAY];

/*
===============
R_LoadFont
===============
*/

void R_LoadFont(void)
{

	draw_chars = GL_FindImage("gfx/fonts/q3ext.tga", it_pic);
	if(!draw_chars)
		draw_chars = GL_FindImage("pics/conchars.pcx", it_pic);
	if(!draw_chars)
		VID_Error(ERR_FATAL, "couldn't load pics/conchars");

	GL_MBind(GL_TEXTURE0, draw_chars->texnum);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Set_FontShader(qboolean enable){

	if (enable){
		qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadTris);
		qglEnableVertexAttribArray(ATT_POSITION);
		qglEnableVertexAttribArray(ATT_TEX0);
		qglEnableVertexAttribArray(ATT_COLOR);

		qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, vertCoord);
		qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, texCoord);
		qglVertexAttribPointer(ATT_COLOR, 4, GL_FLOAT, qfalse, 0, colorCoord);

		GL_BindProgram(genericProgram, 0);
		qglUniform1i(gen_attribColors, 1);
		qglUniform1i(gen_attribConsole, 0);
		qglUniform1i(gen_sky, 0);
		qglUniform1i(gen_3d, 0);
		qglUniform1f(gen_colorModulate, r_textureColorScale->value);
		qglUniformMatrix4fv(gen_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);
	}
	else{

		GL_BindNullProgram();
		qglDisableVertexAttribArray(ATT_POSITION);
		qglDisableVertexAttribArray(ATT_TEX0);
		qglDisableVertexAttribArray(ATT_COLOR);
		qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

}

void Draw_CharScaled(int x, int y, float scale_x, float scale_y, unsigned char num)
{
	int row, col;
	float frow, fcol, size;

	num &= 255;

	if ((num & 127) == 32)
		return;					// space

	if (y <= -8*scale_y)
		return;					// totally off screen

	row = num >> 4;
	col = num & 15;

	frow = row * 0.0625;
	fcol = col * 0.0625;
	size = 0.0625;

	GL_MBind(GL_TEXTURE0, draw_chars->texnum);

	VA_SetElem2(texCoord[0], fcol, frow);
	VA_SetElem2(texCoord[1], fcol + size, frow);
	VA_SetElem2(texCoord[2], fcol + size, frow + size);
	VA_SetElem2(texCoord[3], fcol, frow + size);

	VA_SetElem2(vertCoord[0], x, y);
	VA_SetElem2(vertCoord[1], x + 8 * scale_x, y);
	VA_SetElem2(vertCoord[2], x + 8 * scale_x, y + 8 * scale_y);
	VA_SetElem2(vertCoord[3], x, y + 8 * scale_y);

	VA_SetElem4(colorCoord[0], gl_state.fontColor[0], gl_state.fontColor[1], gl_state.fontColor[2], gl_state.fontColor[3]);
	VA_SetElem4(colorCoord[1], gl_state.fontColor[0], gl_state.fontColor[1], gl_state.fontColor[2], gl_state.fontColor[3]);
	VA_SetElem4(colorCoord[2], gl_state.fontColor[0], gl_state.fontColor[1], gl_state.fontColor[2], gl_state.fontColor[3]);
	VA_SetElem4(colorCoord[3], gl_state.fontColor[0], gl_state.fontColor[1], gl_state.fontColor[2], gl_state.fontColor[3]);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
}



void Draw_StringScaled(int x, int y, float scale_x, float scale_y, const char *str)
{
	int px, py, row, col, num, counter, quadCounter;
	float frow, fcol, size;
	unsigned char *s = (unsigned char *)str;

	if (gl_state.currenttextures[gl_state.currenttmu] !=
		draw_chars->texnum) {
		GL_MBind(GL_TEXTURE0, draw_chars->texnum);
	}

	px = x;
	py = y;
	size = 0.0625;
	counter = 0;

	while (*s) {
		num = *s++;

		if ((num & 127) == 32) {        // space
			px += 8 * scale_x;
			continue;
		}

		if (y <= -8) {                  // totally off screen
			px += 8 * scale_x;
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

		VA_SetElem4(colorCoord[quadCounter + 0], gl_state.fontColor[0], gl_state.fontColor[1], gl_state.fontColor[2], gl_state.fontColor[3]);
		VA_SetElem4(colorCoord[quadCounter + 1], gl_state.fontColor[0], gl_state.fontColor[1], gl_state.fontColor[2], gl_state.fontColor[3]);
		VA_SetElem4(colorCoord[quadCounter + 2], gl_state.fontColor[0], gl_state.fontColor[1], gl_state.fontColor[2], gl_state.fontColor[3]);
		VA_SetElem4(colorCoord[quadCounter + 3], gl_state.fontColor[0], gl_state.fontColor[1], gl_state.fontColor[2], gl_state.fontColor[3]);

		px += 8 * scale_x;
		counter++;

		if (counter == MAX_DRAW_STRING_LENGTH)
		{       /// достигли лимита, невероятный случай! Это какой же широкий монитор должен быть?  :))
			qglDrawElements(GL_TRIANGLES, 6 * counter, GL_UNSIGNED_SHORT, NULL);    /// Нарисуем batch
			counter = 0;                                                            /// Начнём новый batch
		}
	}

	if (counter)
		qglDrawElements(GL_TRIANGLES, 6 * counter, GL_UNSIGNED_SHORT, NULL);
}

/*
=============
Draw_FindPic
=============
*/
image_t *Draw_FindPic(char *name)
{
	image_t *gl;
	char fullname[MAX_QPATH];

	if (name[0] != '/' && name[0] != '\\') {
		Com_sprintf(fullname, sizeof(fullname), "pics/%s.pcx", name);
		gl = GL_FindImage(fullname, it_pic);
	} else
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
void Draw_GetPicSize(int *w, int *h, char *pic)
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




/*
=============
Draw_StretchPic
=============
*/

void Draw_StretchPic2(int x, int y, int w, int h, image_t *gl)
{
	float		scroll = -13 * (r_newrefdef.time / 40.0);
	qboolean	console;

	if (!gl) {
		Com_Printf("NULL pic in Draw_StretchPic\n");
		return;
	}
	if (strstr(gl->name, "conback"))
		console = qtrue;
	else
		console = qfalse;

	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadTris);

	qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, vertCoord);
	qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, texCoord);
	qglVertexAttribPointer(ATT_TEX2, 2, GL_FLOAT, qfalse, 0, texCoord1);
	qglVertexAttribPointer(ATT_COLOR, 4, GL_FLOAT, qfalse, 0, colorCoord);

	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TEX0);
	qglEnableVertexAttribArray(ATT_TEX2);
	qglEnableVertexAttribArray(ATT_COLOR);

	VA_SetElem2(vertCoord[0], x, y);
	VA_SetElem2(vertCoord[1], x + w, y);
	VA_SetElem2(vertCoord[2], x + w, y + h);
	VA_SetElem2(vertCoord[3], x, y + h);

	VA_SetElem4(colorCoord[0], 1.0, 1.0, 1.0, 1.0);
	VA_SetElem4(colorCoord[1], 1.0, 1.0, 1.0, 1.0);
	VA_SetElem4(colorCoord[2], 1.0, 1.0, 1.0, 1.0);
	VA_SetElem4(colorCoord[3], 1.0, 1.0, 1.0, 1.0);

	GL_BindProgram(genericProgram, 0);
	
	qglUniform1i(gen_attribConsole, 0);
	qglUniform1i(gen_attribColors, 0);
	qglUniform1i(gen_sky, 0);
	qglUniform1i(gen_3d, 0);

	if (console){
		qglUniform1i(gen_attribConsole, 1);
	}
	else{
		qglUniform1i(gen_attribColors, 1);
	}
	if (strstr(gl->name, "menuback"))
		qglUniform1f(gen_colorModulate, 1);
	else
		qglUniform1f(gen_colorModulate, r_textureColorScale->value);

	qglUniformMatrix4fv(gen_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	if (scrap_dirty)
		Scrap_Upload();

		GL_MBind(GL_TEXTURE0, gl->texnum);
		VA_SetElem2(texCoord[0], gl->sl, gl->tl);
		VA_SetElem2(texCoord[1], gl->sh, gl->tl);
		VA_SetElem2(texCoord[2], gl->sh, gl->th);
		VA_SetElem2(texCoord[3], gl->sl, gl->th);

		GL_MBind(GL_TEXTURE1, r_scanline->texnum);
		VA_SetElem2(texCoord1[0], gl->sl, gl->tl - scroll);
		VA_SetElem2(texCoord1[1], gl->sh, gl->tl - scroll);
		VA_SetElem2(texCoord1[2], gl->sh, gl->th - scroll);
		VA_SetElem2(texCoord1[3], gl->sl, gl->th - scroll);

		qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

		GL_BindNullProgram();
		GL_SelectTexture(GL_TEXTURE0);
		qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		qglDisableVertexAttribArray(ATT_POSITION);
		qglDisableVertexAttribArray(ATT_TEX0);
		qglDisableVertexAttribArray(ATT_TEX2);
		qglDisableVertexAttribArray(ATT_COLOR);
}


void Draw_StretchPic(int x, int y, int w, int h, char *pic)
{
	qboolean cons = 0;
	image_t *gl;

	gl = Draw_FindPic(pic);

	if (!gl) {
		Com_Printf("Can't find pic: %s\n", pic);
		return;
	}
	Draw_StretchPic2(x, y, w, h, gl);
}

float loadScreenColorFade;

#define WIDTH_FHD 1920.0
#define HEIGHT_FHD 1080.0
#define WIDE_SCREEN_16x9  WIDTH_FHD / HEIGHT_FHD

void Draw_LoadingScreen2(int x, int y, int w, int h, image_t * gl)
{
	float offsX, offsY;
	float woh = (float)vid.width / (float)vid.height;

	if (!gl) {
		Com_Printf("NULL pic in Draw_LoadingScreen2\n");
		return;
	}

		if (woh < WIDE_SCREEN_16x9){  // quad screen
			offsX = (WIDTH_FHD - (HEIGHT_FHD * woh)) / (WIDTH_FHD * 2.0);
			offsY = 0;
		}
		else if (woh > WIDE_SCREEN_16x9){   // super wide screen (21 x 9)
			offsX = 0;
			offsY = (HEIGHT_FHD - (WIDTH_FHD / woh)) / (HEIGHT_FHD * 2.0);
		}
		else{
			offsX = offsY = 0;
		}

		GL_BindProgram(loadingProgram, 0);

		qglUniformMatrix4fv(ls_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);
		qglUniform1f(ls_fade, loadScreenColorFade);

		qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadTris);

		qglEnableVertexAttribArray(ATT_POSITION);
		qglEnableVertexAttribArray(ATT_TEX0);
		qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, vertCoord);
		qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, texCoord);

		GL_MBind(GL_TEXTURE0, gl->texnum);

		VA_SetElem2(texCoord[0], gl->sl + offsX, gl->tl + offsY);
		VA_SetElem2(texCoord[1], gl->sh - offsX, gl->tl + offsY);
		VA_SetElem2(texCoord[2], gl->sh - offsX, gl->th - offsY);
		VA_SetElem2(texCoord[3], gl->sl + offsX, gl->th - offsY);

		VA_SetElem2(vertCoord[0], x, y);
		VA_SetElem2(vertCoord[1], x + w, y);
		VA_SetElem2(vertCoord[2], x + w, y + h);
		VA_SetElem2(vertCoord[3], x, y + h);

		qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

		GL_BindNullProgram();
		qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		qglDisableVertexAttribArray(ATT_POSITION);
		qglDisableVertexAttribArray(ATT_TEX0);
}

void Draw_LoadingScreen(int x, int y, int w, int h, char *pic)
{
	image_t *gl;
	gl = Draw_FindPic(pic);
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

void Draw_Pic2(int x, int y, image_t * gl)
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

	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadTris);
	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TEX0);
	qglEnableVertexAttribArray(ATT_COLOR);
	
    qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, vertCoord);
	qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, texCoord);
	qglVertexAttribPointer(ATT_COLOR, 4, GL_FLOAT, qfalse, 0, colorCoord);
	
	GL_BindProgram(genericProgram, 0);
	qglUniform1i(gen_attribColors, 1);
	qglUniform1i(gen_attribConsole, 0);
	qglUniform1i(gen_sky, 0);
	qglUniform1i(gen_3d, 0);
	qglUniform1f(gen_colorModulate, r_textureColorScale->value);
	qglUniformMatrix4fv(gen_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	if (scrap_dirty)
		Scrap_Upload();

		GL_MBind(GL_TEXTURE0, gl->texnum);
				
		VA_SetElem2(texCoord[0],gl->sl, gl->tl);
		VA_SetElem2(texCoord[1],gl->sh, gl->tl);
		VA_SetElem2(texCoord[2],gl->sh, gl->th);
		VA_SetElem2(texCoord[3],gl->sl, gl->th);
		
		VA_SetElem2(vertCoord[0],x, y);
		VA_SetElem2(vertCoord[1],x + gl->width, y);
		VA_SetElem2(vertCoord[2],x + gl->width, y + gl->height);
		VA_SetElem2(vertCoord[3],x, y + gl->height);
		
		VA_SetElem4(colorCoord[0], 1.0, 1.0, 1.0, 1.0);
		VA_SetElem4(colorCoord[1], 1.0, 1.0, 1.0, 1.0);
		VA_SetElem4(colorCoord[2], 1.0, 1.0, 1.0, 1.0);
		VA_SetElem4(colorCoord[3], 1.0, 1.0, 1.0, 1.0);

		qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

	
	if (!gl->has_alpha)
		GL_Enable(GL_BLEND);

	GL_BindNullProgram();
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	qglDisableVertexAttribArray(ATT_POSITION);
	qglDisableVertexAttribArray(ATT_TEX0);
	qglDisableVertexAttribArray(ATT_COLOR);
}

void Draw_ScaledPic(int x, int y, float sX, float sY, image_t * gl)
{
	int w, h;

	if (!gl) {
		Com_Printf("NULL pic in Draw_Pic\n");
		return;
	}
	
	w = gl->width * sX *gl->picScale_w;
	h = gl->height * sY *gl->picScale_h;

	if (!gl->has_alpha)
		GL_Disable(GL_BLEND);
	
	
	if (strstr(gl->name, "chxp")){ // crosshair hack
		GL_Enable(GL_BLEND);
		GL_BlendFunc(GL_ONE, GL_ONE);
		w = gl->width * sX;
		h = gl->height * sY;
	}

	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadTris);
	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TEX0);
	qglEnableVertexAttribArray(ATT_COLOR);
	
    qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, vertCoord);
	qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, texCoord);
	qglVertexAttribPointer(ATT_COLOR, 4, GL_FLOAT, qfalse, 0, colorCoord);
	
	GL_BindProgram(genericProgram, 0);
	qglUniform1i(gen_attribColors, 1);
	qglUniform1i(gen_attribConsole, 0);
	qglUniform1i(gen_sky, 0);
	qglUniform1i(gen_3d, 0);

	qglUniform1f(gen_colorModulate, r_bump2D->value ? 1.0 : r_textureColorScale->value);
	qglUniformMatrix4fv(gen_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	if (scrap_dirty)
		Scrap_Upload();

		GL_MBind(GL_TEXTURE0, gl->texnum);
				
		VA_SetElem2(texCoord[0],gl->sl, gl->tl);
		VA_SetElem2(texCoord[1],gl->sh, gl->tl);
		VA_SetElem2(texCoord[2],gl->sh, gl->th);
		VA_SetElem2(texCoord[3],gl->sl, gl->th);
		
		VA_SetElem2(vertCoord[0],x, y);
		VA_SetElem2(vertCoord[1],x + w, y);
		VA_SetElem2(vertCoord[2],x + w, y + h);
		VA_SetElem2(vertCoord[3],x, y + h);
		
		VA_SetElem4(colorCoord[0], 1.0, 1.0, 1.0, 1.0);
		VA_SetElem4(colorCoord[1], 1.0, 1.0, 1.0, 1.0);
		VA_SetElem4(colorCoord[2], 1.0, 1.0, 1.0, 1.0);
		VA_SetElem4(colorCoord[3], 1.0, 1.0, 1.0, 1.0);

		qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

	
	if (!gl->has_alpha)
		GL_Enable(GL_BLEND);

	if (strstr(gl->name, "chxp"))
		GL_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	GL_BindNullProgram();
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	qglDisableVertexAttribArray(ATT_POSITION);
	qglDisableVertexAttribArray(ATT_TEX0);
	qglDisableVertexAttribArray(ATT_COLOR);
}

void Draw_ScaledBumpPic(int x, int y, float sX, float sY, image_t *gl, image_t *gl2)
{
	int w, h;
	float lightShift;

	w = gl->width * sX *gl->picScale_w;
	h = gl->height * sY *gl->picScale_h;

	GL_BlendFunc(GL_SRC_ALPHA, GL_ONE); // use addative alpha blending

	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadTris);
	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TEX0);

	qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, vertCoord);
	qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, texCoord);

	GL_BindProgram(light2dProgram, 0);

	lightShift = 66.6 * sin(Sys_Milliseconds() * 0.001f);
	qglUniform2f(light2d_params, lightShift, r_hudLighting->value);
	qglUniformMatrix4fv(light2d_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	if (scrap_dirty)
		Scrap_Upload();

	GL_MBind(GL_TEXTURE0, gl->texnum);
	GL_MBind(GL_TEXTURE1, gl2->texnum);

	VA_SetElem2(texCoord[0], gl->sl, gl->tl);
	VA_SetElem2(texCoord[1], gl->sh, gl->tl);
	VA_SetElem2(texCoord[2], gl->sh, gl->th);
	VA_SetElem2(texCoord[3], gl->sl, gl->th);

	VA_SetElem3(vertCoord[0], x, y, 1.0);
	VA_SetElem3(vertCoord[1], x + w, y, 1.0);
	VA_SetElem3(vertCoord[2], x + w, y + h, 1.0);
	VA_SetElem3(vertCoord[3], x, y + h, 1.0);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);


	GL_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GL_BindNullProgram();
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	qglDisableVertexAttribArray(ATT_POSITION);
	qglDisableVertexAttribArray(ATT_TEX0);
}

void Draw_Pic(int x, int y, char *pic)
{
	image_t *gl;

	gl = Draw_FindPic(pic);
	if (!gl) {
		Com_Printf("Can't find pic: %s\n", pic);
		return;
	}
	Draw_Pic2(x, y, gl);

}

void Draw_PicScaled(int x, int y, float scale_x, float scale_y, char *pic)
{
	image_t *gl;

	gl = Draw_FindPic(pic);
	if (!gl) {
		Com_Printf("Can't find pic: %s\n", pic);
		return;
	}
	Draw_ScaledPic(x, y, scale_x, scale_y, gl);
}

void Draw_PicBumpScaled(int x, int y, float scale_x, float scale_y, char *pic, char *pic2)
{
	image_t *gl;
	image_t *gl2;

	if (!r_bump2D->value)
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
void Draw_TileClear2(int x, int y, int w, int h, image_t * image)
{
	if (!image) {
		Com_Printf("NULL pic in Draw_TileClear\n");
		return;
	}

	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadTris);
	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TEX0);
	qglEnableVertexAttribArray(ATT_COLOR);

	qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, vertCoord);
	qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, texCoord);
	qglVertexAttribPointer(ATT_COLOR, 4, GL_FLOAT, qfalse, 0, colorCoord);

	GL_BindProgram(genericProgram, 0);
	qglUniform1i(gen_attribColors, 1);
	qglUniform1i(gen_attribConsole, 0);
	qglUniform1i(gen_sky, 0);
	qglUniform1i(gen_3d, 0);
	qglUniform1f(gen_colorModulate, r_textureColorScale->value);
	qglUniformMatrix4fv(gen_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);
	
	GL_MBind(GL_TEXTURE0, image->texnum);

	VA_SetElem2(texCoord[0], x / 64.0, y / 64.0);
	VA_SetElem2(texCoord[1], (x + w) / 64.0, y / 64.0);
	VA_SetElem2(texCoord[2], (x + w) / 64.0, y / 64.0);
	VA_SetElem2(texCoord[3], x / 64.0, (y + h) / 64.0);

	VA_SetElem2(vertCoord[0], x, y);
	VA_SetElem2(vertCoord[1], x + w, y);
	VA_SetElem2(vertCoord[2], x + w, y + h);
	VA_SetElem2(vertCoord[3], x, y + h);

	VA_SetElem4(colorCoord[0], 1.0, 1.0, 1.0, 1.0);
	VA_SetElem4(colorCoord[1], 1.0, 1.0, 1.0, 1.0);
	VA_SetElem4(colorCoord[2], 1.0, 1.0, 1.0, 1.0);
	VA_SetElem4(colorCoord[3], 1.0, 1.0, 1.0, 1.0);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

	GL_BindNullProgram();
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	qglDisableVertexAttribArray(ATT_POSITION);
	qglDisableVertexAttribArray(ATT_TEX0);
	qglDisableVertexAttribArray(ATT_COLOR);


}

void Draw_TileClear(int x, int y, int w, int h, char *pic)
{
	image_t *image;
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
void Draw_Fill(int x, int y, int w, int h, float r, float g, float b, float a)
{

	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadTris);
	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, vertCoord);
	
	GL_BindProgram(genericProgram, 0);
	qglUniform1i(gen_attribColors, 0);
	qglUniform1i(gen_attribConsole, 0);
	qglUniform1i(gen_sky, 0);
	qglUniform1i(gen_3d, 0);
	qglUniform4f(gen_color, r, g, b, a);
	qglUniform1f(gen_colorModulate, r_textureColorScale->value);
	qglUniformMatrix4fv(gen_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	VA_SetElem2(vertCoord[0], x, y);
	VA_SetElem2(vertCoord[1], x + w, y);
	VA_SetElem2(vertCoord[2], x + w, y + h);
	VA_SetElem2(vertCoord[3], x, y + h);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
	
	GL_BindNullProgram();
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	qglDisableVertexAttribArray(ATT_POSITION);
}

/*
=================================
Draw_StretchRaw - draw cinematics 
with scanline postprocessing
=================================
*/
extern unsigned r_rawpalette[256];

void Draw_StretchRaw (int sw, int sh, int w, int h, int cols, int rows, byte *data)
{
	unsigned	image32[256*256];
	int			i, j, trows;
	byte		*source;
	int			frac, fracstep;
	float		hscale;
	int			row, x0, y0, x1, y1;
	float		t;
	unsigned	*dest;

	qglClearColor(0.0, 0.0, 0.0, 1.0);

	// setup program
	GL_BindProgram(cinProgram, 0);

	GL_MBind(GL_TEXTURE0, 0);
	qglUniform2f(cin_params, w, cols);
	qglUniformMatrix4fv(cin_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	hscale = rows/256.0;
	trows = 256;
	t = rows*hscale / 256;

	for (i=0 ; i<trows ; i++)
	{
		row = (int)(i*hscale);
		
		if (row > rows)
			break;
		
		source = data + cols*row;
		dest = &image32[i*256];
		fracstep = cols*0x10000/256;
		frac = fracstep >> 1;

		for (j=0 ; j<256 ; j++)
		{
			dest[j] = r_rawpalette[source[frac>>16]];
			frac += fracstep;
		}
	}

	qglTexImage2D (GL_TEXTURE_2D, 0, GL_RGB8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, image32);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	x0 = sw;
	y0 = sh;
	x1 = sw+w;
	y1 = sh+h;

	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadTris);
	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TEX0);

	qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, vertCoord);
	qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, texCoord);

	VA_SetElem2(texCoord[0], 0, 0);
	VA_SetElem2(texCoord[1], 1, 0);
	VA_SetElem2(texCoord[2], 1, 1);
	VA_SetElem2(texCoord[3], 0, 1);

	VA_SetElem2(vertCoord[0], x0, y0);
	VA_SetElem2(vertCoord[1], x1, y0);
	VA_SetElem2(vertCoord[2], x1, y1);
	VA_SetElem2(vertCoord[3], x0, y1);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
	
	GL_BindNullProgram();
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	qglDisableVertexAttribArray(ATT_POSITION);
	qglDisableVertexAttribArray(ATT_TEX0);
}
