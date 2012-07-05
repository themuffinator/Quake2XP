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

// original Q2 palette
static const byte r_originalPalette[] = {
  0,   0,   0,  15,  15,  15,  31,  31,  31,  47,  47,  47, 
 63,  63,  63,  75,  75,  75,  91,  91,  91, 107, 107, 107, 
123, 123, 123, 139, 139, 139, 155, 155, 155, 171, 171, 171, 
187, 187, 187, 203, 203, 203, 219, 219, 219, 235, 235, 235, 
 99,  75,  35,  91,  67,  31,  83,  63,  31,  79,  59,  27, 
 71,  55,  27,  63,  47,  23,  59,  43,  23,  51,  39,  19, 
 47,  35,  19,  43,  31,  19,  39,  27,  15,  35,  23,  15, 
 27,  19,  11,  23,  15,  11,  19,  15,   7,  15,  11,   7, 
 95,  95, 111,  91,  91, 103,  91,  83,  95,  87,  79,  91, 
 83,  75,  83,  79,  71,  75,  71,  63,  67,  63,  59,  59, 
 59,  55,  55,  51,  47,  47,  47,  43,  43,  39,  39,  39, 
 35,  35,  35,  27,  27,  27,  23,  23,  23,  19,  19,  19, 
143, 119,  83, 123,  99,  67, 115,  91,  59, 103,  79,  47, 
207, 151,  75, 167, 123,  59, 139, 103,  47, 111,  83,  39, 
235, 159,  39, 203, 139,  35, 175, 119,  31, 147,  99,  27, 
119,  79,  23,  91,  59,  15,  63,  39,  11,  35,  23,   7, 
167,  59,  43, 159,  47,  35, 151,  43,  27, 139,  39,  19, 
127,  31,  15, 115,  23,  11, 103,  23,   7,  87,  19,   0, 
 75,  15,   0,  67,  15,   0,  59,  15,   0,  51,  11,   0, 
 43,  11,   0,  35,  11,   0,  27,   7,   0,  19,   7,   0, 
123,  95,  75, 115,  87,  67, 107,  83,  63, 103,  79,  59, 
 95,  71,  55,  87,  67,  51,  83,  63,  47,  75,  55,  43, 
 67,  51,  39,  63,  47,  35,  55,  39,  27,  47,  35,  23, 
 39,  27,  19,  31,  23,  15,  23,  15,  11,  15,  11,   7, 
111,  59,  23,  95,  55,  23,  83,  47,  23,  67,  43,  23, 
 55,  35,  19,  39,  27,  15,  27,  19,  11,  15,  11,   7, 
179,  91,  79, 191, 123, 111, 203, 155, 147, 215, 187, 183, 
203, 215, 223, 179, 199, 211, 159, 183, 195, 135, 167, 183, 
115, 151, 167,  91, 135, 155,  71, 119, 139,  47, 103, 127, 
 23,  83, 111,  19,  75, 103,  15,  67,  91,  11,  63,  83, 
  7,  55,  75,   7,  47,  63,   7,  39,  51,   0,  31,  43, 
  0,  23,  31,   0,  15,  19,   0,   7,  11,   0,   0,   0, 
139,  87,  87, 131,  79,  79, 123,  71,  71, 115,  67,  67, 
107,  59,  59,  99,  51,  51,  91,  47,  47,  87,  43,  43, 
 75,  35,  35,  63,  31,  31,  51,  27,  27,  43,  19,  19, 
 31,  15,  15,  19,  11,  11,  11,   7,   7,   0,   0,   0, 
151, 159, 123, 143, 151, 115, 135, 139, 107, 127, 131,  99, 
119, 123,  95, 115, 115,  87, 107, 107,  79,  99,  99,  71, 
 91,  91,  67,  79,  79,  59,  67,  67,  51,  55,  55,  43, 
 47,  47,  35,  35,  35,  27,  23,  23,  19,  15,  15,  11, 
159,  75,  63, 147,  67,  55, 139,  59,  47, 127,  55,  39, 
119,  47,  35, 107,  43,  27,  99,  35,  23,  87,  31,  19, 
 79,  27,  15,  67,  23,  11,  55,  19,  11,  43,  15,   7, 
 31,  11,   7,  23,   7,   0,  11,   0,   0,   0,   0,   0, 
119, 123, 207, 111, 115, 195, 103, 107, 183,  99,  99, 167, 
 91,  91, 155,  83,  87, 143,  75,  79, 127,  71,  71, 115, 
 63,  63, 103,  55,  55,  87,  47,  47,  75,  39,  39,  63, 
 35,  31,  47,  27,  23,  35,  19,  15,  23,  11,   7,   7, 
155, 171, 123, 143, 159, 111, 135, 151,  99, 123, 139,  87, 
115, 131,  75, 103, 119,  67,  95, 111,  59,  87, 103,  51, 
 75,  91,  39,  63,  79,  27,  55,  67,  19,  47,  59,  11, 
 35,  47,   7,  27,  35,   0,  19,  23,   0,  11,  15,   0, 
  0, 255,   0,  35, 231,  15,  63, 211,  27,  83, 187,  39, 
 95, 167,  47,  95, 143,  51,  95, 123,  51, 255, 255, 255, 
255, 255, 211, 255, 255, 167, 255, 255, 127, 255, 255,  83, 
255, 255,  39, 255, 235,  31, 255, 215,  23, 255, 191,  15, 
255, 171,   7, 255, 147,   0, 239, 127,   0, 227, 107,   0, 
211,  87,   0, 199,  71,   0, 183,  59,   0, 171,  43,   0, 
155,  31,   0, 143,  23,   0, 127,  15,   0, 115,   7,   0, 
 95,   0,   0,  71,   0,   0,  47,   0,   0,  27,   0,   0, 
239,   0,   0,  55,  55, 255, 255,   0,   0,   0,   0, 255, 
 43,  43,  35,  27,  27,  23,  19,  19,  15, 235, 151, 127, 
195, 115,  83, 159,  87,  51, 123,  63,  27, 235, 211, 199, 
199, 171, 155, 167, 139, 119, 135, 107,  87, 159,  91,  83
};


