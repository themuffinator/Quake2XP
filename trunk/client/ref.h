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



#ifndef __REF_H
#define __REF_H

#include "../qcommon/qcommon.h"
#include <stdint.h>
#include <limits.h>

// Uncle Mike
#define FBitSet( iBitVector, bit )	((iBitVector) & (bit))
#define SetBits( iBitVector, bits )	((iBitVector) = (iBitVector) | (bits))
#define ClearBits( iBitVector, bits )	((iBitVector) = (iBitVector) & ~(bits))

typedef enum {
PT_DEFAULT,
PT_BUBBLE,
PT_FLY,
PT_BLOOD,
PT_BLOOD2,
PT_BLASTER,
PT_SMOKE,
PT_SPLASH,
PT_SPARK,
PT_BEAM,
PT_SPIRAL,
PT_FLAME,
PT_BLOODSPRAY,
PT_xBLOODSPRAY,
PT_EXPLODE,
PT_WATERPULME,
PT_WATERCIRCLE,
PT_BLOODDRIP,
PT_BLOODMIST,
PT_BLOOD_SPLAT,
PT_BLASTER_BOLT,
PT_BFG_BALL,
PT_BFG_REFR,
PT_BFG_EXPL,
PT_BFG_EXPL2,
PT_BFG_LASER,
PT_FLARE,
PT_RAILBEAM,
PT_MAX
}particleTex_t;

typedef enum {
DECAL_BULLET,
DECAL_BLASTER,
DECAL_EXPLODE,
DECAL_RAIL,
DECAL_BLOOD1,
DECAL_BLOOD2,
DECAL_BLOOD3,
DECAL_BLOOD4,
DECAL_BLOOD5,
DECAL_BLOOD6,
DECAL_BLOOD7,
DECAL_BLOOD8,
DECAL_BLOOD9,
DECAL_ACIDMARK,
DECAL_BFG,
DECAL_MAX
}decalTex_t;

typedef enum {
	DF_OVERBRIGHT	= BIT(0),
	DF_VERTEXLIGHT	= BIT(1),
	DF_TWOSIDE		= BIT(2)
} cDecalsFlagss_t;

typedef enum {
	PF_SCANLINE		= BIT(0),
	PF_LIGHT		= BIT(1),
	PF_VIGNETTE		= BIT(2),
	PF_CROSSHAIR	= BIT(3),
	PF_SRGBGAMMA	= BIT(4),
	PF_LOADSCREEN	= BIT(5),
	PF_VERTEXCOLOR	= BIT(6),
	PF_COLOREDFONT	= BIT(7),
	PF_MEDIANFILTER = BIT(8),
	PF_TECHCOLOR	= BIT(9),
	PF_SKIPAMBIENT	= BIT(10),
	PF_NOALPHA		= BIT(11)
} pictureFlags_t;


#define	MAX_DLIGHTS		32
#define	MAX_ENTITIES	128
#define	MAX_PARTICLES	4096
#define	MAX_LIGHTSTYLES	256

extern vec3_t r_origin;

typedef vec_t vec2_t[2];

typedef vec3_t	mat3_t[3];		// column-major (axis)
typedef vec4_t	mat4_t[4];		// row-major

#define DEG2RAD(v) ((v) * (M_PI / 180.0f))
#define RAD2DEG(v) ((v) * (180.0f / M_PI))

//
// msurface_t->flags
//
typedef enum {
	// vanila
	MSURF_PLANEBACK			= BIT(1), 
	MSURF_DRAWSKY			= BIT(2),
	MSURF_DRAWTURB			= BIT(4),
	MSURF_DRAWBACKGROUND	= BIT(6),
	MSURF_UNDERWATER		= BIT(7),
	//q2xp
	MSURF_ENVMAP			= BIT(3),
	MSURF_SSS				= BIT(5),
	MSURF_WATER				= BIT(8),
	MSURF_SLIME				= BIT(9),
	MSURF_LAVA				= BIT(10),
	MSURF_ALPHA				= BIT(11)
}msurfFlag_t;


#define POWERSUIT_SCALE		4.0F

#define SHELL_RED_COLOR		0xF2
#define SHELL_GREEN_COLOR	0xD0
#define SHELL_BLUE_COLOR	0xF3

#define SHELL_RG_COLOR		0xDC
//#define SHELL_RB_COLOR        0x86
#define SHELL_RB_COLOR		0x68
#define SHELL_BG_COLOR		0x78

//ROGUE
#define SHELL_DOUBLE_COLOR	0xDF	// 223
#define	SHELL_HALF_DAM_COLOR	0x90
#define SHELL_CYAN_COLOR	0x72
//ROGUE

#define SHELL_WHITE_COLOR	0xD7

typedef unsigned short		ushort;
typedef unsigned int		uint;
typedef unsigned long		ulong;

