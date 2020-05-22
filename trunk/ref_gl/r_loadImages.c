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
// r_misc.c

#include "r_local.h"


#ifndef _WIN32
#include <dlfcn.h>
#define qwglGetProcAddress( a ) dlsym( glw_state.hinstOpenGL, a )
#endif

/*
==================
R_InitEngineTextures

==================
*/

void CreateDSTtex(void) {
	image_t	*image;
	uchar	dist[16][16][4];
	int		i, x, y;

	for (x = 0; x < 16; x++)
		for (y = 0; y < 16; y++) {
			dist[x][y][0] = rand () % 255;
			dist[x][y][1] = rand () % 255;
			dist[x][y][2] = rand () % 48;
			dist[x][y][3] = rand () % 48;
		}

	// find a free image_t
	for (i = 0, image = gltextures; i < numgltextures; i++, image++) {
		if (!image->texnum)
			break;
	}
	if (i == numgltextures) {
		if (numgltextures == MAX_GLTEXTURES)
			VID_Error(ERR_FATAL, "MAX_GLTEXTURES");
		numgltextures++;
	}
	image = &gltextures[i];

	strcpy(image->name, "r_DSTTex");

	image->width = 16;
	image->height = 16;
	image->upload_width = 16;
	image->upload_height = 16;
	image->type = it_pic;
	qglGenTextures(1, &image->texnum);
	r_DSTTex = image;

	qglBindTexture(GL_TEXTURE_2D, r_DSTTex->texnum);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 16, 16);
	qglTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, dist);

	r_DSTTex->handle = glGetTextureHandleARB(r_DSTTex->texnum);
	glMakeTextureHandleResidentARB(r_DSTTex->handle);
}


image_t *R_CreateTexture(char *texName, uint targetTex, uint intFormat, uint format, uint type, uint width, uint height, uint warpS, uint warpT, uint filterMin, uint filterMag, uint imageType) {

	int		i;
	image_t* image;
	uint* pix = malloc(vid.width * vid.height * 4 * sizeof(uint));

	// find a free image_t
	for (i = 0, image = gltextures; i < numgltextures; i++, image++) {
		if (!image->texnum)
			break;
	}
	if (i == numgltextures) {
		if (numgltextures == MAX_GLTEXTURES)
			VID_Error(ERR_FATAL, "MAX_GLTEXTURES");
		numgltextures++;
	}
	image = &gltextures[i];

	strcpy(image->name, texName);

	image->width = width;
	image->height = height;
	image->upload_width = width;
	image->upload_height = height;
	image->type = type;
	qglGenTextures(1, &image->texnum);

	qglBindTexture	(targetTex, image->texnum);
	qglTexParameteri(targetTex, GL_TEXTURE_WRAP_S, warpS);
	qglTexParameteri(targetTex, GL_TEXTURE_WRAP_T, warpT);
	qglTexParameteri(targetTex, GL_TEXTURE_MIN_FILTER, filterMin);
	qglTexParameteri(targetTex, GL_TEXTURE_MAG_FILTER, filterMag);
	
	glTexStorage2D(targetTex, 1, intFormat, width, height);
	qglTexSubImage2D(targetTex, 0, 0, 0, width, height, format, imageType, pix);

	image->handle = glGetTextureHandleARB(image->texnum);
	glMakeTextureHandleResidentARB(image->handle);

	free(pix);

	return image;

}


/*
=============
R_FB_Check

Framebuffer must be bound.
=============
*/
static void FB_Check (const char *file, const int line) {
	const char	*s;
	GLenum		code;

	code = qglCheckFramebufferStatus (GL_FRAMEBUFFER);

	// an error occured
	switch (code) {
		case GL_FRAMEBUFFER_COMPLETE:
			return;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			s = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			s = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			s = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			s = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			s = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			s = "GL_FRAMEBUFFER_UNSUPPORTED";
			break;
		case GL_FRAMEBUFFER_UNDEFINED:
			s = "GL_FRAMEBUFFER_UNDEFINED";
			break;
	}

	Com_Printf ("R_FB_Check: %s, line %i: %s\n", file, line, s);
}

