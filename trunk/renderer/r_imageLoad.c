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

/*
#ifndef _WIN32
#include <dlfcn.h>
#define qwglGetProcAddress( a ) dlsym( glw_state.hinstOpenGL, a )
#endif
*/
#ifndef _WIN32 // fix sdl bug
#include <SDL.h>
#define qwglGetProcAddress( a ) SDL_GL_GetProcAddress ( a )
#endif

/*
==================
R_InitEngineTextures

==================
*/

image_t *R_CreateTexture(char *texName, uint targetTex, 
						uint intFormat, uint format, 
						uint flags, uint width, uint height, 
						uint warpS, uint warpT, 
						uint filterMin, uint filterMag, 
						uint imageType, 
						uint *pixdata) {

	int		i;
	image_t* image;
	bool clearData;

	if (!pixdata) {
		pixdata = malloc(vid.width * vid.height * 4 * sizeof(uint));
		clearData = true;
	}
	else
		clearData = false;

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

	strcpy(image->name, texName);

	image->width = width;
	image->height = height;
	image->upload_width = width;
	image->upload_height = height;
	image->type = it_screen;
	image->flags = flags;
	image->hash = Com_HashKey(image->name);
	
	image->texType = targetTex;
	image->intFormat = intFormat;
	image->dataType = imageType;

	glCreateTextures(targetTex, 1, &image->texnum);
	glTextureParameteri(image->texnum, GL_TEXTURE_WRAP_S, warpS);
	glTextureParameteri(image->texnum, GL_TEXTURE_WRAP_T, warpT);
	glTextureParameteri(image->texnum, GL_TEXTURE_MIN_FILTER, filterMin);
	glTextureParameteri(image->texnum, GL_TEXTURE_MAG_FILTER, filterMag);

	if (image->flags & IF_MIPMAP) {
		image->numMips = CalcMipmapCount(width, height);
		glTextureParameteri(image->texnum, GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(image->texnum, GL_TEXTURE_MAX_LEVEL, image->numMips-1);
	}
	else
		image->numMips = 1;

	if(image->flags & IF_SHADOW)
		glTextureParameteri(image->texnum, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

	if (targetTex == GL_TEXTURE_CUBE_MAP) {
		int face;
		for (face = 0; face < 6; face++) {
			glTextureStorage2D(image->texnum, 1, intFormat, width, height);
			glTextureSubImage3D(image->texnum, 0, 0, 0, face, width, height, 1, format, imageType, pixdata);
		}
	}
	else {
		glTextureStorage2D(image->texnum, image->numMips, intFormat, width, height);
		glTextureSubImage2D(image->texnum, 0, 0, 0, width, height, format, imageType, pixdata);
	}
	image->handle = glGetTextureHandleARB(image->texnum);
	glMakeTextureHandleResidentARB(image->handle);

	qglObjectLabel(GL_TEXTURE, image->texnum, strlen(image->name), image->name);
	
	if (image->flags & IF_MIPMAP)
		glGenerateTextureMipmap(image->texnum);

	if(clearData)
		free(pixdata);

	return image;

}

//photoshop helper
/*
X+  -   rot90cc flip y
X-  -   rot90c	flip y
Y+  -   flip y
Y-  -   flip x 
Z+	-	rot90c flip x
Z-	-	rot90c flip x
*/

void R_FlipImage (int idx, img_t *pix, byte *dst) {
	byte *from;
	byte *src = pix->pixels;
	int	width = pix->width;
	int	height = pix->height;
	int	x, y;

	if (idx == 1)		// x neg
	{
		for (y = height - 1; y >= 0; y--) {
			for (x = width - 1; x >= 0; x--) {	// copy rgb components
				from = src + (x*height + y) * 4;
				dst[0] = from[0];
				dst[1] = from[1];
				dst[2] = from[2];
				dst += 3;
			}
		}
		return;
	}

	if (idx == 2)		// y plus
	{
		for (y = height - 1; y >= 0; y--) {
			for (x = 0; x < width; x++) {	// copy rgb components
				from = src + (y*width + x) * 4;
				dst[0] = from[0];
				dst[1] = from[1];
				dst[2] = from[2];
				dst += 3;
			}
		}
		return;
	}

	if (idx == 3)		// y neg
	{
		for (y = 0; y < height; y++) {
			for (x = width - 1; x >= 0; x--) {	// copy rgb components
				from = src + (y*width + x) * 4;
				dst[0] = from[0];
				dst[1] = from[1];
				dst[2] = from[2];
				dst += 3;
			}
		}
		return;
	}

	// x plus, z plus, z neg
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {	// copy rgb components
			from = src + (x*height + y) * 4;
			dst[0] = from[0];
			dst[1] = from[1];
			dst[2] = from[2];
			dst += 3;
		}
	}
}

byte	missingTexture[16][16] =
{
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
	{1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},
	{1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1},
	{1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1},
	{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
	{1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1},
	{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
	{1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1},
	{1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1},
	{1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},
	{1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

void R_InitEngineTextures (void) {
	int		i, x, y;
	char	name[MAX_QPATH];
	uint8_t	bump[1][1][4]	= { 0x80,	0x80,	0xff,	0x10 };
	uint8_t	black[1][1][4]	= { 0x0,	0x0,	0x0,	0x0 };
	uint8_t	white[1][1][4]	= { 0xff,	0xff,	0xff,	0xff };
	uint8_t mt[16][16][4];
	uint8_t wd[16][16][4];

	for (x = 0; x < 16; x++)
		for (y = 0; y < 16; y++) {
			wd[x][y][0] = rand() % 255;
			wd[x][y][1] = rand() % 255;
			wd[x][y][2] = rand() % 48;
			wd[x][y][3] = rand() % 48;
		}

	for (x = 0; x < 16; x++)
	{
		for (y = 0; y < 16; y++)
		{
			mt[y][x][0] = missingTexture[x][y] * 255;
			mt[y][x][1] = missingTexture[x][y] * 255;
			mt[y][x][2] = missingTexture[x][y] * 255;
			mt[y][x][3] = 255;
		}
	}

	gi.defBump = R_CreateTexture("***defBump***", GL_TEXTURE_2D, GL_RGBA8, GL_RGBA, 0, 1, 1,
	GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST, GL_UNSIGNED_BYTE, (uintptr_t*)bump);

	gi.whiteMap	= R_CreateTexture("***whiteMap***", GL_TEXTURE_2D, GL_RGBA8, GL_RGBA, 0, 1, 1,
	GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST, GL_UNSIGNED_BYTE, (uintptr_t *)white);

	gi.blackTexture1x1 = R_CreateTexture("***blackTexture1x1***", GL_TEXTURE_2D, GL_RGBA8, GL_RGBA, 0, 1, 1,
	GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST, GL_UNSIGNED_BYTE, (uintptr_t *)black);

	gi.missingTexture = R_CreateTexture("***missingTexture***", GL_TEXTURE_2D, GL_RGBA8, GL_RGBA, IF_MIPMAP, 16, 16,
	GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_UNSIGNED_BYTE, (uintptr_t *)mt);

	gi.waterDistort = R_CreateTexture("***waterDistort***", GL_TEXTURE_2D, GL_RGBA8, GL_RGBA, IF_MIPMAP, 16, 16,
	GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_UNSIGNED_BYTE, (uintptr_t *)wd);

	gi.cinematic = R_CreateTexture("***cinematic***", GL_TEXTURE_2D, GL_RGB8, GL_RGB, 0, 256, 256,
	GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, GL_UNSIGNED_BYTE, NULL);

	gi.particleTexture[PT_DEFAULT] = R_LoadDDS("gfx/particles/pt_blast.dds", it_part);
	gi.particleTexture[PT_BUBBLE] = R_LoadDDS("gfx/particles/bubble.dds", it_part);

	gi.particleTexture[PT_BLOOD] = R_LoadDDS("gfx/particles/bloodTrail.dds", it_part);	// default
	gi.particleTexture[PT_BLOOD2] = R_LoadDDS("gfx/particles/bloodTrail2.dds", it_part);	// xatrix
	gi.particleTexture[PT_BLASTER] = R_LoadDDS("gfx/particles/pt_blast.dds", it_part);
	gi.particleTexture[PT_SMOKE] = R_LoadDDS("gfx/particles/smoke.dds", it_part);
	gi.particleTexture[PT_SPLASH] = R_LoadDDS("gfx/particles/drop.dds", it_part);
	gi.particleTexture[PT_SPARK] = R_LoadDDS("gfx/particles/spark.dds", it_part);

	gi.particleTexture[PT_BEAM] = R_LoadDDS("gfx/particles/pt_beam.dds", it_part);
	gi.particleTexture[PT_SPIRAL] = R_LoadDDS("gfx/particles/pt_blast.dds", it_part);

	gi.particleTexture[PT_WATERPULME]	= R_LoadDDS("gfx/particles/waterplume.dds", it_part);
	gi.particleTexture[PT_WATERCIRCLE]	= R_LoadDDS("gfx/particles/waterripples.dds", it_part);
	gi.particleTexture[PT_BLOODDRIP]		= R_LoadDDS("gfx/particles/blooddrip.dds", it_part);
	gi.particleTexture[PT_BLOODMIST]		= R_LoadDDS("gfx/particles/bloodmist.dds", it_part);
	gi.particleTexture[PT_BLOOD_SPLAT]	= R_LoadDDS("gfx/decals/decal_splat.dds", it_part);
	gi.particleTexture[PT_BLASTER_BOLT]	= R_LoadDDS("gfx/particles/blaster_bolt.dds", it_part);
	gi.particleTexture[PT_BFG_LASER]		= R_LoadDDS("gfx/particles/bfglaser.dds", it_part);
	gi.particleTexture[PT_RAILBEAM]		= R_LoadDDS("gfx/particles/rail_beam.dds", it_part);

	gi.particleTexture[PT_BFG_BALL]		= R_LoadDDS("gfx/bfg/bfgBall.dds", it_part);
	gi.particleTexture[PT_BFG_REFR]		= R_LoadDDS("gfx/bfg/bfgRefr.dds", it_part);
	gi.particleTexture[PT_BFG_EXPL]		= R_LoadDDS("gfx/bfg/bfg_expl.dds", it_part);
	gi.particleTexture[PT_BFG_EXPL2]		= R_LoadDDS("gfx/bfg/bfgballblast.dds", it_part);

	for (i = 0; i < MAX_BFG_EXPL; i++) {
		Com_sprintf(name, sizeof(name), "gfx/bfg/bfgExpl_%i.dds", i);
		gi.bfgExplosionTex[i] = R_LoadDDS(name, it_part);
		if (!gi.bfgExplosionTex[i])
			gi.bfgExplosionTex[i] = gi.missingTexture;
	}

	gi.laserNormal = R_LoadDDS("gfx/particles/lasei_distort.dds", it_normal);
	if (!gi.laserNormal)
		gi.laserNormal = gi.defBump;

	for (i = 0; i < PT_MAX; i++)
	if (!gi.particleTexture[i])
		gi.particleTexture[i] = gi.missingTexture;

	gi.decalTexture[DECAL_RAIL]		= R_LoadDDS("gfx/decals/decal_railgun.dds", it_part);
	gi.decalTexture[DECAL_BULLET]	= R_LoadDDS("gfx/decals/decal_bullet2.dds", it_part);
	gi.decalTexture[DECAL_BLASTER]	= R_LoadDDS("gfx/decals/decal_blaster2.dds", it_part);
	gi.decalTexture[DECAL_EXPLODE]	= R_LoadDDS("gfx/decals/decal_explode.dds", it_part);
	gi.decalTexture[DECAL_BLOOD1]	= R_LoadDDS("gfx/decals/decal_blood1.dds", it_part);
	gi.decalTexture[DECAL_BLOOD2]	= R_LoadDDS("gfx/decals/decal_blood2.dds", it_part);
	gi.decalTexture[DECAL_BLOOD3]	= R_LoadDDS("gfx/decals/decal_blood3.dds", it_part);
	gi.decalTexture[DECAL_BLOOD4]	= R_LoadDDS("gfx/decals/decal_blood4.dds", it_part);
	gi.decalTexture[DECAL_BLOOD5]	= R_LoadDDS("gfx/decals/decal_blood5.dds", it_part);
	gi.decalTexture[DECAL_BLOOD6]	= R_LoadDDS("gfx/decals/decal_blood6.dds", it_part);
	gi.decalTexture[DECAL_BLOOD7]	= R_LoadDDS("gfx/decals/decal_blood7.dds", it_part);
	gi.decalTexture[DECAL_BLOOD8]	= R_LoadDDS("gfx/decals/decal_blood8.dds", it_part);
	gi.decalTexture[DECAL_BLOOD9]	= R_LoadDDS("gfx/decals/decal_splat.dds", it_part);
	gi.decalTexture[DECAL_ACIDMARK]	= R_LoadDDS("gfx/decals/decal_acidmark.dds", it_part);
	gi.decalTexture[DECAL_BFG]		= R_LoadDDS("gfx/decals/decal_bfg.dds", it_part);


	for (i = 0; i < DECAL_MAX; i++) {
		if (!gi.decalTexture[i])
			gi.decalTexture[i] = gi.missingTexture;
	}

	for (i = 0; i < MAX_CAUSTICS; i++) {
		if (i < 10)
			Com_sprintf (name, sizeof(name), "gfx/caust/caust_0%i.dds", i);
		else
			Com_sprintf (name, sizeof(name), "gfx/caust/caust_%i.dds", i);
		gi.causticTexture[i] = R_LoadDDS(name, it_wall);
		if (!gi.causticTexture[i])
			gi.causticTexture[i] = gi.missingTexture;
	}

	for (i = 0; i < MAX_WATER_NORMALS; i++) {
		if (i < 10)
			Com_sprintf(name, sizeof(name), "gfx/water/00%iNormal.dds", i);
		else
			Com_sprintf(name, sizeof(name), "gfx/water/0%iNormal.dds", i);
		gi.waterNormals[i] = R_LoadDDS(name, it_normal);
		if (!gi.waterNormals[i])
			gi.waterNormals[i] = gi.defBump;
	}

	for (i = 0; i < MAX_FLY; i++) {
		Com_sprintf (name, sizeof(name), "gfx/fly/fly%i.dds", i);
		gi.flyTexture[i] = R_LoadDDS(name, it_part);
		if (!gi.flyTexture[i])
			gi.flyTexture[i] = gi.missingTexture;
	}

	for (i = 0; i < MAX_FLAMEANIM; i++) {
		Com_sprintf (name, sizeof(name), "gfx/flame/fire_0%i.dds", i);
		gi.flameAnimTex[i] = R_LoadDDS(name, it_part);
		if (!gi.flameAnimTex[i])
			gi.flameAnimTex[i] = gi.missingTexture;
	}


	for (i = 0; i < MAX_BLOOD; i++) {
		Com_sprintf (name, sizeof(name), "gfx/particles/bloodhit%i.dds", i);
		gi.bloodTexture[i] = R_LoadDDS(name, it_part);
		if (!gi.bloodTexture[i])
			gi.bloodTexture[i] = gi.missingTexture;

	}

	for (i = 0; i < MAX_xBLOOD; i++) {
		Com_sprintf (name, sizeof(name), "gfx/particles/xbloodhit%i.dds", i);
		gi.xBloodTexture[i] = R_LoadDDS(name, it_part);
		if (!gi.xBloodTexture[i])
			gi.xBloodTexture[i] = gi.missingTexture;

	}

	for (i = 0; i < MAX_EXPLODE; i++) {
		Com_sprintf (name, sizeof(name), "gfx/explode/rlboom_%i.dds", i);
		gi.explosionTex[i] = R_LoadDDS(name, it_part);
		if (!gi.explosionTex[i])
			gi.explosionTex[i] = gi.missingTexture;
	}

	for (i = 0; i < MAX_SHELLS; i++) {
		Com_sprintf (name, sizeof(name), "gfx/shells/shell%i.dds", i);
		gi.aliasShellTex[i] = R_LoadDDS(name, it_wall);
		if (!gi.aliasShellTex[i])
			gi.aliasShellTex[i] = gi.missingTexture;
	}

	gi.distort = R_LoadDDS("gfx/explosion/explosion.dds", it_normal);
	if (!gi.distort)
		gi.distort = gi.defBump;

	gi.environment = R_LoadDDS("gfx/tinfx.dds", it_wall);
	if (!gi.environment)
		gi.environment = gi.missingTexture;

	gi.ssaoRandomNormal = R_LoadDDS("gfx/randomNormal.dds", it_screen);
	if (!gi.ssaoRandomNormal)
		gi.ssaoRandomNormal = gi.defBump;
	
	for (i = 0; i < MAX_GLOBAL_FILTERS; i++) {
		Com_sprintf(name, sizeof(name), "gfx/lights/lf_%i.dds", i+1);
		gi.lightCubeMaps[i] = R_LoadDDS(name, it_wall);
		if (!gi.lightCubeMaps[i])
			gi.lightCubeMaps[i] = gi.missingTexture;
	}

	gi.skinBump = R_LoadDDS("gfx/skinBlend_bump.dds", it_normal);
	if (!gi.skinBump)
		gi.skinBump = gi.defBump;

	gi.lensDirt = R_LoadDDS("gfx/lens_dirt.dds", it_screen);
	if (!gi.lensDirt)
		gi.lensDirt = gi.missingTexture;

	gi.lensBurst = R_LoadDDS("gfx/lens_burst.dds", it_part);
	if (!gi.lensBurst)
		gi.lensBurst = gi.missingTexture;

}


/*
=================
 GL_ScreenShot_f
=================
*/
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#define NUM_CHANNELS 3
#define screenSizeByte (vid.width * vid.height* 3 * sizeof(byte))
#define screenSizeFloat (vid.width * vid.height* 3 * sizeof(float))
#define DATESIZE 16 //dd.mm.yyyy align to 16

void Get_SysDate(bool us, char *out) {
	time_t	clock;
	struct	tm *tm;

	time(&clock);
	tm = localtime(&clock);
	
	// tm->tm_mon starts from 0
	// tm->tm_year starts from 1900
	if (!us)
		Com_sprintf(out, DATESIZE, "%02i.%02i.%04i", tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900); // rus date format
	else
		Com_sprintf(out, DATESIZE, "%02i.%02i.%04i", tm->tm_mon + 1, tm->tm_mday,  tm->tm_year + 1900); // us date format
}

void GL_ScreenShot_f (void) {
	FILE	*file;
	char	picname[MAX_OSPATH] = {0}, checkname[MAX_OSPATH], ext[MAX_OSPATH], date[DATESIZE];
	int		i, w, h;
	int		startTime, endTime;
	float	sec;
	
	startTime = Sys_Milliseconds ();

	if (Q_stricmp (r_screenShot->string, "tga") != 0 &&
		Q_stricmp (r_screenShot->string, "png") != 0 &&
		Q_stricmp (r_screenShot->string, "bmp") != 0 &&
		Q_stricmp(r_screenShot->string, "hdr") != 0 &&
		Q_stricmp (r_screenShot->string, "jpg") != 0)
		Cvar_Set ("r_screenShot", "jpg");
	
	w = vid.width;
	h = vid.height;

	// Create the scrnshots directory if it doesn't exist
	Com_sprintf (checkname, sizeof(checkname), "%s/scrnshots", FS_Gamedir ());
	Sys_Mkdir (checkname);

	Com_sprintf(ext, sizeof(ext), "%s/scrnshots/%s", FS_Gamedir(), r_screenShot->string);
	Sys_Mkdir(ext);

	Get_SysDate(false, date);

	for (i = 0; i <= 999; i++) {
		Com_sprintf (picname, sizeof(picname), "q2xp%04i_%s.%s", i, date, r_screenShot->string);
		Com_sprintf (checkname, sizeof(checkname), "%s/scrnshots/%s/%s", FS_Gamedir (), r_screenShot->string, picname);

		file = fopen (checkname, "rb");
		if (!file)
			break;				// file doesn't exist
		fclose (file);
	}

	if (i == 1000) {
		Com_Printf ("GL_ScreenShot_f: Couldn't create a file\n");
		return;
	}
	
	stbi_flip_vertically_on_write(1);

	if (!Q_stricmp(r_screenShot->string, "hdr")) {
		
		qglBindBuffer(GL_PIXEL_PACK_BUFFER, pbo._fullScreenF);
		qglReadPixels(0, 0, w, h, GL_RGB, GL_FLOAT, 0);
		
		GLfloat *hdrData = (GLfloat *)qglMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, screenSizeFloat, GL_MAP_READ_BIT);
		
		if (hdrData)
			stbi_write_hdr(checkname, w, h, NUM_CHANNELS, hdrData);
		
		qglUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	}
	else {
		qglBindBuffer(GL_PIXEL_PACK_BUFFER, pbo._fullScreen);
		qglReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, 0);

		GLbyte *data = (GLbyte *)qglMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, screenSizeByte, GL_MAP_READ_BIT);

		if (data){

			if (!Q_stricmp(r_screenShot->string, "tga"))
				stbi_write_tga(checkname, w, h, NUM_CHANNELS, data);

			if (!Q_stricmp(r_screenShot->string, "png"))
				stbi_write_png(checkname, w, h, NUM_CHANNELS, data, vid.width * NUM_CHANNELS);

			if (!Q_stricmp(r_screenShot->string, "jpg"))
				stbi_write_jpg(checkname, w, h, NUM_CHANNELS, data, 100); // max quality

			if (!Q_stricmp(r_screenShot->string, "bmp"))
				stbi_write_bmp(checkname, w, h, NUM_CHANNELS, data);
			
			qglUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		}		
	}
	qglBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	// Done!
	Com_Printf ("Wrote %s\n", picname);

	endTime = Sys_Milliseconds ();
	sec = (float)endTime - (float)startTime;
//	Com_Printf ("Screenshot time: "S_COLOR_GREEN"%5.4f"S_COLOR_WHITE" sec\n", sec * 0.001);

}

void GL_LevelShot_f(void) {
	char	picname[80] = {0}, checkname[MAX_OSPATH], shortName[MAX_QPATH];

	// Create the scrnshots levelshots if it doesn't exist
	Com_sprintf(checkname, sizeof(checkname), "%s/levelshots", FS_Gamedir());
	Sys_Mkdir(checkname);

	strcpy(shortName, r_worldmodel->name + 5); // skip "maps/"
	shortName[strlen(shortName) - 4] = 0; // skip ".bsp"

	Com_sprintf(picname, sizeof(picname), "%s.%s", shortName, "jpg");
	Com_sprintf(checkname, sizeof(checkname), "%s/levelshots/%s", FS_Gamedir(), picname);

	qglBindBuffer(GL_PIXEL_PACK_BUFFER, pbo._fullScreen);
	qglReadPixels(0, 0, vid.width, vid.height, GL_RGB, GL_UNSIGNED_BYTE, 0);

	GLbyte *data = (GLbyte *)qglMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, screenSizeByte, GL_MAP_READ_BIT);
	
	if (data) {
			stbi_write_jpg(checkname, vid.width, vid.height, NUM_CHANNELS, data, 100);
	}
	qglUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	qglBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	// Done!
	Com_Printf("Wrote level shot %s\n", & picname);

}

void GL_MakeSaveShot(char* dir) {
	char	name[MAX_QPATH];
	int		w, h;

	Com_sprintf(name, sizeof(name), "%s/savexp/%s/shot.jpg", FS_Gamedir(), dir);
	remove(name);

	w = vid.width;
	h = vid.height;

	qglBindBuffer(GL_PIXEL_PACK_BUFFER, pbo._fullScreen);
	qglReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, 0);

	GLbyte *data = (GLbyte *)qglMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, screenSizeByte, GL_MAP_READ_BIT);

	if (data) {
		stbi_flip_vertically_on_write(1);
		stbi_write_jpg(name, w, h, NUM_CHANNELS, data, 100);
	}

	qglUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	qglBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	// Done!
	Com_Printf("Wrote save shot %s\n", name);

}