typedef const char			cchar;
typedef unsigned char		uchar;

// status bar head
float		headYaw;
float		headEndPitch;
float		headEndYaw;
int			headEndTime;
float		headStartPitch;
float		headStartYaw;
int			headStartTime;


typedef struct entity_s {
	
	struct model_s *model;		// opaque type outside refresh
	
	float	angles[3];
	mat3_t	axis;			// entity -> world space
	mat4_t	orMatrix, matrix;
	vec3_t	addColor;

	bool angleMod;
	/*
	 ** most recent data
	 */
	float	origin[3];				// also used as RF_BEAM's "from"
	int		frame;					// also used as RF_BEAM's diameter
	int		frameCount;				// for vis calc

	/*
	 ** previous data for lerping
	 */
	float	oldOrigin[3];			// also used as RF_BEAM's "to"
	int		oldFrame;
	
	// iqm
	int		iqmFrameTime;
	/*
	 ** misc
	 */
	float	backLerp;				// 0.0 = current, 1.0 = old
	int		skinnum;				// also used as RF_BEAM's palette index

	int		lightStyle;			// for flashing entities
	float	alpha;				// ignore if RF_TRANSLUCENT isn't set

	int		flags;
	vec3_t	color;

	struct	image_s *skin;		// NULL for inline skin
	struct	image_s	*bump;
	
	float	minmax[6];
	vec3_t	mins;
	vec3_t	maxs;
	byte	vis[MAX_MAP_LEAFS / 8];

} entity_t;

#define ENTITY_FLAGS  68

typedef struct {
	vec3_t		origin, color, angles;
	float		intensity, _cone;
	int			filter, style;
	bool	spotlight;

} dlight_t;

typedef struct {
	vec3_t	origin;
	vec3_t	mins;
	vec3_t	maxs;
	vec3_t	color;
	vec3_t	length;
	vec3_t	angle;
	vec3_t	oldOrg;
	vec3_t	dir;
	float	alpha;
	int		type;
	float	orient;
	float	len;
	int		flags;
	float	size;
	int		sFactor;
	int		dFactor;
	float	time;
} particle_t;

typedef enum {
	PARTICLE_BOUNCE			=	BIT(0),
	PARTICLE_FRICTION		=	BIT(1),
	PARTICLE_DIRECTIONAL	=	BIT(2),
	PARTICLE_VERTEXLIGHT	=	BIT(3),
	PARTICLE_STRETCH		=	BIT(4),
	PARTICLE_UNDERWATER		=	BIT(5),
	PARTICLE_OVERBRIGHT		=	BIT(6),
	PARTICLE_SPIRAL			=	BIT(7),
	PARTICLE_AIRONLY		=	BIT(8),
	PARTICLE_LIGHTING		=	BIT(9),
	PARTICLE_ALIGNED		=	BIT(10),
	PARTICLE_NONSOLID		=	BIT(11),
	PARTICLE_STOPED			=	BIT(12),
	PARTICLE_CLAMP			=	BIT(13),
	PARTICLE_NOFADE			=	BIT(14),
	PARTICLE_DEFAULT		=	BIT(15),
	PARTICLE_ROTATE			=	BIT(16),
	PARTICLE_SOFT_MIDLE		=	BIT(17),
	PARTICLE_DISTORT		=	BIT(18)
} cPatricleFlags_t;

typedef enum {
	CLM_BOUNCE		=	BIT(0),
	CLM_FRICTION	=	BIT(1),
	CLM_DIRECTIONAL	=	BIT(2),
	CLM_ROTATE		=	BIT(3),
	CLM_STOPPED		=	BIT(4),
	CLM_STRETCH		=	BIT(5),
	CLM_MSHELL		=	BIT(6),
	CLM_NOSHADOW	=	BIT(7)
} cModelFlags_t;

typedef struct {
	float rgb[3];				// 0.0 - 2.0
	float white;				// highest of rgb
} lightstyle_t;

typedef struct {
	float strength;
	vec3_t direction;
	vec3_t color;
} m_dlight_t;

void* Sys_LoadLibrary(const char* path, const char* sym, void** handle);
void* Sys_GetProcAddress(void* handle, const char* sym);
void Sys_FreeLibrary(void* handle);
/*

  skins will be outline flood filled and mip mapped
  pics and sprites with alpha will be outline flood filled
  pic won't be mip mapped

  model skin
  sprite frame
  wall texture
  pic

  */

typedef enum {
	it_skin,
	it_sprite,
	it_wall,
	it_pic,
	it_sky,
	it_normal,
	it_pbr,
	it_mipmap,
	it_nomips, 
	it_screen,
	it_part // clamp to edge mode
} imagetype_t;

typedef enum {
	IF_MIPMAP	= BIT(0),
	IF_CUBEMAP	= BIT(1),
	IF_SHADOW	= BIT(3)
} imageFlags_t;