#define _R_FB_Check();		FB_Check(__FILE__, __LINE__);

void CreateSSAOBuffer(void) {
	qboolean statusOK;

	Com_Printf("Load "S_COLOR_YELLOW "SSAO FBO ");

	r_miniDepthTex = R_CreateTexture("***r_miniDepthTex***", GL_TEXTURE_RECTANGLE, GL_R16F, GL_RED, it_pic, vid.width / 2, vid.height / 2, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST, GL_FLOAT);
	
	for(int i = 0; i<2; i++)
		r_ssaoColorTex[i] = R_CreateTexture("***r_miniDepthTex***", GL_TEXTURE_RECTANGLE, GL_SRGB8, GL_RGB, it_pic, vid.width / 2, vid.height / 2, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_UNSIGNED_BYTE);

	r_ssaoColorTexIndex = 0;

	qglGenFramebuffers(1, &fboId);
	qglBindFramebuffer(GL_FRAMEBUFFER, fboId);
	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, r_ssaoColorTex[0]->texnum, 0);
	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE, r_ssaoColorTex[1]->texnum, 0);
	qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_RECTANGLE, r_miniDepthTex->texnum, 0);

	statusOK = qglCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	if (!statusOK)
		Com_Printf(S_COLOR_RED"Failed!");
	else
		Com_Printf(S_COLOR_WHITE"succeeded\n");

	qglBindFramebuffer(GL_FRAMEBUFFER, 0);

}


void Load3dLut(void) {
	int		i, j, len;
	char	name[MAX_OSPATH];
	char	checkname[MAX_OSPATH];
	char	*buf;
	image_t	*image;
	
	Com_Printf("\n======" S_COLOR_YELLOW " Load Color Lookup Tables " S_COLOR_WHITE "=====\n\n");
	lutCount = 0;
	for (j = 0; j < MAX_LUTS; j++) {

		Com_sprintf(name, sizeof(name), "***lut_%i***", j);

		// find a free image
		for (i = 0, image = gltextures; i < numgltextures; i++, image++) {
			if (!image->texnum)
				break;
		}
		if (i == numgltextures) {
			if (numgltextures == MAX_GLTEXTURES)
				VID_Error(ERR_FATAL, "MAX_GLTEXTURES");
			numgltextures++;
		}
		image = &gltextures[i];

		strcpy(image->name, name);

		image->width = vid.width;
		image->height = vid.height;
		image->upload_width = vid.width;
		image->upload_height = vid.height;
		image->type = it_pic;
		qglGenTextures(1, &image->texnum);
		r_3dLut[j] = image;

		Com_sprintf(checkname, sizeof(checkname), "gfx/lut/lut_%i.lut", j);
		len = FS_LoadFile(checkname, (void **)&buf);
		if (len < 0)
			continue;
		
		int LUTsize = buf[0];
		r_3dLut[j]->lutSize = (float)LUTsize;
		char *title = buf + sizeof(LUTsize) + (LUTsize * LUTsize * LUTsize) * sizeof(vec3_t);

		Com_Printf("Load LUT:" S_COLOR_GREEN " %s\n", title);

		qglBindTexture(GL_TEXTURE_3D, r_3dLut[j]->texnum);

		qglTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		qglTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		qglTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		qglTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

		qglTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, LUTsize, LUTsize, LUTsize, 0, GL_RGB, GL_FLOAT, buf + sizeof(LUTsize));

		FS_FreeFile(buf);
		lutCount++;

		image->handle = glGetTextureHandleARB(image->texnum);
		glMakeTextureHandleResidentARB(image->handle);

	}
	Com_Printf("\n=====================================\n\n");
}



