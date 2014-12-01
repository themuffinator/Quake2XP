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

vec2_t texCoord[MAX_VERTEX_ARRAY];
vec2_t texCoord1[MAX_VERTEX_ARRAY];
vec3_t vertCoord[MAX_VERTEX_ARRAY];
vec4_t colorCoord[MAX_VERTEX_ARRAY];

/*
===============
R_LoadFont
===============
*/


void R_LoadFont(void)
{

	draw_chars = GL_FindImage("gfx/fonts/q3ext.pcx", it_pic);
	if(!draw_chars)
		draw_chars = GL_FindImage("pics/conchars.pcx", it_pic);
	if(!draw_chars)
		VID_Error(ERR_FATAL, "couldn't load pics/conchars");

	GL_Bind(draw_chars->texnum);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


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

	GL_Blend(true, 0, 0);
	GL_Bind(draw_chars->texnum);

	qglBegin(GL_QUADS);
	qglTexCoord2f(fcol, frow);
	qglVertex2f(x, y);
	qglTexCoord2f(fcol + size, frow);
	qglVertex2f(x + 8*scale_x, y);
	qglTexCoord2f(fcol + size, frow + size);
	qglVertex2f(x + 8*scale_x, y + 8*scale_y);
	qglTexCoord2f(fcol, frow + size);
	qglVertex2f(x, y + 8*scale_y);
	qglEnd();

	GL_Blend(false, 0, 0);

}


void Draw_CharScaledShadow(int x, int y, float scale_x, float scale_y, unsigned char num)
{
	int row, col;
	float frow, fcol, size;
	
	return;

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
	GL_Blend(true, 0, 0);
	GL_Bind(draw_chars->texnum);

	qglColor3f(0.0, 0.0, 0.0);
	qglBegin(GL_QUADS);
	qglTexCoord2f(fcol, frow);
	qglVertex2f(x, y);
	qglTexCoord2f(fcol + size, frow);
	qglVertex2f(x + 9*scale_x, y);
	qglTexCoord2f(fcol + size, frow + size);
	qglVertex2f(x + 9*scale_x, y + 9*scale_y);
	qglTexCoord2f(fcol, frow + size);
	qglVertex2f(x, y + 9*scale_y);
	qglEnd();

	qglColor3f(1.0, 1.0, 1.0);
	GL_Blend(false, 0, 0);
}


void Draw_StringScaled(int x, int y, float scale_x, float scale_y, const char *str)
{
	int px, py, row, col, num;
	float frow, fcol, size;
	unsigned char *s = (unsigned char *) str;

	if (gl_state.currenttextures[gl_state.currenttmu] !=
		draw_chars->texnum) {
		GL_Bind(draw_chars->texnum);
	}

	qglEnable(GL_BLEND);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	px = x;
	py = y;
	size = 0.0625;
	
	qglBegin(GL_QUADS);
	while (*s) {
		num = *s++;

		if ((num & 127) == 32) {	// space
			px += 8;
			continue;
		}

		if (y <= -8) {			// totally off screen
			px += 8;
			continue;
		}

		row = num >> 4;
		col = num & 15;

		frow = row * 0.0625;
		fcol = col * 0.0625;

		qglTexCoord2f(fcol, frow);
		qglVertex2f(px, py);
		qglTexCoord2f(fcol + size, frow);
		qglVertex2f(px + 8*scale_x, py);
		qglTexCoord2f(fcol + size, frow + size);
		qglVertex2f(px + 8*scale_x, py + 8*scale_y);
		qglTexCoord2f(fcol, frow + size);
		qglVertex2f(px, py + 8*scale_y);
		px += 8*scale_x;
	}
	qglEnd();

	qglDisable(GL_BLEND);
}