vec3_t hColor;

typedef struct image_s {
	char		name[MAX_QPATH];		// game path, including extension
	char		bare_name[MAX_QPATH];	// filename only, as called when searching
	imagetype_t type;
	int			width, height;			// source image
	int			upload_width, upload_height;	// after power of two and picmip
	int			registration_sequence;	// 0 = free
	int			numMips;
	// drawing
	int			texnum;					// gl texture binding
	float		sl, tl, sh, th;		// 0,0 - 1,1 unless part of the scrap
	
	int			numChannels;
	uint		intFormat;
	uint		texType;
	uint		dataType;
	uint		flags;

	bool	floatTex;
	bool	compressed;

	bool	has_alpha;
	bool	paletted;
	bool	envMap;
	bool	legacySky;

	float		picScale_w;
	float		picScale_h;
	float		parallaxScale,
				specularScale,
				SpecularExp,
				envScale, 
				rghScale;

	//bindless graphics
	uint64_t		handle;
	
	//lut description
//	float		lutSize;
//	char		lutName[MAX_QPATH];
	vec3_t		reflectivity;
	uint		hash;
} image_t;


typedef struct mtexInfo_s {
	float vecs[2][4];
	int flags;
	int numFrames;
	struct mtexInfo_s *next;	// animation chain

	image_t *albedo;
	image_t *normalmap;
	image_t *emissive;
	image_t *envmap;
	image_t *pbr;
	image_t *maskmap;

	int value;

} mtexInfo_t;

int
c_brushTris,
c_lightBrushTris,
c_litAliasTris,
c_aliasTris,
c_numDynamicShadows,
c_numDynamicShadowsTris,
c_particlesTris,
c_decalsTris,
c_staticShadowTris,
c_numDips;
;

extern int c_numVisLights;

#define	VERTEXSIZE	16

typedef struct glpoly_s {

	struct	glpoly_s	*next;
	struct	glpoly_s	*chain;
	struct	glpoly_s	**neighbours;

	vec3_t	normal;
	vec3_t	center;
	int		lightTimestamp;
	int		lightTimestampRA;
	int		shadowTimestamp;
//	int		ShadowedFace;
	int		numVerts;
	int		flags;
	float	verts[4][VERTEXSIZE];	// variable sized (xyz s1t1 s2t2)
} glpoly_t;

//temporaly storage for polygons that use an edge
typedef struct {
	byte		used;		//how many polygons use this edge
	glpoly_t	*poly[2];	//pointer to the polygons who use this edge
} temp_connect_t;

temp_connect_t	*tempEdges;

typedef struct msurface_s {
	int visframe;				// should be drawn when node is crossed

	cplane_t *plane;
	int flags;

	int firstedge;				// look up in model->surfEdges[], negative
	// numbers
	int numEdges;				// are backwards edges

	short texturemins[2];
	short extents[2];

	int light_s, light_t;		// gl lightmap coordinates
	// lightmaps

	glpoly_t *polys;			// multiple if warped

	struct msurface_s *texturechain;
	struct msurface_s *lightmapchain;

	mtexInfo_t *texInfo;

	float c_s, c_t;

	// lighting info
	int dlightframe;
	int dlightbits;

	byte styles[MAXLIGHTMAPS];
	float cached_light[MAXLIGHTMAPS];	// values currently used in
	// lightmap
	byte *samples;				// [numstyles * surfsize * 3] for vanilla or [numstyles * 3 * surfsize * 3] for XP lightmaps

	int checkCount;
	vec3_t center;

	struct msurface_s *fogchain;
	int fragmentframe;
	entity_t *ent;
	vec3_t		mins, maxs;

	int	numIndices;
	int	numVertices;
	unsigned int sort;

	//vbo
	size_t vbo_pos;
	int	xyz_size;
	int st_size;
	int lm_size;

	int	baseIndex;
	int numIdx;

} msurface_t;

typedef struct mnode_s {
	// common with leaf
	int contents;				// -1, to differentiate from leafs
	int visframe;				// node needs to be traversed if current

	float minmaxs[6];			// for bounding box culling

	struct mnode_s *parent;

	// node specific
	cplane_t *plane;
	struct mnode_s *children[2];

	unsigned short firstsurface;
	unsigned short numsurfaces;
} mnode_t;

/*
=====================
DECALS

=====================
*/

#define MAX_DECALS				8192
#define MAX_DECAL_VERTS			512
#define MAX_DECAL_FRAGMENTS		384