char	*lsuf[6] = { "ft", "bk", "lf", "rt", "up", "dn" };
unsigned	trans[4096 * 4096];

void IL_LoadImage (char *filename, byte ** pic, int *width, int *height, ILenum type);
void R_FlipImage (int idx, img_t *pix, byte *dst) {
	byte *from;
	byte *src = pix->pixels;
	int	width = pix->width;
	int	height = pix->height;
	int	x, y;

	if (idx == 1)		// bk
	{
		for (y = height - 1; y >= 0; y--) {
			for (x = width - 1; x >= 0; x--) {	// copy rgb components
				from = src + (x*height + y) * 4;
				dst[0] = from[0];
				dst[1] = from[1];
				dst[2] = from[2];
				dst[3] = 255;
				dst += 4;
			}
		}
		return;
	}

	if (idx == 2)		// lf
	{
		for (y = height - 1; y >= 0; y--) {
			for (x = 0; x < width; x++) {	// copy rgb components
				from = src + (y*width + x) * 4;
				dst[0] = from[0];
				dst[1] = from[1];
				dst[2] = from[2];
				dst[3] = 255;
				dst += 4;
			}
		}
		return;
	}

	if (idx == 3)		// rt
	{
		for (y = 0; y < height; y++) {
			for (x = width - 1; x >= 0; x--) {	// copy rgb components
				from = src + (y*width + x) * 4;
				dst[0] = from[0];
				dst[1] = from[1];
				dst[2] = from[2];
				dst[3] = 255;
				dst += 4;
			}
		}
		return;
	}

	// ft, up, dn
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {	// copy rgb components
			from = src + (x*height + y) * 4;
			dst[0] = from[0];
			dst[1] = from[1];
			dst[2] = from[2];
			dst[3] = 255;
			dst += 4;
		}
	}
}

image_t *R_LoadLightFilter (int id) {
	int		i, minw, minh, maxw, maxh;
	image_t	*image;
	char	name[MAX_OSPATH];
	char	checkname[MAX_OSPATH];
	img_t	pix[6];
	byte	*nullpixels;
	qboolean	allNull = qtrue;

	Com_sprintf (name, sizeof(name), "***Filter%2i***", id + 1);

	// find a free image_t
	for (i = 0, image = gltextures; i < numgltextures; i++, image++) {
		if (!image->texnum)
			break;
	}
	if (i == numgltextures) {
		if (numgltextures == MAX_GLTEXTURES)
			Com_Error (ERR_FATAL, "MAX_GLTEXTURES");
		numgltextures++;
	}
	image = &gltextures[i];

	strcpy (image->name, name);
	image->registration_sequence = registration_sequence;
	image->type = it_pic;
	qglGenTextures (1, &image->texnum);

	qglBindTexture (GL_TEXTURE_CUBE_MAP, image->texnum);
	qglTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	qglTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	qglTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	qglTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	qglTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	minw = minh = 0;
	maxw = maxh = 9999999;
	for (i = 0; i < 6; i++) {
		pix[i].pixels = NULL;
		pix[i].width = pix[i].height = 0;
		Com_sprintf (checkname, sizeof(checkname), "gfx/lights/%i_%s.tga", id + 1, lsuf[i]);

		// Berserker: stop spam
		if (FS_LoadFile (checkname, NULL) != -1) {
			IL_LoadImage (checkname, &pix[i].pixels, &pix[i].width, &pix[i].height, IL_TGA);
			if (pix[i].width) {
				if (minw < pix[i].width)	
					minw = pix[i].width;
				if (maxw > pix[i].width)	
					maxw = pix[i].width;
			}

			if (pix[i].height) {
				if (minh < pix[i].height)	
					minh = pix[i].height;
				if (maxh > pix[i].height)	
					maxh = pix[i].height;
			}
		}
	}

	if ((minw == 0) || (minh == 0)) {
		///		Com_DPrintf("R_LoadLightFilter: filter %i does not exist\n", id+1);	// Berserker: stop spam
		minw = minh = maxw = maxh = 1;	// Для отсутствующего фильтра пусть будет фильтр 1х1 черный... (нет света)
	}

	if ((minw != maxw) || (minh != maxh) || (minw != minh))
		Com_Error (ERR_DROP, "R_LoadLightFilter: (%i) all images must be quadratic with equal sizes", id + 1);

	for (i = 0; i < 6; i++) {
		if (pix[i].pixels) {
			allNull = qfalse;
			R_FlipImage (i, &pix[i], (byte*)trans);
			free (pix[i].pixels);
			qglTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, minw, minh, 0, GL_RGBA, GL_UNSIGNED_BYTE, /*pix[i].pixels*/ trans);
		}
		else {
			nullpixels = (byte*)calloc (minw*minh * 4, 1);
			qglTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, minw, minh, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullpixels);
			free (nullpixels);
		}

	}

	image->width = minw;
	image->height = minh;
	image->upload_width = image->width * 6;
	image->upload_height = image->height * 6;

	image->handle = glGetTextureHandleARB(image->texnum);
	glMakeTextureHandleResidentARB(image->handle);

	if (allNull)
		image->registration_sequence = -1;	// free

	return image;
}


