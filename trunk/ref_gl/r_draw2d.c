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
vec3_t vertCoord[MAX_VERTEX_ARRAY];

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
	GL_Overbrights(true);
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
	GL_Overbrights(false);
}


void Draw_CharScaledShadow(int x, int y, float scale_x, float scale_y, unsigned char num)
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
	GL_Overbrights(false);
}


void Draw_StringScaled(int x, int y, float scale_x, float scale_y, const char *str)
{
	int px, py, row, col, num, va = 0;
	float frow, fcol, size;
	unsigned char *s = (unsigned char *) str;

	if (gl_state.currenttextures[gl_state.currenttmu] !=
		draw_chars->texnum) {
		GL_Bind(draw_chars->texnum);
	}

	GL_Overbrights(true);
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
	GL_Overbrights(false);
}


void Draw_StringScaledShadow(int x, int y, float scale_x, float scale_y, const char *str)
{
	int px, py, row, col, num, va = 0;
	float frow, fcol, size;
	unsigned char *s = (unsigned char *) str;

	if (gl_state.currenttextures[gl_state.currenttmu] !=
		draw_chars->texnum) {
		GL_Bind(draw_chars->texnum);
	}

	GL_Overbrights(false);
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
	GL_Overbrights(false);
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

void Draw_StretchPic2(int x, int y, int w, int h, image_t * gl,
					  qboolean cons)
{
		
	if (!gl) {
		Com_Printf("NULL pic in Draw_StretchPic\n");
		return;
	}
	GL_Blend(true, 0, 0);

	if (cons) {

		qglColor4f(1, 1, 1, 0.55);
		GL_Overbrights(false);
		qglDepthMask(GL_FALSE);	
	} else {
		qglColor4f(1, 1, 1, 1);
		GL_Overbrights(true);
		qglDepthMask(GL_TRUE);	

	}
	if (scrap_dirty)
		Scrap_Upload();

	
		GL_Bind(gl->texnum);
		qglBegin(GL_QUADS);
		qglTexCoord2f(gl->sl, gl->tl);
		qglVertex2f(x, y);
		qglTexCoord2f(gl->sh, gl->tl);
		qglVertex2f(x + w, y);
		qglTexCoord2f(gl->sh, gl->th);
		qglVertex2f(x + w, y + h);
		qglTexCoord2f(gl->sl, gl->th);
		qglVertex2f(x, y + h);
		qglEnd();

	
	GL_Blend(false, 0, 0);
	GL_Overbrights(false);
	qglDepthMask(GL_TRUE);	
}


void Draw_StretchPic(int x, int y, int w, int h, char *pic)
{
	qboolean cons = 0;
	image_t *gl;
	GL_Overbrights(true);
	gl = Draw_FindPic(pic);

	if (!gl) {
		Com_Printf("Can't find pic: %s\n", pic);
		return;
	}


	Draw_StretchPic2(x, y, w, h, gl, cons);
	GL_Overbrights(false);
}


/*
=============
Draw_Pic
=============
*/

void Draw_Pic2(int x, int y, image_t * gl)
{
	int w, h;
	
	w = gl->width;
	h = gl->height;
	GL_Overbrights(true);
	
	if (!gl) {
		Com_Printf("NULL pic in Draw_Pic\n");
		return;
	}
	
	if(gl->has_alpha)
		GL_Blend(true, 0, 0);

	qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	qglTexCoordPointer (2, GL_FLOAT, sizeof(vec2_t), texCoord[0]);
     
	qglEnableClientState (GL_VERTEX_ARRAY);
	qglVertexPointer (3, GL_FLOAT, sizeof(vec3_t), vertCoord[0]);
     

	if (scrap_dirty)
		Scrap_Upload();

		GL_Bind(gl->texnum);
				
		VA_SetElem2(texCoord[0],gl->sl, gl->tl);
		VA_SetElem2(texCoord[1],gl->sh, gl->tl);
		VA_SetElem2(texCoord[2],gl->sh, gl->th);
		VA_SetElem2(texCoord[3],gl->sl, gl->th);
		
		VA_SetElem2(vertCoord[0],x, y);
		VA_SetElem2(vertCoord[1],x + gl->width, y);
		VA_SetElem2(vertCoord[2],x + gl->width, y + gl->height);
		VA_SetElem2(vertCoord[3],x, y + gl->height);
		

		qglDrawArrays (GL_QUADS, 0, 4);

	
	GL_Blend(false, 0, 0);
	GL_Overbrights(false);
	qglDisableClientState(GL_VERTEX_ARRAY);
	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void Draw_ScaledPic(int x, int y, float scale_x, float scale_y, image_t * gl)
{
	int w, h;
	
	w = gl->width*gl->picScale_w*scale_x;
	h = gl->height*gl->picScale_h*scale_y;
	GL_Overbrights(true);
	
	if (!gl) {
		Com_Printf("NULL pic in Draw_Pic\n");
		return;
	}
	
	if(gl->has_alpha)
		GL_Blend(true, 0, 0);
	
	if (strstr(gl->name, "chxp"))
		GL_Blend(true, GL_ONE, GL_ONE);

	qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	qglTexCoordPointer (2, GL_FLOAT, sizeof(vec2_t), texCoord[0]);
     
	qglEnableClientState (GL_VERTEX_ARRAY);
	qglVertexPointer (3, GL_FLOAT, sizeof(vec3_t), vertCoord[0]);
     

	if (scrap_dirty)
		Scrap_Upload();

		GL_Bind(gl->texnum);
				
		VA_SetElem2(texCoord[0],gl->sl, gl->tl);
		VA_SetElem2(texCoord[1],gl->sh, gl->tl);
		VA_SetElem2(texCoord[2],gl->sh, gl->th);
		VA_SetElem2(texCoord[3],gl->sl, gl->th);
		
		VA_SetElem2(vertCoord[0],x, y);
		VA_SetElem2(vertCoord[1],x + w, y);
		VA_SetElem2(vertCoord[2],x + w, y + h);
		VA_SetElem2(vertCoord[3],x, y + h);
		
		qglDrawArrays (GL_QUADS, 0, 4);
	
	GL_Blend(false, 0, 0);	
	GL_Overbrights(false);
	qglDisableClientState(GL_VERTEX_ARRAY);
	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void Draw_Pic(int x, int y, char *pic)
{
	image_t *gl;
	GL_Overbrights(true);
	gl = Draw_FindPic(pic);
	if (!gl) {
		Com_Printf("Can't find pic: %s\n", pic);
		return;
	}
	Draw_Pic2(x, y, gl);
	GL_Overbrights(false);

}

void Draw_PicScaled(int x, int y, float scale_x, float scale_y, char *pic)
{
	image_t *gl;
	GL_Overbrights(true);
	gl = Draw_FindPic(pic);
	if (!gl) {
		Com_Printf("Can't find pic: %s\n", pic);
		return;
	}
	Draw_ScaledPic(x, y, scale_x, scale_y, gl);
	GL_Overbrights(false);

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


	GL_Bind(image->texnum);
	qglBegin(GL_QUADS);
	qglTexCoord2f(x / 64.0, y / 64.0);
	qglVertex2f(x, y);
	qglTexCoord2f((x + w) / 64.0, y / 64.0);
	qglVertex2f(x + w, y);
	qglTexCoord2f((x + w) / 64.0, (y + h) / 64.0);
	qglVertex2f(x + w, y + h);
	qglTexCoord2f(x / 64.0, (y + h) / 64.0);
	qglVertex2f(x, y + h);
	qglEnd();




}

void Draw_TileClear(int x, int y, int w, int h, char *pic)
{
	image_t *image;
//GL_Overbrights (true);
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
void Draw_Fill(int x, int y, int w, int h, int c)
{
	
	qglDisable(GL_TEXTURE_2D);
	
	if (c > 255)
		goto nocolor;

	qglColor3f(d_8to24tablef[c][0], d_8to24tablef[c][1],
			   d_8to24tablef[c][2]);
	
nocolor:

	qglBegin(GL_QUADS);

	qglVertex2f(x, y);
	qglVertex2f(x + w, y);
	qglVertex2f(x + w, y + h);
	qglVertex2f(x, y + h);

	qglEnd();

	qglColor3f(1, 1, 1);
	qglEnable(GL_TEXTURE_2D);
}

//=============================================================================

/*
================
Draw_FadeScreen

================
*/
void Draw_FadeScreen(void)
{
	GL_Blend(true, 0, 0);
	qglDisable(GL_TEXTURE_2D);
	qglColor4f(0, 0, 0, 0.5);

	qglBegin(GL_QUADS);

	qglVertex2f(0, 0);
	qglVertex2f(vid.width, 0);
	qglVertex2f(vid.width, vid.height);
	qglVertex2f(0, vid.height);

	qglEnd();


	qglColor4f(1, 1, 1, 1);
	qglEnable(GL_TEXTURE_2D);
	GL_Blend(false, 0, 0);

}


/*
=============
Draw_StretchRaw
=============
*/
extern unsigned r_rawpalette[256];

void Draw_StretchRaw (int x, int y, int w, int h, int cols, int rows, byte *data)
{
	int			i, j, trows;
	byte		*source;
	int			frac, fracstep;
	float		hscale;
	int			row;
	float		t;
	unsigned image32[256*256] = { 0 };
	unsigned *dest = image32;

	// clear wide screen borders
	qglClear(GL_COLOR_BUFFER_BIT);
	qglClearColor(0.0, 0.0, 0.0, 1);
	GL_MBind(GL_TEXTURE0_ARB, 0);
	GL_Overbrights (true);
	
	// Berserker: fixed bug - crap cin edges
	hscale = rows * 0.00390625;
	trows = 256;
	t = rows*hscale * 0.00390625;
	fracstep = cols * 0x10000 * 0.00390625;

	for (i=0 ; i<trows ; i++, dest+=256)
	{
		row = (int)(i*hscale);
		if (row > rows)
			break;
		source = data + cols*row;

		frac = fracstep >> 1;
		for (j=0 ; j<256 ; j+=4)
		{
			dest[j] = r_rawpalette[source[frac>>16]];
			frac += fracstep;
			dest[j+1] = r_rawpalette[source[frac>>16]];
			frac += fracstep;
			dest[j+2] = r_rawpalette[source[frac>>16]];
			frac += fracstep;
			dest[j+3] = r_rawpalette[source[frac>>16]];
			frac += fracstep;
		}
	}

	qglTexImage2D (GL_TEXTURE_2D, 0, GL_RGB8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, image32);
	
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	qglBegin (GL_QUADS);
	qglTexCoord2f (0, 0);
	qglVertex2i (x, y);
	qglTexCoord2f (1, 0);
	qglVertex2i (x+w, y);
	qglTexCoord2f (1, t);
	qglVertex2i (x+w, y+h);
	qglTexCoord2f (0, t);
	qglVertex2i (x, y+h);
	qglEnd ();

	GL_Overbrights (false);
}