typedef struct decals_t {
	struct decals_t *prev, *next;
	mnode_t *node;

	float time, endTime;

	int			numVerts;
	int			numIndices;
	uint32_t	*indices;

	vec3_t verts[MAX_DECAL_VERTS];
	vec2_t st[MAX_DECAL_VERTS];
	vec3_t color;
	vec3_t endColor;
	float alpha;
	float endAlpha;
	float size;
	vec3_t org;
	vec3_t normal;
	int type;
	int flags;
	int sFactor;
	int dFactor;
} decals_t;



typedef struct {
	mnode_t *node;
	msurface_t *surf;

	int firstVert;
	int numVerts;
} fragment_t;

//================
// end decals
//================


typedef struct {

	uint	_fullScreen;
	uint	_fullScreenF;
}pbo_t;

pbo_t pbo;

typedef struct {
	int		x, y, width, height;	// in virtual screen coordinates
	uint	virtualWidth, virtualHeight;
	float	fov_x, fov_y;
	float	vieworg[3];
	float	vieworg_old[3];
	float	viewangles[3];
	float	viewanglesOld[3];

	float blend[4];				// rgba 0-1 full screen blend
	float time;					// time is uesed to auto animate
	float hdrTime;
	int rdflags;				// RDF_UNDERWATER, etc
	bool mirrorView;
	byte *areabits;				// if not NULL, only areas with set bits
	// will be drawn

	// world bounds
	vec3_t	visBounds[2];
	float	zFar;
	lightstyle_t *lightstyles;	// [MAX_LIGHTSTYLES]

	// viewport
	int		viewport[4];
	vec3_t	cornerRays[4];
	vec2_t	depthParms;
	mat3_t	axis;
	mat4_t	projectionMatrix;
	mat4_t	orthoMatrix;
	mat4_t	modelViewMatrix;
	mat4_t	modelViewProjectionMatrix;
	mat4_t	modelViewProjectionMatrixTranspose;
	mat4_t	skyMatrix;

	int num_entities;
	entity_t *entities;

	int num_dlights;
	dlight_t *dlights;

	int num_particles;
	particle_t *particles;

	int numDecals;
	decals_t *decals;
} refdef_t;

extern float loadScreenColorFade;
extern char *sInf;

//#define	API_VERSION		3

//
// these are the functions exported by the refresh module
//

int R_GetClippedFragments (vec3_t origin, float radius, mat3_t axis,
	int maxfverts, vec3_t * fverts, int maxfragments,
	fragment_t * fragments);

void R_Draw_StretchPic(int x, int y, int w, int h, int flags, image_t *image, image_t *imageBump);
void Draw_StretchPic(int x, int y, int w, int h, int flags, char *imageName, char *imageName2);

void Draw_PicScaled (int x, int y, float scaleX, float scaleY, int flags, char *pic, char *pic2);
void Draw_ScaledPic (int x, int y, float scaleX, float scaleY, int flags, image_t *gl, image_t *gl2);

//void Draw_PicBumpScaled(int x, int y, float scale_x, float scale_y, char* pic, char* pic2);
//void Draw_ScaledBumpPic(int x, int y, float scale_x, float scale_y, image_t* gl, image_t* gl2);

void Draw_Fill (int x, int y, int w, int h, float r, float g, float b, float a, bool loading);
void R_BeginRegistration (char *map);
void R_SetSky (char *name, float rotate, vec3_t axis);
void R_EndRegistration (void);
void R_RenderFrame (refdef_t * fd);
void Draw_GetPicSize (int *w, int *h, char *name);	// will return 0 0 if
// not found

void R_ModelRadius (struct model_s * model, vec3_t rad);
void R_ModelCenter (struct model_s * model, vec3_t center);

void R_Shutdown (void);
bool R_CullPoint (vec3_t org);
int R_Init (void *hinstance, void *wndproc);
image_t *Draw_FindPic (char *name);
struct model_s *R_RegisterModel (char *name);
struct image_s *R_RegisterSkin (char *name);
image_t *Draw_FindPic (char *name);
void R_SetPalette (const unsigned char *palette);
void R_BeginFrame ();
void GLimp_EndFrame (void);
void GLimp_AppActivate (bool active);
void VID_NewWindow (int width, int height);
void VectorNormalizeFast(vec3_t v);

struct sfx_s;

//
// these are the functions imported by the refresh module
//

void Con_Printf (int print_level, char *str, ...);
cvar_t *Cvar_Get (char *name, char *value, int flags);
void Cvar_SetValue (char *name, float value);
cvar_t *Cvar_Set (char *name, char *value);
void Cmd_AddCommand (char *name, void (*cmd) (void));
void Cmd_RemoveCommand (char *name);
int Cmd_Argc (void);
char *Cmd_Argv (int i);
void VID_Error (int err_level, char *str, ...);
void Cbuf_ExecuteText (int exec_when, char *text);
int FS_LoadFile (const char *name, void **buf);
void FS_FreeFile (void *buf);
char *FS_Gamedir (void);

#endif							// __REF_H