void R_InitEngineTextures (void) {
	int		i;
	static byte	notex[1][1][4]	= { 0x0, 0x0, 0x0, 0x0 };
	static byte	bump[1][1][4]	= { 0x80, 0x80, 0xff, 0x10 };
	static byte	white[1][1][4]	= { 0xff, 0xff, 0xff, 0xff };

	r_defBump	= GL_LoadPic ("***r_defBump***",	(byte *)bump, 1, 1, it_bump, 32);
	r_whiteMap	= GL_LoadPic ("***r_whiteMap***",	(byte *)white, 1, 1, it_bump, 32);
	r_notexture = GL_LoadPic ("***r_notexture***",	(byte *)notex, 1, 1, it_wall, 32);

	r_particletexture[PT_DEFAULT] = GL_FindImage ("gfx/particles/pt_blast.tga", it_wall);
	r_particletexture[PT_BUBBLE] = GL_FindImage ("gfx/particles/bubble.png", it_wall);
	r_particletexture[PT_FLY] = GL_FindImage ("gfx/fly/fly0.png", it_wall);
	r_particletexture[PT_BLOOD] = GL_FindImage ("gfx/particles/bloodTrail.tga", it_wall);	// default
	r_particletexture[PT_BLOOD2] = GL_FindImage ("gfx/particles/bloodTrail2.tga", it_wall);	// xatrix
	r_particletexture[PT_BLASTER] = GL_FindImage ("gfx/particles/pt_blast.tga", it_wall);
	r_particletexture[PT_SMOKE] = GL_FindImage ("gfx/particles/smoke.png", it_wall);
	r_particletexture[PT_SPLASH] = GL_FindImage ("gfx/particles/drop.tga", it_wall);
	r_particletexture[PT_SPARK] = GL_FindImage ("gfx/particles/spark.tga", it_wall);

	r_particletexture[PT_BEAM] = GL_FindImage ("gfx/particles/pt_beam.png", it_wall);
	r_particletexture[PT_SPIRAL] = GL_FindImage ("gfx/particles/pt_blast.tga", it_wall);
	r_particletexture[PT_FLAME] = GL_FindImage ("gfx/flame/fire_00.tga", it_wall);
	r_particletexture[PT_BLOODSPRAY] = GL_FindImage ("gfx/particles/bloodhit0.tga", it_wall);
	r_particletexture[PT_xBLOODSPRAY] = GL_FindImage ("gfx/particles/xbloodhit0.tga", it_wall);

	r_particletexture[PT_EXPLODE]		= GL_FindImage ("gfx/explode/rlboom_0.jpg", it_wall);
	r_particletexture[PT_WATERPULME]	= GL_FindImage ("gfx/particles/waterplume.tga", it_wall);
	r_particletexture[PT_WATERCIRCLE]	= GL_FindImage ("gfx/particles/water_ripples.dds", it_wall);
	r_particletexture[PT_BLOODDRIP]		= GL_FindImage ("gfx/particles/blooddrip.tga", it_wall);
	r_particletexture[PT_BLOODMIST]		= GL_FindImage ("gfx/particles/bloodmist.tga", it_wall);
	r_particletexture[PT_BLOOD_SPLAT]	= GL_FindImage ("gfx/decals/decal_splat.tga", it_wall);
	r_particletexture[PT_BLASTER_BOLT]	= GL_FindImage ("gfx/particles/blaster_bolt.tga", it_wall);
	r_particletexture[PT_BFG_BALL]		= GL_FindImage("gfx/bfg/bfgBall.tga", it_wall);
	r_particletexture[PT_BFG_REFR]		= GL_FindImage("gfx/bfg/bfgRefr.tga", it_wall);
	r_particletexture[PT_BFG_EXPL]		= GL_FindImage("gfx/bfg/bfg_expl.tga", it_wall);
	r_particletexture[PT_BFG_EXPL2]		= GL_FindImage("gfx/bfg/bfgballblast.tga", it_wall);
	r_particletexture[PT_FLARE]			= GL_FindImage("gfx/flares/flare0.tga", it_wall);

	for (i = 0; i < PT_MAX; i++)
	if (!r_particletexture[i])
		r_particletexture[i] = r_notexture;

	r_decaltexture[DECAL_RAIL] =
		GL_FindImage ("gfx/decals/decal_railgun.tga", it_wall);
	r_decaltexture[DECAL_BULLET] =
		GL_FindImage ("gfx/decals/decal_bullet2.tga", it_wall);
	r_decaltexture[DECAL_BLASTER] =
		GL_FindImage ("gfx/decals/decal_blaster2.tga", it_wall);
	r_decaltexture[DECAL_EXPLODE] =
		GL_FindImage ("gfx/decals/decal_explode.tga", it_wall);
	r_decaltexture[DECAL_BLOOD1] =
		GL_FindImage ("gfx/decals/decal_blood1.tga", it_wall);
	r_decaltexture[DECAL_BLOOD2] =
		GL_FindImage ("gfx/decals/decal_blood2.tga", it_wall);
	r_decaltexture[DECAL_BLOOD3] =
		GL_FindImage ("gfx/decals/decal_blood3.tga", it_wall);
	r_decaltexture[DECAL_BLOOD4] =
		GL_FindImage ("gfx/decals/decal_blood4.tga", it_wall);
	r_decaltexture[DECAL_BLOOD5] =
		GL_FindImage ("gfx/decals/decal_blood5.tga", it_wall);
	r_decaltexture[DECAL_BLOOD6] =
		GL_FindImage ("gfx/decals/decal_blood6.tga", it_wall);
	r_decaltexture[DECAL_BLOOD7] =
		GL_FindImage ("gfx/decals/decal_blood7.tga", it_wall);
	r_decaltexture[DECAL_BLOOD8] =
		GL_FindImage ("gfx/decals/decal_blood8.tga", it_wall);
	r_decaltexture[DECAL_BLOOD9] =
		GL_FindImage ("gfx/decals/decal_splat.tga", it_wall);
	r_decaltexture[DECAL_ACIDMARK] =
		GL_FindImage ("gfx/decals/decal_acidmark.tga", it_wall);
	r_decaltexture[DECAL_BFG] =
		GL_FindImage ("gfx/decals/decal_bfg.tga", it_wall);

	for (i = 0; i < DECAL_MAX; i++)
	if (!r_decaltexture[i])
		r_decaltexture[i] = r_notexture;

	for (i = 0; i < MAX_CAUSTICS; i++) {
		char name[MAX_QPATH];

		if (i < 10)
			Com_sprintf (name, sizeof(name), "gfx/caust/caust_0%i.tga", i);
		else
			Com_sprintf (name, sizeof(name), "gfx/caust/caust_%i.tga", i);
		r_caustic[i] = GL_FindImage (name, it_wall);
		if (!r_caustic[i])
			r_caustic[i] = r_notexture;
	}

	for (i = 0; i < MAX_BFG_EXPL; i++) {
		char name[MAX_QPATH];
		Com_sprintf(name, sizeof(name), "gfx/bfg/bfgExpl_%i.tga", i);

		r_bfg_expl[i] = GL_FindImage(name, it_wall);
		if (!r_bfg_expl[i])
			r_bfg_expl[i] = r_notexture;
	}

	for (i = 0; i < MAX_WATER_NORMALS; i++) {
		char name[MAX_QPATH];

		if (i < 10)
			Com_sprintf(name, sizeof(name), "gfx/water/00%iNormal.tga", i);
		else
			Com_sprintf(name, sizeof(name), "gfx/water/0%iNormal.tga", i);
		r_waterNormals[i] = GL_FindImage(name, it_bump);
		if (!r_waterNormals[i])
			r_waterNormals[i] = r_notexture;
	}

	for (i = 0; i < MAX_FLY; i++) {
		char frame[MAX_QPATH];
		Com_sprintf (frame, sizeof(frame), "gfx/fly/fly%i.png", i);
		fly[i] = GL_FindImage (frame, it_wall);
		if (!fly[i])
			fly[i] = r_notexture;
	}

	for (i = 0; i < MAX_FLAMEANIM; i++) {
		char frame2[MAX_QPATH];
		Com_sprintf (frame2, sizeof(frame2), "gfx/flame/fire_0%i.tga", i);
		flameanim[i] = GL_FindImage (frame2, it_wall);
		if (!flameanim[i])
			flameanim[i] = r_notexture;
	}


	for (i = 0; i < MAX_BLOOD; i++) {
		char bloodspr[MAX_QPATH];
		Com_sprintf (bloodspr, sizeof(bloodspr),
			"gfx/particles/bloodhit%i.tga", i);
		r_blood[i] = GL_FindImage (bloodspr, it_wall);
		if (!r_blood[i])
			r_blood[i] = r_notexture;

	}

	for (i = 0; i < MAX_xBLOOD; i++) {
		char xbloodspr[MAX_QPATH];

		Com_sprintf (xbloodspr, sizeof(xbloodspr),
			"gfx/particles/xbloodhit%i.tga", i);
		r_xblood[i] = GL_FindImage (xbloodspr, it_wall);
		if (!r_xblood[i])
			r_xblood[i] = r_notexture;

	}

	for (i = 0; i < MAX_EXPLODE; i++) {
		char expl[MAX_QPATH];
		Com_sprintf (expl, sizeof(expl), "gfx/explode/rlboom_%i.jpg", i);
		r_explode[i] = GL_FindImage (expl, it_wall);
		if (!r_explode[i])
			r_explode[i] = r_notexture;
	}

	for (i = 0; i < MAX_SHELLS; i++) {
		char shell[MAX_QPATH];
		Com_sprintf (shell, sizeof(shell), "gfx/shells/shell%i.tga", i);
		r_texshell[i] = GL_FindImage (shell, it_wall);
		if (!r_texshell[i])
			r_texshell[i] = r_notexture;
	}

	r_distort = GL_FindImage ("gfx/explosion/explosion.tga", it_wall);
	if (!r_distort)
		r_distort = r_notexture;

	r_conBump = GL_FindImage ("pics/conback_bump.tga", it_wall);
	if (!r_conBump)
		r_conBump = r_defBump;

	r_envTex = GL_FindImage ("gfx/tinfx.jpg", it_wall);
	if (!r_envTex)
		r_envTex = r_notexture;

	r_randomNormalTex = GL_FindImage ("gfx/randomNormal.png", it_pic);
	if (!r_randomNormalTex)
		r_randomNormalTex = r_notexture;

	for (i = 0; i < MAX_GLOBAL_FILTERS; i++)
		r_lightCubeMap[i] = R_LoadLightFilter (i);

	skinBump = GL_FindImage("gfx/skinBlend_bump.tga", it_bump);
	if (!skinBump)
		skinBump = r_notexture;

	CreateDSTtex();
	Load3dLut();

	r_depthTex		=	R_CreateTexture("***r_depthTex***", GL_TEXTURE_RECTANGLE, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, it_pic, vid.width, vid.height, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST, GL_UNSIGNED_INT);
	r_screenTex		=	R_CreateTexture("***r_screenTex***", GL_TEXTURE_RECTANGLE, GL_SRGB8, GL_RGB, it_pic, vid.width, vid.height, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_UNSIGNED_BYTE);
	r_fxaaTex		=	R_CreateTexture("***r_fxaaTex***", GL_TEXTURE_2D, GL_SRGB8, GL_RGB, it_pic, vid.width, vid.height, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_UNSIGNED_BYTE);
	r_fixFovTex		=	R_CreateTexture("***r_fixFovTex***", GL_TEXTURE_2D, GL_SRGB8, GL_RGB, it_pic, vid.width, vid.height, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_UNSIGNED_BYTE);
	r_cinImage		=	R_CreateTexture("***r_cinImage***", GL_TEXTURE_2D, GL_RGB8, GL_RGB, it_pic, 256, 256, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_UNSIGNED_BYTE);
	r_bloomImage	=	R_CreateTexture("***r_bloomImage***", GL_TEXTURE_RECTANGLE, GL_SRGB8, GL_RGB, it_pic, vid.width * 0.25, vid.height * 0.25, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_UNSIGNED_BYTE);
	r_thermalImage	=	R_CreateTexture("***r_thermalImage***", GL_TEXTURE_RECTANGLE, GL_SRGB8, GL_RGB, it_pic, vid.width * 0.5, vid.height * 0.5, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_UNSIGNED_BYTE);
}