image_t gltextures[MAX_GLTEXTURES];
image_t *r_lblendimage;
int numgltextures;
int base_textureid;				// gltextures[i] = base_textureid+i

static byte intensitytable[256];
static unsigned char gammatable[256];



unsigned d_8to24table[256];
float d_8to24tablef[256][3];

qboolean GL_Upload8(byte * data, int width, int height, qboolean mipmap,
					qboolean is_sky);
qboolean GL_Upload32(unsigned *data, int width, int height,
					 qboolean mipmap);


int gl_solid_format =  GL_RGBA;
int gl_alpha_format = GL_RGBA;

int gl_tex_solid_format = GL_RGBA;
int gl_tex_alpha_format = GL_RGBA;

int gl_filter_min = GL_LINEAR_MIPMAP_LINEAR;	// default to trilinear
												// filtering - MrG
int gl_filter_max = GL_LINEAR;


void GL_SelectTexture(GLenum texture)
{
	int tmu;

	if (!qglActiveTextureARB)
		return;

	tmu = texture - GL_TEXTURE0_ARB;

	if (tmu == gl_state.currenttmu)
		return;

	gl_state.currenttmu = tmu;

	if (qglActiveTextureARB) {
		qglActiveTextureARB(texture);
		qglClientActiveTextureARB(texture);
	}
}

void GL_TexEnv(GLenum mode)
{
	static int lastmodes[2] = { -1, -1 };

	if (mode != lastmodes[gl_state.currenttmu]) {
		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode);
		lastmodes[gl_state.currenttmu] = mode;
	}
}

void GL_Bind(int texnum)
{
	extern image_t *draw_chars;

	if (r_noBind->value && draw_chars)	// performance evaluation option
		texnum = draw_chars->texnum;
	if (gl_state.currenttextures[gl_state.currenttmu] == texnum)
		return;
	gl_state.currenttextures[gl_state.currenttmu] = texnum;
	qglBindTexture(GL_TEXTURE_2D, texnum);
}

void GL_BindRect(int texnum)
{
	extern image_t *draw_chars;

	if (r_noBind->value && draw_chars)	// performance evaluation option
		texnum = draw_chars->texnum;
	if (gl_state.currenttextures[gl_state.currenttmu] == texnum)
		return;
	gl_state.currenttextures[gl_state.currenttmu] = texnum;
	qglBindTexture(GL_TEXTURE_RECTANGLE_ARB, texnum);
}

void GL_MBind(GLenum target, int texnum)
{
	int targ = target - GL_TEXTURE0_ARB;
	GL_SelectTexture(target);

	if (gl_state.currenttextures[targ] == texnum)
		return;
	GL_Bind(texnum);
}

void GL_MBindRect(GLenum target, int texnum)
{
	int targ = target - GL_TEXTURE0_ARB;
	GL_SelectTexture(target);

	if (gl_state.currenttextures[targ] == texnum)
		return;

	GL_BindRect(texnum);
}

void GL_Blend(qboolean on, int src, int dst)
{
	if (on) {
		qglEnable(GL_BLEND);
		if (dst == 0 || src == 0)
			qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		else
			qglBlendFunc(src, dst);
	}

	else
	{
		qglDisable(GL_BLEND);
		qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		dst = 0;
		src = 0;
	}

}

void R_CaptureColorBuffer()
{
		
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
		
	qglEnable(GL_TEXTURE_RECTANGLE_ARB);
	GL_MBindRect(GL_TEXTURE0_ARB, ScreenMap->texnum);
	qglCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
	qglDisable(GL_TEXTURE_RECTANGLE_ARB);
}

void R_CaptureDepthBuffer()
{
		
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
		
	qglEnable(GL_TEXTURE_RECTANGLE_ARB);
	GL_MBindRect(GL_TEXTURE0_ARB, depthMap->texnum);
	qglCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
	qglDisable(GL_TEXTURE_RECTANGLE_ARB);

}

void GL_PicsColorScaleARB(qboolean enable)
{
	
	if (enable) {				// turn on
		GL_TexEnv(GL_COMBINE_ARB);

		qglTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
		qglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
		qglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PREVIOUS_ARB);
		qglTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, (int) r_picsColorScale->value);

		qglTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_MODULATE);
		qglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
		qglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, GL_PREVIOUS_ARB);
		qglTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);
	} else {					// turn off
		GL_TexEnv(GL_MODULATE);

		qglTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
		qglTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);
	}
}

/*
===============
GL_TextureMode
===============
*/

