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
==================
R_InitEngineTextures
==================
*/

image_t *r_caustic[MAX_CAUSTICS];
image_t *fly[MAX_FLY];
image_t *flameanim[MAX_FLAMEANIM];
image_t *r_flare;
image_t *r_blood[MAX_BLOOD];
image_t *r_explode[MAX_EXPLODE];
image_t *r_xblood[MAX_BLOOD];
image_t *r_distort;
image_t *r_predator;
image_t	*r_texshell[MAX_SHELLS];
image_t *r_blackTexture;
image_t *r_DSTTex;
image_t *r_scanline;
image_t	*r_envTex;

void CreateDSTTex_ARB (void)
{
	unsigned char	dist[16][16][4];
	int				x,y;

#ifdef _WIN32
	srand(GetTickCount());
#else
	srand(time(NULL));
#endif
	for (x=0; x<16; x++)
		for (y=0; y<16; y++) {
			dist[x][y][0] = rand()%255;
			dist[x][y][1] = rand()%255;
			dist[x][y][2] = rand()%48;
			dist[x][y][3] = rand()%48;
		}

	r_DSTTex =  GL_LoadPic("***r_DSTTex***", (byte *) dist, 16, 16, it_pic, 24);
	
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	qglHint(GL_GENERATE_MIPMAP_HINT_SGIS, GL_NICEST);
	qglTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
}

image_t *depthMap;