/*
=============================
Universal GL_ScreenShot_f
with DevIL Lib
TGA JPG PNG BMP PCX support
=============================
*/
void GL_ScreenShot_f (void) {
	FILE	*file;
	char	picname[80], checkname[MAX_OSPATH];
	int		i, image = 0;
	ILuint	ImagesToSave[1];
	int		startTime, endTime;
	float	sec;

	startTime = Sys_Milliseconds ();

	if (Q_stricmp (r_screenShot->string, "jpg") != 0 &&
		Q_stricmp (r_screenShot->string, "tga") != 0 &&
		Q_stricmp (r_screenShot->string, "png") != 0 &&
		Q_stricmp (r_screenShot->string, "bmp") != 0 &&
		Q_stricmp (r_screenShot->string, "tif") != 0 &&
		Q_stricmp (r_screenShot->string, "pcx") != 0)
		Cvar_Set ("r_screenShot", "jpg");

	if (!Q_stricmp (r_screenShot->string, "tga"))
		image = IL_TGA;
	if (!Q_stricmp (r_screenShot->string, "png"))
		image = IL_PNG;
	if (!Q_stricmp (r_screenShot->string, "bmp"))
		image = IL_BMP;
	if (!Q_stricmp (r_screenShot->string, "pcx"))
		image = IL_PCX;
	if (!Q_stricmp (r_screenShot->string, "jpg"))
		image = IL_JPG;
	if (!Q_stricmp (r_screenShot->string, "tif"))
		image = IL_TIF;

	// Create the scrnshots directory if it doesn't exist
	Com_sprintf (checkname, sizeof(checkname), "%s/screenshots", FS_Gamedir ());
	Sys_Mkdir (checkname);

	for (i = 0; i <= 999; i++) {
		Com_sprintf (picname, sizeof(picname), "q2xp%04i.%s", i,
			r_screenShot->string);
		Com_sprintf (checkname, sizeof(checkname), "%s/screenshots/%s",
			FS_Gamedir (), picname);

		file = fopen (checkname, "rb");
		if (!file)
			break;				// file doesn't exist
		fclose (file);
	}

	if (i == 1000) {
		Com_Printf ("GL_ScreenShot_f: Couldn't create a file\n");
		return;
	}


	if ((r_screenShotJpegQuality->integer >= 99) || (r_screenShotJpegQuality->integer <= 0))
		Cvar_SetValue ("r_screenShotJpegQuality", 99);

	ilHint (IL_COMPRESSION_HINT, IL_USE_COMPRESSION);
	ilSetInteger (IL_JPG_QUALITY, r_screenShotJpegQuality->integer);

	ilGenImages (1, ImagesToSave);
	ilBindImage (ImagesToSave[0]);

	if (ilutGLScreen ())
		ilSave (image, checkname);


	ilDeleteImages (1, ImagesToSave);

	// Done!
	Com_Printf ("Wrote %s\n", picname);

	endTime = Sys_Milliseconds ();
	sec = (float)endTime - (float)startTime;
	Com_DPrintf ("Screenshot time: "S_COLOR_GREEN"%5.4f"S_COLOR_WHITE" sec\n", sec * 0.001);
}