void GL_TextureMode(char *string)
{
	int i;
	image_t *glt;

	strlwr(string);

	if (Q_stricmp(r_textureMode->string,	"GL_NEAREST") != 0 &&
	Q_stricmp(r_textureMode->string,		"GL_LINEAR") != 0 &&
	Q_stricmp(r_textureMode->string,		"GL_NEAREST_MIPMAP_NEAREST") != 0 &&
	Q_stricmp(r_textureMode->string,		"GL_LINEAR_MIPMAP_NEAREST") != 0 &&
	Q_stricmp(r_textureMode->string,		"GL_NEAREST_MIPMAP_LINEAR") != 0 &&
	Q_stricmp(r_textureMode->string,		"GL_LINEAR_MIPMAP_LINEAR") != 0)
	Cvar_Set("r_textureMode",			"GL_NEAREST");
	
	if (!Q_stricmp(r_textureMode->string, "GL_NEAREST")){
		gl_filter_min = GL_NEAREST;
		gl_filter_max = GL_NEAREST;
	}
	if (!Q_stricmp(r_textureMode->string, "GL_LINEAR")){
		gl_filter_min = GL_LINEAR;
		gl_filter_max = GL_LINEAR;
	}
	if (!Q_stricmp(r_textureMode->string, "GL_NEAREST_MIPMAP_NEAREST")){
		gl_filter_min = GL_NEAREST_MIPMAP_NEAREST;
		gl_filter_max = GL_NEAREST;
	}
	if (!Q_stricmp(r_textureMode->string, "GL_LINEAR_MIPMAP_NEAREST")){
		gl_filter_min = GL_LINEAR_MIPMAP_NEAREST;
		gl_filter_max = GL_LINEAR;
	}
	if (!Q_stricmp(r_textureMode->string, "GL_NEAREST_MIPMAP_LINEAR")){
		gl_filter_min = GL_NEAREST_MIPMAP_LINEAR;
		gl_filter_max = GL_NEAREST;
	}
	if (!Q_stricmp(r_textureMode->string, "GL_LINEAR_MIPMAP_LINEAR")){
		gl_filter_min = GL_LINEAR_MIPMAP_LINEAR;
		gl_filter_max = GL_LINEAR;
	}
	
	// change all the existing mipmap texture objects
	for (i = 0, glt = gltextures; i < numgltextures; i++, glt++) {
		if (glt->type != it_pic && glt->type != it_sky) {
			GL_Bind(glt->texnum);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
			// realtime update anisotropy level
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, r_anisotropic->value);

		}
	}
}



/*
===============
GL_ImageList_f
===============
*/

void GL_ImageList_f(void)
{
	int i, totalTexturesSize;
	image_t *image;
	int texels;

	const char *palstrings[2] = {
		"RGB",
		"PAL"
	};
	totalTexturesSize = 0;
	Com_Printf("------------------\n");
	texels = 0;

	for (i = 0, image = gltextures+100; i < numgltextures; i++, image++) {
		
		if (image->texnum <= 0)
			continue;
		
		texels += image->upload_width * image->upload_height;

		
		totalTexturesSize += image->upload_width * image->upload_height*4;

		switch (image->type) {
		case it_skin:
			Com_Printf("Skin");
			break;
		case it_sprite:
			Com_Printf("Sprite");
			break;
		case it_wall:
			Com_Printf("Wall");
			break;
		case it_pic:
			Com_Printf("Pics");
			break;
		default:
			Com_Printf(" ");
			break;
		}

		Com_Printf(" %3i %3i %s: %s\n",
				   image->upload_width, image->upload_height,
				   palstrings[image->paletted], image->name);
	}
	Com_Printf("Total texel count (not counting mipmaps): %i\n",
			   texels);

	Com_Printf("%3.2f MB total image memory\n",	(float)totalTexturesSize / (1024 * 1024));

}


/*
=============================================================================

  scrap allocation

  Allocate all the little status bar obejcts into a single texture
  to crutch up inefficient hardware / drivers

=============================================================================
*/

#define	MAX_SCRAPS		1
#define	BLOCK_SIZE		256


int scrap_allocated[MAX_SCRAPS][BLOCK_SIZE];
byte scrap_texels[MAX_SCRAPS][BLOCK_SIZE * BLOCK_SIZE];
qboolean scrap_dirty;

// returns a texture number and the position inside it
int Scrap_AllocBlock(int w, int h, int *x, int *y)
{
	int i, j;
	int best, best2;
	int texnum;

	for (texnum = 0; texnum < MAX_SCRAPS; texnum++) {
		best = BLOCK_SIZE;

		for (i = 0; i < BLOCK_SIZE - w; i++) {
			best2 = 0;

			for (j = 0; j < w; j++) {
				if (scrap_allocated[texnum][i + j] >= best)
					break;
				if (scrap_allocated[texnum][i + j] > best2)
					best2 = scrap_allocated[texnum][i + j];
			}
			if (j == w) {		// this is a valid spot
				*x = i;
				*y = best = best2;
			}
		}

		if (best + h > BLOCK_SIZE)
			continue;

		for (i = 0; i < w; i++)
			scrap_allocated[texnum][*x + i] = best + h;

		return texnum;
	}

	return -1;
//  Sys_Error ("Scrap_AllocBlock: full");
}

int scrap_uploads;

void Scrap_Upload(void)
{
	scrap_uploads++;
	GL_Bind(TEXNUM_SCRAPS);
	GL_Upload8(scrap_texels[0], BLOCK_SIZE, BLOCK_SIZE, false, false);
	scrap_dirty = false;
}

/*
=================================================================

PCX LOADING

=================================================================
*/