void CreateDepthTexture(void){

	
	int		i;
	char	name[15] = "***DepthTex***";
	image_t	*image;

	// find a free image_t
	for (i=0, image=gltextures ; i<numgltextures ; i++,image++)
	{
		if (!image->texnum)
			break;
	}
	if (i == numgltextures)
	{
		if (numgltextures == MAX_GLTEXTURES)
			VID_Error (ERR_FATAL, "MAX_GLTEXTURES");
		numgltextures++;
	}
	image = &gltextures[i];
	
	strcpy (image->name, name);

	image->width = vid.width;
	image->height = vid.height;
	image->upload_width = vid.width;
	image->upload_height = vid.height;
	image->type = it_pic;
	image->texnum = TEXNUM_IMAGES + (image - gltextures);

	depthMap = image;


	 // create depth texture
   
    qglBindTexture   ( GL_TEXTURE_RECTANGLE_ARB, depthMap->texnum );
    qglTexParameteri ( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    qglTexParameteri ( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	qglTexParameteri ( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); // rectangle!
    qglTexParameteri ( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); // rectangle!

    qglTexImage2D     ( GL_TEXTURE_RECTANGLE_ARB, 0, GL_DEPTH_COMPONENT24, vid.width, vid.height, 0,
                       GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL );

}


image_t *ScreenMap;

void CreateScreenRect(void){

	
	int		i;
	char	name[16] = "***ScreenMap***";
	image_t	*image;

	// find a free image_t
	for (i=0, image=gltextures ; i<numgltextures ; i++,image++)
	{
		if (!image->texnum)
			break;
	}
	if (i == numgltextures)
	{
		if (numgltextures == MAX_GLTEXTURES)
			VID_Error (ERR_FATAL, "MAX_GLTEXTURES");
		numgltextures++;
	}
	image = &gltextures[i];
	
	strcpy (image->name, name);

	image->width = vid.width;
	image->height = vid.height;
	image->upload_width = vid.width;
	image->upload_height = vid.height;
	image->type = it_pic;
	image->texnum = TEXNUM_IMAGES + (image - gltextures);

	ScreenMap = image;


	 // create screen texture
   
    qglBindTexture   (GL_TEXTURE_RECTANGLE_ARB, ScreenMap->texnum);
    qglTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    qglTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	qglTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    qglTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    qglTexImage2D     ( GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, vid.width, vid.height, 0,
                       GL_RGB, GL_UNSIGNED_BYTE, NULL );

}

image_t *shadowMask;

void CreateShadowMask(void){

	
	int		i;
	char	name[17] = "***shadowMask***";
	image_t	*image;

	// find a free image_t
	for (i=0, image=gltextures ; i<numgltextures ; i++,image++)
	{
		if (!image->texnum)
			break;
	}
	if (i == numgltextures)
	{
		if (numgltextures == MAX_GLTEXTURES)
			VID_Error (ERR_FATAL, "MAX_GLTEXTURES");
		numgltextures++;
	}
	image = &gltextures[i];
	
	strcpy (image->name, name);

	image->width = vid.width;
	image->height = vid.height;
	image->upload_width = vid.width;
	image->upload_height = vid.height;
	image->type = it_pic;
	image->texnum = TEXNUM_IMAGES + (image - gltextures);

	shadowMask = image;


	 // create shadow mask texture
   
    qglBindTexture   (GL_TEXTURE_RECTANGLE_ARB, shadowMask->texnum);
    qglTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    qglTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	qglTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    qglTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    qglTexImage2D     ( GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, vid.width, vid.height, 0,
                       GL_RGBA, GL_UNSIGNED_BYTE, NULL );

}

byte missing_texture[4][4] = {
	{0.0, 0.0, 0.0, 0.0},
	{0.0, 0.0, 0.0, 0.0},
	{0.0, 0.0, 0.0, 0.0},
	{0.0, 0.0, 0.0, 0.0},
};

image_t  *r_defBump;

void R_InitEngineTextures(void)
{
	int		x, y, i;
	byte	notex[4][4][4];
	byte bump[1][1][4] = {0x80, 0x80, 0xff,0x40};

	for (x = 0; x < 4; x++) {
		for (y = 0; y < 4; y++) {
			notex[y][x][0] = missing_texture[x][y] * 255;
			notex[y][x][1] = missing_texture[x][y] * 255;
			notex[y][x][2] = missing_texture[x][y] * 255;
			notex[y][x][3] = 255;
		}
	}

	r_defBump = GL_LoadPic("***r_defBump***", (byte *) bump, 1, 1, it_wall, 32);	


	r_notexture =
		GL_LoadPic("***r_notexture***", (byte *) notex, 4, 4, it_wall, 32);	
	
	r_particletexture[PT_DEFAULT] = GL_FindImage("gfx/particles/pt_blast.tga", it_wall);
	r_particletexture[PT_BUBBLE] = 	GL_FindImage("gfx/particles/bubble.png", it_wall);
	r_particletexture[PT_FLY] =		GL_FindImage("gfx/fly/fly0.png", it_wall);
	r_particletexture[PT_BLOOD] =	GL_FindImage("gfx/particles/bloodTrail.tga", it_wall);	// default
	r_particletexture[PT_BLOOD2] =	GL_FindImage("gfx/particles/bloodTrail2.tga", it_wall);	// xatrix
	r_particletexture[PT_BLASTER] =	GL_FindImage("gfx/particles/pt_blast.tga", it_wall);
	r_particletexture[PT_SMOKE] =	GL_FindImage("gfx/particles/smoke.png", it_wall);
	r_particletexture[PT_SPLASH] =	GL_FindImage("gfx/particles/drop.tga", it_wall);
	r_particletexture[PT_SPARK] =	GL_FindImage("gfx/particles/spark.tga", it_wall);

	r_particletexture[PT_BEAM] =	GL_FindImage("gfx/particles/pt_beam.png", it_wall);
	r_particletexture[PT_SPIRAL] =	GL_FindImage("gfx/particles/pt_blast.tga", it_wall);
	r_particletexture[PT_FLAME] =	GL_FindImage("gfx/flame/fire_00.tga", it_wall);
	r_particletexture[PT_BLOODSPRAY] =	GL_FindImage("gfx/particles/bloodhit0.tga", it_wall);
	r_particletexture[PT_xBLOODSPRAY] =	GL_FindImage("gfx/particles/xbloodhit0.tga", it_wall);

	r_particletexture[PT_EXPLODE] =		GL_FindImage("gfx/explode/rlboom_0.jpg", it_wall);
	r_particletexture[PT_WATERPULME] =	GL_FindImage("gfx/particles/waterplume.tga", it_wall);
	r_particletexture[PT_WATERCIRCLE] =	GL_FindImage("gfx/particles/water_ripples.dds", it_wall);
	r_particletexture[PT_BLOODDRIP] =	GL_FindImage("gfx/particles/blooddrip.tga", it_wall);
	r_particletexture[PT_BLOODMIST] =	GL_FindImage("gfx/particles/bloodmist.tga", it_wall);
	r_particletexture[PT_BLOOD_SPLAT] =	GL_FindImage("gfx/decals/decal_splat.tga", it_wall);
	r_particletexture[PT_BLASTER_BOLT] =GL_FindImage("gfx/particles/blaster_bolt.tga", it_wall);

	for (x = 0; x < PT_MAX; x++)
		if (!r_particletexture[x])
			r_particletexture[x] = r_notexture;
	
	r_decaltexture[DECAL_RAIL] =
		GL_FindImage("gfx/decals/decal_railgun.tga", it_wall);
	r_decaltexture[DECAL_BULLET] =
		GL_FindImage("gfx/decals/decal_bullet2.tga", it_wall);
	r_decaltexture[DECAL_BLASTER] =
		GL_FindImage("gfx/decals/decal_blaster2.tga", it_wall);
	r_decaltexture[DECAL_EXPLODE] =
		GL_FindImage("gfx/decals/decal_explode.tga", it_wall);
	r_decaltexture[DECAL_BLOOD1] =
		GL_FindImage("gfx/decals/decal_blood1.tga", it_wall);
	r_decaltexture[DECAL_BLOOD2] =
		GL_FindImage("gfx/decals/decal_blood2.tga", it_wall);
	r_decaltexture[DECAL_BLOOD3] =
		GL_FindImage("gfx/decals/decal_blood3.tga", it_wall);
	r_decaltexture[DECAL_BLOOD4] =
		GL_FindImage("gfx/decals/decal_blood4.tga", it_wall);
	r_decaltexture[DECAL_BLOOD5] =
		GL_FindImage("gfx/decals/decal_blood5.tga", it_wall);
	r_decaltexture[DECAL_BLOOD6] =
		GL_FindImage("gfx/decals/decal_blood6.tga", it_wall);
	r_decaltexture[DECAL_BLOOD7] =
		GL_FindImage("gfx/decals/decal_blood7.tga", it_wall);
	r_decaltexture[DECAL_BLOOD8] =
		GL_FindImage("gfx/decals/decal_blood8.tga", it_wall);
	r_decaltexture[DECAL_BLOOD9] =
		GL_FindImage("gfx/decals/decal_splat.tga", it_wall);
	r_decaltexture[DECAL_ACIDMARK] =
		GL_FindImage("gfx/decals/decal_acidmark.tga", it_wall);
	r_decaltexture[DECAL_BFG] =
		GL_FindImage("gfx/decals/decal_bfg.tga", it_wall);

	for (x = 0; x < DECAL_MAX; x++)
		if (!r_decaltexture[x])
			r_decaltexture[x] = r_notexture;

	for (i = 0; i < MAX_CAUSTICS; i++) {
		char name[MAX_QPATH];

		if (i < 10)
			Com_sprintf(name, sizeof(name), "gfx/caust/caust_0%i.jpg", i);
		else
			Com_sprintf(name, sizeof(name), "gfx/caust/caust_%i.jpg", i);
		r_caustic[i] = GL_FindImage(name, it_wall);
		if (!r_caustic[i])
			r_caustic[i] = r_notexture;

	}

	for (i = 0; i < MAX_FLY; i++) {
		char frame[MAX_QPATH];
		Com_sprintf(frame, sizeof(frame), "gfx/fly/fly%i.png", i);
		fly[i] = GL_FindImage(frame, it_wall);
		if (!fly[i])
			fly[i] = r_notexture;
	}

	for (i = 0; i < MAX_FLAMEANIM; i++) {
		char frame2[MAX_QPATH];
		Com_sprintf(frame2, sizeof(frame2), "gfx/flame/fire_0%i.tga", i);
		flameanim[i] = GL_FindImage(frame2, it_wall);
		if (!flameanim[i])
			flameanim[i] = r_notexture;
	}


	for (i = 0; i < MAX_BLOOD; i++) {
		char bloodspr[MAX_QPATH];
		Com_sprintf(bloodspr, sizeof(bloodspr),
					"gfx/particles/bloodhit%i.tga", i);
		r_blood[i] = GL_FindImage(bloodspr, it_wall);
		if (!r_blood[i])
			r_blood[i] = r_notexture;

	}
	
	for (i = 0; i < MAX_xBLOOD; i++) {
		char xbloodspr[MAX_QPATH];
		
		Com_sprintf(xbloodspr, sizeof(xbloodspr),
					"gfx/particles/xbloodhit%i.tga", i);
		r_xblood[i] = GL_FindImage(xbloodspr, it_wall);
		if (!r_xblood[i])
			r_xblood[i] = r_notexture;

	}

	for (i = 0; i < MAX_EXPLODE; i++) {
		char expl[MAX_QPATH];
		Com_sprintf(expl, sizeof(expl), "gfx/explode/rlboom_%i.jpg", i);
		r_explode[i] = GL_FindImage(expl, it_wall);
		if (!r_explode[i])
			r_explode[i] = r_notexture;
	}

	
	for (i = 0; i < MAX_SHELLS; i++) {
		char shell[MAX_QPATH];
		Com_sprintf(shell, sizeof(shell), "gfx/shells/shell%i.tga", i);
		r_texshell[i] = GL_FindImage(shell, it_wall);
		if (!r_texshell[i])
			r_texshell[i] = r_notexture;
	}
	
	r_flare = GL_FindImage("gfx/flares/flare0.tga", it_wall);
	if(!r_flare)
		r_flare = r_notexture;
	r_radarmap = GL_FindImage("gfx/radar/radarmap.tga", it_wall);
	if(!r_radarmap)
		r_radarmap = r_notexture;
	r_around = GL_FindImage("gfx/radar/around.tga", it_wall);
	if(!r_around)
		r_around = r_notexture;
		
	r_distort = GL_FindImage("gfx/distort/explosion.tga", it_wall);
	if(!r_distort)
		r_distort = r_notexture;

	r_predator = GL_FindImage("gfx/distort/modeldst.tga", it_wall);
	if(!r_predator)
		r_predator = r_notexture;

	r_blackTexture = GL_FindImage("gfx/blacktex.tga", it_wall);
		if(!r_blackTexture)
			r_blackTexture = r_notexture;
	
	r_scanline  = GL_FindImage("pics/conback_add.tga", it_wall);
		if(!r_scanline)
			r_scanline = r_notexture;
	
	r_envTex =  GL_FindImage("gfx/tinfx.jpg", it_wall);
		if(!r_envTex)
			r_envTex = r_notexture;

	CreateDSTTex_ARB();
	CreateDepthTexture();
	CreateScreenRect();
	CreateShadowMask();
}


/* 
============================= 
 Universal GL_ScreenShot_f  
	with DevIL Lib 
TGA JPG PNG BMP PCX support
============================= 
*/

void GL_ScreenShot_f(void)
{
	FILE *file;
	char picname[80], checkname[MAX_OSPATH];
	int i, image = 0;
	ILuint ImagesToSave[1];

	if (Q_stricmp(r_screenShot->string, "jpg") != 0 &&
		Q_stricmp(r_screenShot->string, "tga") != 0 &&
		Q_stricmp(r_screenShot->string, "png") != 0 &&
		Q_stricmp(r_screenShot->string, "bmp") != 0 &&
		Q_stricmp(r_screenShot->string, "tif") != 0 &&
		Q_stricmp(r_screenShot->string, "pcx") != 0)
			Cvar_Set("r_screenShot", "jpg");

	if (!Q_stricmp(r_screenShot->string, "tga"))
		image = IL_TGA;
	if (!Q_stricmp(r_screenShot->string, "png"))
		image = IL_PNG;
	if (!Q_stricmp(r_screenShot->string, "bmp"))
		image = IL_BMP;
	if (!Q_stricmp(r_screenShot->string, "pcx"))
		image = IL_PCX;
	if (!Q_stricmp(r_screenShot->string, "jpg"))
		image = IL_JPG;
	if (!Q_stricmp(r_screenShot->string, "tif"))
		image = IL_TIF;

	// Create the scrnshots directory if it doesn't exist
	Com_sprintf(checkname, sizeof(checkname), "%s/screenshots", FS_Gamedir());
	Sys_Mkdir(checkname);
	
	for (i = 0; i <= 999; i++) {
		Com_sprintf(picname, sizeof(picname), "q2xp%04i.%s", i,
					r_screenShot->string);
		Com_sprintf(checkname, sizeof(checkname), "%s/screenshots/%s",
					FS_Gamedir(), picname);

		file = fopen(checkname, "rb");
		if (!file)
			break;				// file doesn't exist
		fclose(file);
	}

	if (i == 1000) {
		Com_Printf("GL_ScreenShot_f: Couldn't create a file\n");
		return;
	}
	
	
	if ((r_screenShotJpegQuality->value >= 99) || (r_screenShotJpegQuality->value <= 0))
		Cvar_SetValue("r_screenShotJpegQuality", 99);

	ilHint(IL_COMPRESSION_HINT, IL_USE_COMPRESSION);
	ilSetInteger(IL_JPG_QUALITY, (int)r_screenShotJpegQuality->value);
	
	ilGenImages(1, ImagesToSave);
	ilBindImage(ImagesToSave[0]);

	if (ilutGLScreen()) {
		iluGammaCorrect(2.2 - r_gamma->value);
		ilSave(image, checkname);
	}

	ilDeleteImages(1, ImagesToSave);

	// Done!
	Com_Printf("Wrote %s\n", picname);
}


/*
** GL_Strings_f
*/
void GL_Strings_f(void)
{
	char *string = "";
#ifdef _WIN32
	string = (char*)glw_state.wglExtsString;
#endif
	Com_Printf("\n");
	Com_Printf("GL_VENDOR:   "S_COLOR_GREEN"%s\n",		gl_config.vendor_string);
	Com_Printf("GL_RENDERER: "S_COLOR_GREEN"%s\n",		gl_config.renderer_string);
	Com_Printf("GL_VERSION:  "S_COLOR_GREEN"%s\n\n",		gl_config.version_string);
	Com_Printf("WGL_EXTENSIONS:\n"S_COLOR_YELLOW"%s\n\n",	string);
	Com_Printf("GL_EXTENSIONS:\n"S_COLOR_YELLOW"%s\n\n",	gl_config.extensions_string);
}


/*
** GL_SetDefaultState
*/
extern unsigned int bloomtex;
extern unsigned int thermaltex;

void GL_SetDefaultState(void)
{
	
	bloomtex = 0;
	thermaltex = 0;

	qglClearColor		(0.35, 0.35, 0.35, 1);
	qglCullFace			(GL_FRONT);
	qglEnable			(GL_TEXTURE_2D);

	qglAlphaFunc		(GL_GREATER, 0.6f);

	qglDisable			(GL_DEPTH_TEST);
	qglDisable			(GL_CULL_FACE);
	qglDisable			(GL_STENCIL_TEST);
	qglDisable			(GL_BLEND);
	gl_state.blend		= (qboolean)false;

	qglColor4f			(1, 1, 1, 1);
	qglColorMask		(1, 1, 1, 1);

	qglHint				(GL_GENERATE_MIPMAP_HINT_SGIS,		GL_NICEST);
	qglHint				(GL_TEXTURE_COMPRESSION_HINT_ARB,	GL_NICEST);
	
	qglPolygonMode		(GL_FRONT_AND_BACK, GL_FILL);

	GL_TextureMode		(r_textureMode->string);
	
	qglTexParameterf	(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
	qglTexParameterf	(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

	qglTexParameterf	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	qglTexParameterf	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	qglBlendFunc		(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GL_PicsColorScaleARB(false);
	GL_TexEnv			(GL_REPLACE);

	gl_state.texgen		= (qboolean)false;
	qglDisable			(GL_TEXTURE_GEN_S);
	qglDisable			(GL_TEXTURE_GEN_T);
	qglDisable			(GL_TEXTURE_GEN_R);
	qglDisable			(GL_TEXTURE_GEN_Q);
	
	GL_UpdateSwapInterval();

}