void Draw_StringScaledShadow(int x, int y, float scale_x, float scale_y, const char *str)
{
	int px, py, row, col, num;
	float frow, fcol, size;
	unsigned char *s = (unsigned char *) str;
	
	return;

	if (gl_state.currenttextures[gl_state.currenttmu] !=
		draw_chars->texnum) {
		GL_Bind(draw_chars->texnum);
	}

	qglEnable(GL_BLEND);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	qglColor3f(0.0, 0.0, 0.0);

	px = x;
	py = y;
	size = 0.0625;
	
	qglBegin(GL_QUADS);
	while (*s) {
		num = *s++;

		if ((num & 127) == 32) {	// space
			px += 8;
			continue;
		}

		if (y <= -8) {			// totally off screen
			px += 8;
			continue;
		}

		row = num >> 4;
		col = num & 15;

		frow = row * 0.0625;
		fcol = col * 0.0625;

		qglTexCoord2f(fcol, frow);
		qglVertex2f(px, py);
		qglTexCoord2f(fcol + size, frow);
		qglVertex2f(px + 9*scale_x, py);
		qglTexCoord2f(fcol + size, frow + size);
		qglVertex2f(px + 9*scale_x, py + 9*scale_y);
		qglTexCoord2f(fcol, frow + size);
		qglVertex2f(px, py + 9*scale_y);
		px += 8*scale_x;
	}
	qglEnd();

	qglColor3f(1.0, 1.0, 1.0);
	qglDisable(GL_BLEND);
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

void Draw_StretchPic2(int x, int y, int w, int h, image_t *gl, qboolean cons)
{
	int			id;
	unsigned	defBits = 0;
	float		scroll = -13 * (r_newrefdef.time / 40.0);
	qboolean	console;

	if (!gl) {
		Com_Printf("NULL pic in Draw_StretchPic\n");
		return;
	}
	if (strstr(gl->name, "conback"))
		console = true;
	else
		console = false;

	qglEnableVertexAttribArray(ATRB_POSITION);
	qglEnableVertexAttribArray(ATRB_TEX0);
	qglEnableVertexAttribArray(ATRB_TEX2);
	qglEnableVertexAttribArray(ATRB_COLOR);

	qglVertexAttribPointer(ATRB_POSITION, 3, GL_FLOAT, false, 0, vertCoord);
	qglVertexAttribPointer(ATRB_TEX0, 2, GL_FLOAT, false, 0, texCoord);
	qglVertexAttribPointer(ATRB_TEX2, 2, GL_FLOAT, false, 0, texCoord1);
	qglVertexAttribPointer(ATRB_COLOR, 4, GL_FLOAT, false, 0, colorCoord);
	
	VA_SetElem2(vertCoord[0], x, y);
	VA_SetElem2(vertCoord[1], x + w, y);
	VA_SetElem2(vertCoord[2], x + w, y + h);
	VA_SetElem2(vertCoord[3], x, y + h);

	VA_SetElem4(colorCoord[0], 1.0, 1.0, 1.0, 1.0);
	VA_SetElem4(colorCoord[1], 1.0, 1.0, 1.0, 1.0);
	VA_SetElem4(colorCoord[2], 1.0, 1.0, 1.0, 1.0);
	VA_SetElem4(colorCoord[3], 1.0, 1.0, 1.0, 1.0);

	if (console)
		defBits = worldDefs.ConsoleBits;
	else
		defBits = worldDefs.AttribColorBits;

	GL_BindProgram(genericProgram, defBits);
	id = genericProgram->id[defBits];
	qglUniform1f(qglGetUniformLocation(id, "u_colorScale"), r_worldColorScale->value);

	if (scrap_dirty)
		Scrap_Upload();

		GL_MBind(GL_TEXTURE0_ARB, gl->texnum);
		qglUniform1i(qglGetUniformLocation(id, "u_map"), 0);
		VA_SetElem2(texCoord[0], gl->sl, gl->tl);
		VA_SetElem2(texCoord[1], gl->sh, gl->tl);
		VA_SetElem2(texCoord[2], gl->sh, gl->th);
		VA_SetElem2(texCoord[3], gl->sl, gl->th);

		GL_MBind(GL_TEXTURE1_ARB, r_scanline->texnum);
		qglUniform1i(qglGetUniformLocation(id, "u_map1"), 1);
		VA_SetElem2(texCoord1[0], gl->sl, gl->tl - scroll);
		VA_SetElem2(texCoord1[1], gl->sh, gl->tl - scroll);
		VA_SetElem2(texCoord1[2], gl->sh, gl->th - scroll);
		VA_SetElem2(texCoord1[3], gl->sl, gl->th - scroll);

		qglDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		GL_BindNullProgram();
		GL_SelectTexture(GL_TEXTURE0_ARB);

		qglDisableVertexAttribArray(ATRB_POSITION);
		qglDisableVertexAttribArray(ATRB_TEX0);
		qglDisableVertexAttribArray(ATRB_TEX2);
		qglDisableVertexAttribArray(ATRB_COLOR);
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
	Draw_StretchPic2(x, y, w, h, gl, cons);
}

float loadScreenColorFade;
void Draw_LoadingScreen2(int x, int y, int w, int h, image_t * gl)
{
	int id;
	unsigned defBits = 0;

	if (!gl) {
		Com_Printf("NULL pic in Draw_LoadingScreen2\n");
		return;
	}

		GL_BindProgram(loadingProgram, defBits);
		id = loadingProgram->id[defBits];
		qglUniform1i(qglGetUniformLocation(id, "u_map"), 0);
		qglUniform1f(qglGetUniformLocation(id, "u_colorScale"), loadScreenColorFade);
		
		qglEnableVertexAttribArray(ATRB_POSITION);
		qglEnableVertexAttribArray(ATRB_TEX0);
		qglVertexAttribPointer(ATRB_POSITION, 3, GL_FLOAT, false, 0, vertCoord);
		qglVertexAttribPointer(ATRB_TEX0, 2, GL_FLOAT, false, 0, texCoord);

		GL_MBind(GL_TEXTURE0_ARB, gl->texnum);

		VA_SetElem2(texCoord[0], gl->sl, gl->tl);
		VA_SetElem2(texCoord[1], gl->sh, gl->tl);
		VA_SetElem2(texCoord[2], gl->sh, gl->th);
		VA_SetElem2(texCoord[3], gl->sl, gl->th);

		VA_SetElem2(vertCoord[0], x, y);
		VA_SetElem2(vertCoord[1], x + w, y);
		VA_SetElem2(vertCoord[2], x + w, y + h);
		VA_SetElem2(vertCoord[3], x, y + h);

		qglDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		GL_BindNullProgram();
		qglDisableVertexAttribArray(ATRB_POSITION);
		qglDisableVertexAttribArray(ATRB_TEX0);
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
	int w, h, id;
	unsigned defBits = 0;

	if (!gl) {
		Com_Printf("NULL pic in Draw_Pic\n");
		return;
	}

	w = gl->width;
	h = gl->height;
	
	if(gl->has_alpha)
		GL_Blend(true, 0, 0);

	qglEnableVertexAttribArray(ATRB_POSITION);
	qglEnableVertexAttribArray(ATRB_TEX0);
	qglEnableVertexAttribArray(ATRB_COLOR);
	
    qglVertexAttribPointer(ATRB_POSITION, 3, GL_FLOAT, false, 0, vertCoord);
	qglVertexAttribPointer(ATRB_TEX0, 2, GL_FLOAT, false, 0, texCoord);
	qglVertexAttribPointer(ATRB_COLOR, 4, GL_FLOAT, false, 0, colorCoord);
	
	defBits = worldDefs.AttribColorBits;
	GL_BindProgram(genericProgram, defBits);
	id = genericProgram->id[defBits];
	qglUniform1i(qglGetUniformLocation(id, "u_map"), 0);
	qglUniform1f(qglGetUniformLocation(id, "u_colorScale"), r_worldColorScale->value);

	if (scrap_dirty)
		Scrap_Upload();

		GL_MBind(GL_TEXTURE0_ARB, gl->texnum);
				
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

		qglDrawArrays (GL_TRIANGLE_FAN, 0, 4);

	
	GL_Blend(false, 0, 0);
	GL_BindNullProgram();
	qglDisableVertexAttribArray(ATRB_POSITION);
	qglDisableVertexAttribArray(ATRB_TEX0);
	qglDisableVertexAttribArray(ATRB_COLOR);
}

void Draw_ScaledPic(int x, int y, float sX, float sY, image_t * gl)
{
	int w, h, id;
	unsigned	defBits = 0;

	if (!gl) {
		Com_Printf("NULL pic in Draw_Pic\n");
		return;
	}
	
	w = gl->width * sX *gl->picScale_w;
	h = gl->height * sY *gl->picScale_h;

	if(gl->has_alpha)
		GL_Blend(true, 0, 0);
	
	
	if (strstr(gl->name, "chxp")){ // crosshair hack
		GL_Blend(true, GL_ONE, GL_ONE);
		w = gl->width * sX;
		h = gl->height * sY;
	}

	qglEnableVertexAttribArray(ATRB_POSITION);
	qglEnableVertexAttribArray(ATRB_TEX0);
	qglEnableVertexAttribArray(ATRB_COLOR);
	
    qglVertexAttribPointer(ATRB_POSITION, 3, GL_FLOAT, false, 0, vertCoord);
	qglVertexAttribPointer(ATRB_TEX0, 2, GL_FLOAT, false, 0, texCoord);
	qglVertexAttribPointer(ATRB_COLOR, 4, GL_FLOAT, false, 0, colorCoord);
	
	defBits = worldDefs.AttribColorBits;
	GL_BindProgram(genericProgram, defBits);
	id = genericProgram->id[defBits];
	qglUniform1i(qglGetUniformLocation(id, "u_map"), 0);
	qglUniform1f(qglGetUniformLocation(id, "u_colorScale"), r_worldColorScale->value);

	if (scrap_dirty)
		Scrap_Upload();

		GL_MBind(GL_TEXTURE0_ARB, gl->texnum);
				
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

		qglDrawArrays (GL_TRIANGLE_FAN, 0, 4);

	
	GL_Blend(false, 0, 0);
	GL_BindNullProgram();
	qglDisableVertexAttribArray(ATRB_POSITION);
	qglDisableVertexAttribArray(ATRB_TEX0);
	qglDisableVertexAttribArray(ATRB_COLOR);
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


/*
=============
Draw_TileClear

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
void Draw_TileClear2(int x, int y, int w, int h, image_t * image)
{
	int			id;
	unsigned	defBits = 0;

	if (!image) {
		Com_Printf("NULL pic in Draw_TileClear\n");
		return;
	}

	qglEnableVertexAttribArray(ATRB_POSITION);
	qglEnableVertexAttribArray(ATRB_TEX0);
	qglEnableVertexAttribArray(ATRB_COLOR);

	qglVertexAttribPointer(ATRB_POSITION, 3, GL_FLOAT, false, 0, vertCoord);
	qglVertexAttribPointer(ATRB_TEX0, 2, GL_FLOAT, false, 0, texCoord);
	qglVertexAttribPointer(ATRB_COLOR, 4, GL_FLOAT, false, 0, colorCoord);

	defBits = worldDefs.AttribColorBits;
	GL_BindProgram(genericProgram, defBits);
	id = genericProgram->id[defBits];
	qglUniform1i(qglGetUniformLocation(id, "u_map"), 0);
	qglUniform1f(qglGetUniformLocation(id, "u_colorScale"), r_worldColorScale->value);

	GL_Bind(image->texnum);

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

	qglDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	GL_BindNullProgram();
	qglDisableVertexAttribArray(ATRB_POSITION);
	qglDisableVertexAttribArray(ATRB_TEX0);
	qglDisableVertexAttribArray(ATRB_COLOR);


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
	unsigned	defBits;
	int			id;

	qglEnableVertexAttribArray(ATRB_POSITION);
	qglVertexAttribPointer(ATRB_POSITION, 3, GL_FLOAT, false, 0, vertCoord);

	defBits = 0;
	GL_BindProgram(genericProgram, defBits);
	id = genericProgram->id[defBits];
	qglUniform4f(qglGetUniformLocation(id, "u_color"), r, g, b, a);
	qglUniform1f(qglGetUniformLocation(id, "u_colorScale"), r_worldColorScale->value);

	VA_SetElem2(vertCoord[0], x, y);
	VA_SetElem2(vertCoord[1], x + w, y);
	VA_SetElem2(vertCoord[2], x + w, y + h);
	VA_SetElem2(vertCoord[3], x, y + h);

	qglDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	
	GL_BindNullProgram();
	qglDisableVertexAttribArray(ATRB_POSITION);
}

//=============================================================================

/*
================
Draw_FadeScreen

================
*/
void Draw_FadeScreen(void)
{
	unsigned	defBits;
	int			id;

	GL_Blend(true, 0, 0);

	qglEnableVertexAttribArray(ATRB_POSITION);
	qglVertexAttribPointer(ATRB_POSITION, 3, GL_FLOAT, false, 0, vertCoord);

	defBits = 0;
	GL_BindProgram(genericProgram, defBits);
	id = genericProgram->id[defBits];
	qglUniform4f(qglGetUniformLocation(id, "u_color"), 0, 0, 0, 0.5);
	qglUniform1f(qglGetUniformLocation(id, "u_colorScale"), 1.0);

	VA_SetElem2(vertCoord[0], 0, 0);
	VA_SetElem2(vertCoord[1], vid.width, 0);
	VA_SetElem2(vertCoord[2], vid.width, vid.height);
	VA_SetElem2(vertCoord[3], 0, vid.height);

	qglDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	GL_BindNullProgram();
	qglDisableVertexAttribArray(ATRB_POSITION);
	GL_Blend(false, 0, 0);

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
	int			id;
	unsigned	defBits = 0;

	qglClear(GL_COLOR_BUFFER_BIT);
	qglClearColor(0.0, 0.0, 0.0, 1);

	// setup program
	GL_BindProgram(cinProgram, defBits);
	id = cinProgram->id[defBits];

	GL_MBind(GL_TEXTURE0, 0);
	qglUniform1i(qglGetUniformLocation(id, "u_cinMap"), 0);
	qglUniform2f(qglGetUniformLocation(id, "u_cinSize"), w, h);
	qglUniform2f(qglGetUniformLocation(id, "u_cinIntSize"), cols, rows);

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

	qglEnableVertexAttribArray(ATRB_POSITION);
	qglEnableVertexAttribArray(ATRB_TEX0);

	qglVertexAttribPointer(ATRB_POSITION, 3, GL_FLOAT, false, 0, vertCoord);
	qglVertexAttribPointer(ATRB_TEX0, 2, GL_FLOAT, false, 0, texCoord);

	VA_SetElem2(texCoord[0], 0, 0);
	VA_SetElem2(texCoord[1], 1, 0);
	VA_SetElem2(texCoord[2], 1, 1);
	VA_SetElem2(texCoord[3], 0, 1);

	VA_SetElem2(vertCoord[0], x0, y0);
	VA_SetElem2(vertCoord[1], x1, y0);
	VA_SetElem2(vertCoord[2], x1, y1);
	VA_SetElem2(vertCoord[3], x0, y1);

	qglDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	
	GL_BindNullProgram();
	qglDisableVertexAttribArray(ATRB_POSITION);
	qglDisableVertexAttribArray(ATRB_TEX0);
}