/*
==============
LoadPCX
==============
*/
void LoadPCX(char *filename, byte ** pic, byte ** palette, int *width,
			 int *height)
{
	byte *raw;
	pcx_t *pcx;
	int x, y;
	int len;
	int dataByte, runLength;
	byte *out, *pix;

	*pic = NULL;
	*palette = NULL;

	// 
	// load the file
	// 
	len = FS_LoadFile(filename, (void **) &raw);
	if (!raw) {
		Con_Printf(PRINT_DEVELOPER, "Bad pcx file %s\n", filename);
		return;
	}

	if (!len) {
		FS_FreeFile(raw);
		Con_Printf(PRINT_DEVELOPER, "Bad pcx file %s\n", filename);
		return;
	}
	// 
	// parse the PCX file
	// 
	pcx = (pcx_t *) raw;

	pcx->xmin = LittleShort(pcx->xmin);
	pcx->ymin = LittleShort(pcx->ymin);
	pcx->xmax = LittleShort(pcx->xmax);
	pcx->ymax = LittleShort(pcx->ymax);
	pcx->hres = LittleShort(pcx->hres);
	pcx->vres = LittleShort(pcx->vres);
	pcx->bytes_per_line = LittleShort(pcx->bytes_per_line);
	pcx->palette_type = LittleShort(pcx->palette_type);

	raw = &pcx->data;

	if (pcx->manufacturer != 0x0a
		|| pcx->version != 5
		|| pcx->encoding != 1
		|| pcx->bits_per_pixel != 8
		|| pcx->xmax >= 640 || pcx->ymax >= 480) {
		Com_Printf("Bad pcx file %s\n", filename);
		return;
	}

	out = malloc((pcx->ymax + 1) * (pcx->xmax + 1));

	*pic = out;

	pix = out;

	if (palette) {
		*palette = malloc(768);
		Q_memcpy(*palette, (byte *) pcx + len - 768, 768);
	}

	if (width)
		*width = pcx->xmax + 1;
	if (height)
		*height = pcx->ymax + 1;

	for (y = 0; y <= pcx->ymax; y++, pix += pcx->xmax + 1) {
		for (x = 0; x <= pcx->xmax;) {
			dataByte = *raw++;

			if ((dataByte & 0xC0) == 0xC0) {
				runLength = dataByte & 0x3F;
				dataByte = *raw++;
			} else
				runLength = 1;

			while (runLength-- > 0)
				pix[x++] = dataByte;
		}

	}

	if (raw - (byte *) pcx > len) {
		Con_Printf(PRINT_DEVELOPER, "PCX file %s was malformed", filename);
		free(*pic);
		*pic = NULL;
	}

	FS_FreeFile(pcx);

}



/*
====================================================================

IMAGE FLOOD FILLING

====================================================================
*/


/*
=================
Mod_FloodFillSkin

Fill background pixels so mipmapping doesn't have haloes
=================
*/

typedef struct {
	short x, y;
} floodfill_t;

// must be a power of 2
#define FLOODFILL_FIFO_SIZE 0x1000
#define FLOODFILL_FIFO_MASK (FLOODFILL_FIFO_SIZE - 1)

#define FLOODFILL_STEP( off, dx, dy ) \
{ \
	if (pos[off] == fillcolor) \
	{ \
		pos[off] = 255; \
		fifo[inpt].x = x + (dx), fifo[inpt].y = y + (dy); \
		inpt = (inpt + 1) & FLOODFILL_FIFO_MASK; \
	} \
	else if (pos[off] != 255) fdc = pos[off]; \
}

void R_FloodFillSkin(byte * skin, int skinwidth, int skinheight)
{
	byte fillcolor;
	floodfill_t fifo[FLOODFILL_FIFO_SIZE];
	int inpt = 0, outpt = 0;
	int filledcolor = -1;
	int i;

	if (!skin) {
		Com_Printf("WARNING: R_FloodFillSkin: NULL data\n");
		return;
	}
	// assume this is the pixel to fill
	fillcolor = *skin;

	if (filledcolor == -1) {
		filledcolor = 0;
		// attempt to find opaque black
		for (i = 0; i < 256; ++i)
			if (d_8to24table[i] == (255 << 0))	// alpha 1.0
			{
				filledcolor = i;
				break;
			}
	}
	// can't fill to filled color or to transparent color (used as visited 
	// marker)
	if ((fillcolor == filledcolor) || (fillcolor == 255)) {
		// printf( "not filling skin from %d to %d\n", fillcolor,
		// filledcolor );
		return;
	}

	fifo[inpt].x = 0, fifo[inpt].y = 0;
	inpt = (inpt + 1) & FLOODFILL_FIFO_MASK;

	while (outpt != inpt) {
		int x = fifo[outpt].x, y = fifo[outpt].y;
		int fdc = filledcolor;
		byte *pos = &skin[x + skinwidth * y];

		outpt = (outpt + 1) & FLOODFILL_FIFO_MASK;

		if (x > 0)
			FLOODFILL_STEP(-1, -1, 0);
		if (x < skinwidth - 1)
			FLOODFILL_STEP(1, 1, 0);
		if (y > 0)
			FLOODFILL_STEP(-skinwidth, 0, -1);
		if (y < skinheight - 1)
			FLOODFILL_STEP(skinwidth, 0, 1);
		skin[x + skinwidth * y] = fdc;
	}
}


//=======================================================



/*
================
GL_ResampleTexture
================
*/
void GL_ResampleTexture(unsigned *in, int inwidth, int inheight,
						unsigned *out, int outwidth, int outheight)
{
	int i, j;
	unsigned *inrow, *inrow2;
	unsigned frac, fracstep;
	unsigned p1[4096], p2[4096];
	byte *pix1, *pix2, *pix3, *pix4;

	// divby0 error! - psychospaz
	if (outheight == 0 || outwidth == 0) {
		out = NULL;
		return;
	}

	fracstep = inwidth * 0x10000 / outwidth;

	frac = fracstep >> 2;
	for (i = 0; i < outwidth; i++) {
		p1[i] = 4 * (frac >> 16);
		frac += fracstep;
	}
	frac = 3 * (fracstep >> 2);
	for (i = 0; i < outwidth; i++) {
		p2[i] = 4 * (frac >> 16);
		frac += fracstep;
	}

	for (i = 0; i < outheight; i++, out += outwidth) {
		inrow = in + inwidth * (int) ((i + 0.25) * inheight / outheight);
		inrow2 = in + inwidth * (int) ((i + 0.75) * inheight / outheight);
		frac = fracstep >> 1;
		for (j = 0; j < outwidth; j++) {
			// XXX: I've tried with SSE and it doesn't improve considerably,
			// also keep in mind to allocate extra space for average overflow.
			pix1 = (byte *) inrow + p1[j];
			pix2 = (byte *) inrow + p2[j];
			pix3 = (byte *) inrow2 + p1[j];
			pix4 = (byte *) inrow2 + p2[j];
			((byte *) (out + j))[0] =
				(pix1[0] + pix2[0] + pix3[0] + pix4[0]) >> 2;
			((byte *) (out + j))[1] =
				(pix1[1] + pix2[1] + pix3[1] + pix4[1]) >> 2;
			((byte *) (out + j))[2] =
				(pix1[2] + pix2[2] + pix3[2] + pix4[2]) >> 2;
			((byte *) (out + j))[3] =
				(pix1[3] + pix2[3] + pix3[3] + pix4[3]) >> 2;
		}
	}
}