void GL_LevelShot_f(void) {
	char	picname[80], checkname[MAX_OSPATH], shortName[MAX_QPATH];
	int		image = 0;
	ILuint	ImagesToSave[1];

	// Create the scrnshots levelshots if it doesn't exist
	Com_sprintf(checkname, sizeof(checkname), "%s/levelshots", FS_Gamedir());
	Sys_Mkdir(checkname);

	strcpy(shortName, r_worldmodel->name + 5); // skip "maps/"
	shortName[strlen(shortName) - 4] = 0; // skip ".bsp"

	Com_sprintf(picname, sizeof(picname), "%s.%s", shortName, "jpg");
	Com_sprintf(checkname, sizeof(checkname), "%s/levelshots/%s", FS_Gamedir(), picname);

	if ((r_screenShotJpegQuality->integer >= 99) || (r_screenShotJpegQuality->integer <= 0))
		Cvar_SetValue("r_screenShotJpegQuality", 99);

	ilHint(IL_COMPRESSION_HINT, IL_USE_COMPRESSION);
	ilSetInteger(IL_JPG_QUALITY, r_screenShotJpegQuality->integer);

	ilGenImages(1, ImagesToSave);
	ilBindImage(ImagesToSave[0]);

	if (ilutGLScreen())
		ilSave(IL_JPG, checkname);

	ilDeleteImages(1, ImagesToSave);

	// Done!
	Com_Printf("Wrote level shot %s\n", picname);

}

void GL_MakeSaveShot(char* dir) {
	char	name[MAX_QPATH];
	int		image = 0;
	ILuint	ImagesToSave[1];

	Com_sprintf(name, sizeof(name), "%s/save/%s/shot.jpg", FS_Gamedir(), dir);
	remove(name);

	if ((r_screenShotJpegQuality->integer >= 99) || (r_screenShotJpegQuality->integer <= 0))
		Cvar_SetValue("r_screenShotJpegQuality", 99);

	ilHint(IL_COMPRESSION_HINT, IL_USE_COMPRESSION);
	ilSetInteger(IL_JPG_QUALITY, r_screenShotJpegQuality->integer);

	ilGenImages(1, ImagesToSave);
	ilBindImage(ImagesToSave[0]);

	if (ilutGLScreen()) {
		iluGammaCorrect(0.5);
		ilSave(image, name);
	}

	ilDeleteImages(1, ImagesToSave);

	// Done!
	Com_Printf("Wrote save shot %s\n", name);

}