/*
===============
GL_Upload32

Returns has_alpha
===============
*/

int upload_width, upload_height;
qboolean uploaded_paletted;

int nearest_power_of_2(int size)
{
	int i = 2;

	while (1) {
		i <<= 1;
		if (size == i)
			return i;
		if (size > i && size < (i << 1)) {
			if (size >= ((i + (i << 1)) / 2))
				return i << 1;
			else
				return i;
		}
	};
}


image_t *GL_LoadPic(char *name, byte * pic, int width, int height,
					imagetype_t type, int bits);

// Fast reciprocal square root (Quake 3 game code)
__inline float RSqrt(float number)
{
	long i;
	float x2, y;
	const float threehalfs = 1.5f;

	x2 = number * 0.5f;
	y  = number;
	i  = * (long *) &y;						// evil floating point bit level hacking
	i  = 0x5f3759df - (i >> 1);             // what the fuck?
	y  = * (float *) &i;
	y  = y * (threehalfs - (x2 * y * y));   // 1st iteration

	return y;
}

void GL_sRgbTextureConversion (unsigned *in, int inwidth, int inheight)
{
		int		i, c;
		byte	*p;

		p = (byte *)in;

		c = inwidth*inheight;
		for (i=0 ; i<c ; i++, p+=4)
		{
			if(p[3] && (p[0]>4 || p[1]>4 || p[2]>4))
			{
				p[0] = pow(p[0],0.43);
				p[1] = pow(p[1],0.43);
				p[2] = pow(p[2],0.43);
			}
		}
	
}



extern cvar_t	*r_maxTextureSize;

qboolean GL_Upload32(unsigned *data, int width, int height,
					 qboolean mipmap)
{

	int samples;
	unsigned *scaled;
	int scaled_width, scaled_height;
	int i, c;
	byte *scan;
	int comp;
	uploaded_paletted = false;

	// scan the texture for any non-255 alpha
	c = width * height;
	scan = ((byte *) data) + 3;
	samples = 3;

	for (i = 0; i < c; i++, scan += 4) {
		if (*scan != 255) {
			samples = 4;
			break;
		}
	}

	if (samples == 3){

	if (gl_state.texture_compression_arb && mipmap)
		comp = GL_COMPRESSED_RGB_ARB;
	else 
		comp = gl_tex_solid_format;
	}

	if (samples == 4){

	if (gl_state.texture_compression_arb && mipmap)
		comp = GL_COMPRESSED_RGBA_ARB;
	else 
		comp = gl_tex_alpha_format;
	}
	
	// find sizes to scale to

	{
		int max_size;

		qglGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size);
		
		if(gl_state.nPot){
		scaled_width = width;
		scaled_height = height;
		}
		else
		{
		scaled_width = nearest_power_of_2(width);
		scaled_height = nearest_power_of_2(height);
		}
		
		if(r_maxTextureSize->value >= max_size)
			Cvar_SetValue("r_maxTextureSize", max_size);

		if(r_maxTextureSize->value <= 64 && r_maxTextureSize->value > 0 )
			Cvar_SetValue("r_maxTextureSize", 64);

		if(r_maxTextureSize->value)
			max_size =	r_maxTextureSize->value;
		
		if (scaled_width > max_size)
			scaled_width = max_size;
		if (scaled_height > max_size)
			scaled_height = max_size;
	}

	if (scaled_width == width && scaled_height == height) {
		scaled_width = width;
		scaled_height = height;
		scaled = data;
        
	} else {
		scaled = malloc((scaled_width * scaled_height) * 4);
        GL_ResampleTexture(data, width, height, scaled, scaled_width, scaled_height);
    }


	if (mipmap) {

		qglTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, true);
		qglTexImage2D(GL_TEXTURE_2D, 0, comp, scaled_width, scaled_height,
					  0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);

	} else {

		qglTexImage2D(GL_TEXTURE_2D, 0, comp, scaled_width, scaled_height,
					  0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
	}



	if (scaled_width != width || scaled_height != height)
		free(scaled);

	upload_width = scaled_width;
	upload_height = scaled_height;

	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (mipmap) ? gl_filter_min : gl_filter_max);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, r_anisotropic->value);
	
	return (samples == 4);
}




/*
===============
GL_Upload8

Returns has_alpha
===============
*/

qboolean GL_Upload8(byte * data, int width, int height, qboolean mipmap,
					qboolean is_sky)
{
	unsigned trans[512 * 256];
	int i, s;
	int p;

	s = width * height;

	if (s > sizeof(trans) / 4)
		VID_Error(ERR_DROP, "GL_Upload8: too large %d width %d height",
				  width, height);

	for (i = 0; i < s; i++) {
		p = data[i];
		trans[i] = d_8to24table[p];

		if (p == 255) {			// transparent, so scan around for another 
								// color
			// to avoid alpha fringes
			// FIXME: do a full flood fill so mips work...
			if (i > width && data[i - width] != 255)
				p = data[i - width];
			else if (i < s - width && data[i + width] != 255)
				p = data[i + width];
			else if (i > 0 && data[i - 1] != 255)
				p = data[i - 1];
			else if (i < s - 1 && data[i + 1] != 255)
				p = data[i + 1];
			else
				p = 0;
			// copy rgb components
			((byte *) & trans[i])[0] = ((byte *) & d_8to24table[p])[0];
			((byte *) & trans[i])[1] = ((byte *) & d_8to24table[p])[1];
			((byte *) & trans[i])[2] = ((byte *) & d_8to24table[p])[2];
		}
	}


	return GL_Upload32(trans, width, height, mipmap);
}


/*
================
GL_LoadPic

This is also used as an entry point for the generated r_notexture
================
*/

image_t *GL_LoadPic(char *name, byte * pic, int width, int height,
					imagetype_t type, int bits)
{
	image_t *image;
	int i;
	int len;
	char s[128];
	
	// find a free image_t
	for (i = 0, image = gltextures; i < numgltextures; i++, image++) {
		if (!image->texnum)
			break;
	}
	if (i == numgltextures) {
		if (numgltextures == MAX_GLTEXTURES)
			VID_Error(ERR_DROP, "MAX_GLTEXTURES");
		numgltextures++;
	}
	image = &gltextures[i];

	if (strlen(name) >= sizeof(image->name))
		VID_Error(ERR_DROP, "Draw_LoadPic: \"%s\" is too long", name);
	strcpy(image->name, name);
	image->registration_sequence = registration_sequence;

	image->width = width;
	image->height = height;
	image->picScale_w = 1.0;
	image->picScale_h = 1.0;
	image->type = type;
	
	if (type == it_skin && bits == 8)
		R_FloodFillSkin(pic, width, height);

	// Knightmare- Nexus's image replacement scaling code
	len = strlen(name);
	strcpy(s, name);
	// NOTE: once Q3 map support is added, be be sure to disable this for
	// Q3 format maps,
	// because they will be natively textured with hi-res textures.
	if (!strcmp(s + len - 4, ".tga") || !strcmp(s + len - 4, ".dds") || !strcmp(s + len - 4, ".png") || !strcmp(s + len - 4, ".jpg"))
	{
		miptex_t *mt;
		s[len - 3] = 'w';
		s[len - 2] = 'a';
		s[len - 1] = 'l';		// replace extension 
		FS_LoadFile(s, (void **) &mt);	// load .wal file 

		if (mt) {
			image->width = LittleLong(mt->width);	// grab size from wal
			image->height = LittleLong(mt->height);
			FS_FreeFile((void *) mt);	// free the wal
		}
		
	}
	
	if (image->type == it_pic && bits >= 24) //Scale hi-res pics
	{
		byte *pics, *palettes;
		int pcx_w, pcx_h;
		s[len - 3] = 'p';
		s[len - 2] = 'c';
		s[len - 1] = 'x';		
		LoadPCX(s, &pics, &palettes, &pcx_w, &pcx_h);	

		image->picScale_w = 1.0;
		image->picScale_h = 1.0;

		if (pcx_w > 0 && pcx_h > 0) {
			
			image->picScale_w = (float)pcx_w/image->width;
			image->picScale_h = (float)pcx_h/image->height;
		
			if (pics)
				free(pics);
			if (palettes)
				free(palettes);
			
		}
			
	}
	// load little pics into the scrap
	if (image->type == it_pic && bits == 8
		&& image->width < 64 && image->height < 64) {
		int x, y;
		int i, j, k;
		int texnum;

		texnum = Scrap_AllocBlock(image->width, image->height, &x, &y);
		if (texnum == -1)
			goto nonscrap;
		scrap_dirty = true;

		// copy the texels into the scrap block
		k = 0;
		for (i = 0; i < image->height; i++)
			for (j = 0; j < image->width; j++, k++)
				scrap_texels[texnum][(y + i) * BLOCK_SIZE + x + j] =
					pic[k];
		image->texnum = TEXNUM_SCRAPS + texnum;
		image->scrap = true;
		image->has_alpha = true;
		image->sl = (x + 0.01) / (float) BLOCK_SIZE;
		image->sh = (x + image->width - 0.01) / (float) BLOCK_SIZE;
		image->tl = (y + 0.01) / (float) BLOCK_SIZE;
		image->th = (y + image->height - 0.01) / (float) BLOCK_SIZE;
	} else {
	  nonscrap:
		image->scrap = false;
		image->texnum = TEXNUM_IMAGES + (image - gltextures);
		GL_Bind(image->texnum);
		if (bits == 8)
			image->has_alpha =
				GL_Upload8(pic, width, height, (image->type != it_pic && image->type != it_sky), image->type == it_sky);
		else {
									
		image->has_alpha = GL_Upload32(	(unsigned *) pic, width, height, (image->type != it_pic && image->type != it_sky));
					
		}
		if(gl_state.nPot){
		image->upload_width = width;	
		image->upload_height = height;
		}
		else
		{
		image->upload_width = upload_width;	// after power of 2 and scales
		image->upload_height = upload_height;
		}
		image->paletted = uploaded_paletted;
		image->sl = 0;
		image->sh = 1;
		image->tl = 0;
		image->th = 1;

	}

	return image;
}






void LoadImageErrors(void)
{
	ILenum Error;
	char message[2048];

	while ((Error = ilGetError()) != IL_NO_ERROR) {
		memset(message, 0, 2048);
		sprintf(message, "%d: %s", Error, iluErrorString(Error));
		Com_Printf("%s\n", message);
	}
}

void IL_LoadImage(char *filename, byte ** pic, int *width, int *height,
				  ILenum type)
{
	int length;
	unsigned char *buffer, *buf;
	ILubyte *image;
	ILuint imageID;
	signed int w, h;

	length = FS_LoadFile(filename, (void **) &buffer);
	if (!buffer) {
		Con_Printf(PRINT_DEVELOPER, "Bad image file %s\n", filename);
		return;
	}

	if (!length) {
		FS_FreeFile(buffer);
		Con_Printf(PRINT_DEVELOPER, "Bad image file %s\n", filename);
		return;
	}

	ilGenImages(1, &imageID);
	ilBindImage(imageID);

	if (!ilLoadL(type, (ILvoid *) buffer, (ILint) length)) {
		FS_FreeFile(buffer);
		free(buffer);
		LoadImageErrors();
		return;
	}

	if (!ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE)) {
		FS_FreeFile(buffer);
		LoadImageErrors();
		ilDeleteImages(1, &imageID);
		return;
	}

	w = ilGetInteger(IL_IMAGE_WIDTH);
	h = ilGetInteger(IL_IMAGE_HEIGHT);
	image = ilGetData();

	buf = (unsigned char*)malloc(w * h * 4);
	Q_memcpy(buf, image, w * h * 4);

	ilDeleteImages(1, &imageID);
	FS_FreeFile(buffer);

	*pic = buf;
	*width = w;
	*height = h;
	return;
}

/*
================
GL_LoadWal
================
*/
image_t *GL_LoadWal(char *name)
{
	miptex_t *mt;
	int width, height, ofs;
	image_t *image;
	
	FS_LoadFile(name, (void **) &mt);

	if (!mt) {
		Com_Printf("GL_FindImage: can't load %s\n", name);
		return r_notexture;
	}

	width = LittleLong(mt->width);
	height = LittleLong(mt->height);
	ofs = LittleLong(mt->offsets[0]);


	image = GL_LoadPic(name, (byte *) mt + ofs, width, height, it_wall, 8);

	FS_FreeFile((void *) mt);

	return image;
}



/*
===============
GL_FindImage

Finds or loads the given image
===============
*/
unsigned int nmap[4194304];
char override = 0;
image_t *GL_FindImage(char *name, imagetype_t type)
{
	image_t *image;
	int i, len;
	byte *pic, *palette;
	int width, height;
	
	
	if (!name)
		return NULL;			
	len = strlen(name);
	if (len < 5)
		return NULL;			


	// look for it
	for (i = 0, image = gltextures; i < numgltextures; i++, image++) {
		if (!strcmp(name, image->name)) {
			image->registration_sequence = registration_sequence;
			return image;
		}
	}

	// 
	// load the pic from disk
	// 

	pic = NULL;
	palette = NULL;
		
	if (strcmp(name + len - 4, ".png") && strcmp(name + len - 4, ".tga") && strcmp(name + len - 4, ".dds") && !override) {	// Targa override  crap

		char s[128];
		override = 1;
		strcpy(s, name);
		s[len - 3] = 't';
		s[len - 2] = 'g';
		s[len - 1] = 'a';

		image = GL_FindImage(s, type);
		if (image) {
			override = 0;
			return image;
		}
	}
	if (strcmp(name + len - 4, ".png") && strcmp(name + len - 4, ".tga") && strcmp(name + len - 4, ".dds") && !override) {	// Jpeg override  crap

		char s[128];
		override = 1;
		strcpy(s, name);
		s[len - 3] = 'j';
		s[len - 2] = 'p';
		s[len - 1] = 'g';

		image = GL_FindImage(s, type);
		if (image) {
			override = 0;
			return image;
		}
	}
	if (strcmp(name + len - 4, ".png") && strcmp(name + len - 4, ".tga") && strcmp(name + len - 4, ".dds") && !override) {	// Png override  crap
																															
		char s[128];
		override = 1;
		strcpy(s, name);
		s[len - 3] = 'p';
		s[len - 2] = 'n';
		s[len - 1] = 'g';

		image = GL_FindImage(s, type);
		if (image) {
			override = 0;
			return image;
		}
	}


	if (strcmp(name + len - 4, ".png") && strcmp(name + len - 4, ".tga") && strcmp(name + len - 4, ".dds") && !override) {	// DDS override  crap
																															
		char s[128];
		override = 1;
		strcpy(s, name);
		s[len - 3] = 'd';
		s[len - 2] = 'd';
		s[len - 1] = 's';

		image = GL_FindImage(s, type);
		if (image) {
			override = 0;
			return image;
		}
	}

	override = 0;
	if (!strcmp(name + len - 4, ".pcx")) {
		LoadPCX(name, &pic, &palette, &width, &height);

		if (!pic)
			return NULL;
	
	image = GL_LoadPic(name, pic, width, height, type, 8);

	} else if (!strcmp(name + len - 4, ".wal")) {

		image = GL_LoadWal(name);



	} else if (!strcmp(name + len - 4, ".tga")) {

		//	LoadTGA(name, &pic, &width, &height);
		IL_LoadImage(name, &pic, &width, &height, IL_TGA);

		if (!pic)
			return NULL;

		image = GL_LoadPic(name, pic, width, height, type, 32);
		
	}

	else if (!strcmp(name + len - 4, ".dds")) {
		IL_LoadImage(name, &pic, &width, &height, IL_DDS);

		if (!pic)
			return NULL;

		image = GL_LoadPic(name, pic, width, height, type, 32);
	}


	else if (!strcmp(name + len - 4, ".jpg")) {
		IL_LoadImage(name, &pic, &width, &height, IL_JPG);
		if (!pic)
			return NULL;

		image = GL_LoadPic(name, pic, width, height, type, 24);
	} else if (!strcmp(name + len - 4, ".png")) {
		IL_LoadImage(name, &pic, &width, &height, IL_PNG);
		if (!pic)
			return NULL;

		image = GL_LoadPic(name, pic, width, height, type, 32);
	}
	else
		return NULL;


	if (pic)
		free(pic);
	

	if (palette)
		free(palette);
	

	return image;
}





/*
===============
R_RegisterSkin
===============
*/

struct image_s *R_RegisterSkin(char *name)
{
	return GL_FindImage(name, it_skin);
}

struct image_s *R_RegisterPlayerBump (char *name, struct image_s *tex)
{
	image_t	*img;
	char	gl[48];

	if(!tex)
		img = r_defBump;
	
	strcpy(gl, name);
	gl[strlen(gl) - 4] = 0;
	strcat(gl, "_nm.tga");
	img = GL_FindImage (gl, it_skin);

	if (!img)
	{
		strcpy(gl, name);
		gl[strlen(gl) - 4] = 0;
		strcat(gl, "_nm.dds");
		img = GL_FindImage (gl, it_skin);
	}
	if(!img)
		img = r_defBump;

	return img;
}

/*
================
GL_FreeUnusedImages

Any image that was not touched on this registration sequence
will be freed.
================
*/

void GL_FreeUnusedImages(void)
{
	int i;
	image_t *image;

	// never free r_notexture or particle texture
	r_notexture->registration_sequence = registration_sequence;
	r_radarmap->registration_sequence = registration_sequence;
	r_around->registration_sequence = registration_sequence;
	r_flare->registration_sequence = registration_sequence;

	for (i = 0; i < PT_MAX; i++) {
		r_particletexture[i]->registration_sequence =
			registration_sequence;
	}

	for (i = 0; i < MAX_CAUSTICS; i++) {
		r_caustic[i]->registration_sequence = registration_sequence;
	}


	for (i = 0; i < MAX_FLY; i++) {
		fly[i]->registration_sequence = registration_sequence;
	}

	for (i = 0; i < MAX_FLAMEANIM; i++) {
		flameanim[i]->registration_sequence = registration_sequence;
	}

	for (i = 0; i < MAX_BLOOD; i++) {
		r_blood[i]->registration_sequence = registration_sequence;
	}
	
	for (i = 0; i < MAX_xBLOOD; i++) {
		r_xblood[i]->registration_sequence = registration_sequence;
	}


	for (i = 0; i < MAX_EXPLODE; i++) {
		r_explode[i]->registration_sequence = registration_sequence;
	}

	for (i = 0; i < DECAL_MAX; i++) {
		r_decaltexture[i]->registration_sequence = registration_sequence;
	}
	
	
	for (i = 0; i < MAX_SHELLS; i++){
		r_texshell[i]->registration_sequence = registration_sequence;
	}
	

	r_distort->registration_sequence = registration_sequence;
	r_blackTexture->registration_sequence = registration_sequence;
	r_defBump->registration_sequence = registration_sequence;
	r_scanline->registration_sequence = registration_sequence;
	r_envTex->registration_sequence = registration_sequence;

	for (i = 0, image = gltextures; i < numgltextures; i++, image++) {
		if (image->registration_sequence == registration_sequence)
			continue;			// used this sequence
		if (!image->registration_sequence)
			continue;			// free image_t slot
		if (image->type == it_pic)
			continue;			// don't free pics


		// free it
		qglDeleteTextures(1, &image->texnum);
		memset(image, 0, sizeof(*image));
	}
}


/*
===============
Draw_GetPalette
===============
*/
int Draw_GetPalette(void)
{
	int i;
	int r, g, b;
	unsigned v;
	byte  *pal;

	// load the palette
	pal = (byte*)r_originalPalette;

	for (i = 0; i < 256; i++) {
		r = pal[i * 3 + 0];
		g = pal[i * 3 + 1];
		b = pal[i * 3 + 2];

		v = (255 << 24) + (r << 0) + (g << 8) + (b << 16);
		d_8to24table[i] = LittleLong(v);

		d_8to24tablef[i][0] = r * 0.003921568627450980392156862745098f;
		d_8to24tablef[i][1] = g * 0.003921568627450980392156862745098f;
		d_8to24tablef[i][2] = b * 0.003921568627450980392156862745098f;

	}

	d_8to24table[255] &= LittleLong(0xffffff);	// 255 is transparent

	return 0;
}


/*
===============
GL_InitImages
===============
*/
void GL_InitImages(void)
{
	int i, j;
	float g = r_gamma->value;

	registration_sequence = 1;
		

	gl_state.inverse_intensity = 1;

	Draw_GetPalette();

	for (i = 0; i < 256; i++) {
		if (g == 1) {
			gammatable[i] = i;
		} else {
			float inf;

			inf =
				255 * pow((i + 0.5) * 0.0039138943248532289628180039138943,
						  g) + 0.5;
			if (inf < 0)
				inf = 0;
			if (inf > 255)
				inf = 255;
			gammatable[i] = inf;
		}
	}

	for (i = 0; i < 256; i++) {
		j = i;
		if (j > 255)
			j = 255;
		intensitytable[i] = j;
	}


}

/*
===============
GL_ShutdownImages
===============
*/

void GL_ShutdownImages(void)
{
	int i;
	image_t *image;
	GLuint t;

	for (i = 0, image = gltextures; i < numgltextures; i++, image++) {
		if (!image->registration_sequence)
			continue;			// free image_t slot

		// free it
		qglDeleteTextures(1, (GLuint*)&image->texnum);
		memset(image, 0, sizeof(*image));
	}

	// Berserker's fix for old Q2 bug:
	// free lightmaps
	if (gl_lms.current_lightmap_texture)
	{
		t = TEXNUM_LIGHTMAPS;
		qglDeleteTextures (gl_lms.current_lightmap_texture, &t);
	}

}
