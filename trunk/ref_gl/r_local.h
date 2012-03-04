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

#ifndef R_LOCAL_H
#define R_LOCAL_H


#ifdef _WIN32

#include <windows.h>


#endif

#include <stdio.h>
#include <math.h>


#include <GL/glu.h>



#include "r_particle.h"


#include "glext.h"
#include <math.h>

#ifdef _WIN32
#include "wglext.h"

#include "imagelib/il.h"
#include "imagelib/ilu.h"
#include "imagelib/ilut.h"

#else
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>
#endif

#include "../client/ref.h"

#include "qgl.h"

#ifndef GL_NV_multisample_coverage
#define GL_COVERAGE_SAMPLES_NV            0x80A9
#define GL_COLOR_SAMPLES_NV               0x8E20
#endif

#ifndef WGL_NV_multisample_coverage
#define WGL_COVERAGE_SAMPLES_NV           0x2042
#define WGL_COLOR_SAMPLES_NV              0x20B9
#endif

// up / down
#define	PITCH	0

// left / right
#define	YAW		1

// fall over
#define	ROLL	2


//#ifndef __VIDDEF_T
//#define __VIDDEF_T
#ifndef VIDDEF_LOCK
#define VIDDEF_LOCK
typedef struct {
	unsigned width, height;		// coordinates from main game
} viddef_t;
#endif

extern viddef_t vid;


 
#define	TEXNUM_SCRAPS		1152
#define	TEXNUM_IMAGES		1153

#define	TEXNUM_LIGHTMAPS	8192
#define	MAX_GLTEXTURES		8192 


 // ===================================================================

#define GL_SMOOTH_LINE_WIDTH_RANGE 0x0B22
extern float lineAAwidth;

typedef enum {
	rserr_ok,

	rserr_invalid_fullscreen,
	rserr_invalid_mode,

	rserr_unknown
} rserr_t;

#include "r_model.h"

void GL_BeginRendering(int *x, int *y, int *width, int *height);
void GL_EndRendering(void);

void GL_SetDefaultState(void);
void GL_UpdateSwapInterval(void);

extern float gldepthmin, gldepthmax;


typedef struct {
	float x, y, z;
	float s, t;
	float r, g, b;
} glvert_t;



#define BACKFACE_EPSILON	0.01


//====================================================


#define		MAX_CAUSTICS		32
extern image_t *r_caustic[MAX_CAUSTICS];

#define		MAX_FLY		2
extern image_t *fly[MAX_FLY];

#define		MAX_FLAMEANIM		5
extern image_t *flameanim[MAX_FLAMEANIM];

#define		MAX_BLOOD 6
extern image_t *r_blood[MAX_BLOOD];
#define		MAX_xBLOOD 6
extern image_t *r_xblood[MAX_BLOOD];

#define	MAX_SHELLS 6
extern	image_t	*r_texshell[MAX_SHELLS];

#define		MAX_EXPLODE 8
extern image_t *r_explode[MAX_EXPLODE];

#define MAX_MODEL_DLIGHTS 128

extern qboolean drawFlares;
extern image_t gltextures[MAX_GLTEXTURES];
extern int numgltextures;
extern image_t *r_notexture;
extern image_t *r_distort;
extern image_t *r_predator;
extern image_t *depthMap;

image_t *r_particletexture[PT_MAX];
image_t *r_decaltexture[DECAL_MAX];

extern image_t *r_radarmap;
extern image_t *r_around;
extern image_t *r_flare;

extern image_t *sun_object;
extern image_t *sun1_object;
extern image_t *sun2_object;

extern image_t *draw_chars;
extern image_t *r_DSTTex;
extern image_t *r_blackTexture;

extern image_t	*r_defBump;
extern image_t	*ScreenMap;
extern image_t	*r_envTex;
extern image_t	*shadowMask;

extern entity_t *currententity;
extern model_t *currentmodel;
extern int r_visframecount;
extern int r_framecount;
extern cplane_t frustum[4];

extern	int gl_filter_min, gl_filter_max;
extern	int ocQueries[MAX_FLARES];
//
// view origin
//
extern vec3_t vup;
extern vec3_t vpn;
extern vec3_t vright;
extern vec3_t r_origin;
extern entity_t r_worldentity;

typedef vec_t mat4x4_t[16];

typedef vec3_t	mat3_t[3];		// column-major (axis)
typedef vec4_t	mat4_t[4];		// row-major


//
// screen size info
//
extern refdef_t r_newrefdef;
extern int r_viewcluster, r_viewcluster2, r_oldviewcluster,
	r_oldviewcluster2;

cvar_t *r_noRefresh;
cvar_t *r_drawEntities;
cvar_t *r_drawWorld;
cvar_t *r_speeds;
cvar_t *r_noVis;
cvar_t *r_noCull;
cvar_t *r_leftHand;
cvar_t *r_lightLevel;	
cvar_t *r_mode;
cvar_t *r_dynamic;
cvar_t *r_noBind;
cvar_t *r_cull;
cvar_t *r_polyBlend;
cvar_t *r_vsync;
cvar_t *r_textureMode;
cvar_t *r_lockPvs;
cvar_t *r_fullScreen;
cvar_t *r_gamma;
cvar_t *vid_ref;
cvar_t *r_finish;
cvar_t *r_hardwareGamma;

cvar_t	*r_causticIntens;

cvar_t	*r_displayRefresh;

cvar_t	*r_screenShot;
cvar_t	*r_screenShotJpegQuality;

cvar_t	*r_worldColorScale;
cvar_t	*r_picsColorScale;
cvar_t	*r_textureCompression;
cvar_t	*r_anisotropic;
cvar_t	*r_maxAnisotropy;

cvar_t	*r_shadows;
cvar_t	*r_shadowWorldLightScale;
cvar_t	*r_shadowVolumesDebug;
cvar_t	*r_playerShadow;
cvar_t	*r_shadowCapOffset;

cvar_t	*r_radarSize;			// GLOOM radar
cvar_t	*r_radarZoom;
cvar_t	*r_radar;

cvar_t	*r_arbSamples;
cvar_t	*r_nvSamplesCoverange;

cvar_t	*deathmatch;

cvar_t	*r_drawFlares;
cvar_t	*r_flaresIntens;
cvar_t	*r_flareWeldThreshold;

cvar_t	*r_customWidth;
cvar_t	*r_customHeight;

cvar_t	*r_bloom;
cvar_t	*r_bloomThreshold;
cvar_t	*r_bloomIntens;

cvar_t	*sys_priority;

cvar_t	*r_DrawRangeElements;
cvar_t	*r_bumpAlias;
cvar_t	*r_bumpWorld;
cvar_t	*r_pplWorldAmbient;
cvar_t	*r_pplMaxDlights;

cvar_t	*hunk_bsp;
cvar_t	*hunk_model;
cvar_t	*hunk_sprite;

//cvar_t	*r_vbo;
cvar_t	*r_maxTextureSize;

cvar_t	*r_parallax;
cvar_t	*r_parallaxScale;

cvar_t	*r_dof;
cvar_t	*r_dofBias;
cvar_t	*r_dofFocus;

cvar_t	*r_radialBlur;
cvar_t	*r_radialBlurFov;
cvar_t	*r_tbnSmoothAngle;

cvar_t	*r_softParticles;
cvar_t	*r_ignoreGlErrors;

cvar_t	*r_stereoVision;
cvar_t	*r_stereoSeparation;

int CL_PMpointcontents(vec3_t point);
qboolean outMap;

extern float ref_realtime;

extern int gl_lightmap_format;
extern int gl_solid_format;
extern int gl_alpha_format;
extern int gl_tex_solid_format;
extern int gl_tex_alpha_format;

extern float r_world_matrix[16];
extern float r_project_matrix[16];
extern qboolean inwaterfognode;
extern int r_visframecount;

extern int radarOldTime;
extern int r_viewport[4];
extern qboolean spacebox;

extern qboolean arbMultisampleSupported;

void GL_Bind(int texnum);
void GL_MBind(GLenum target, int texnum);
void GL_TexEnv(GLenum value);
void GL_EnableMultitexture(qboolean enable);
void GL_SelectTexture(GLenum);

void R_LightPoint(vec3_t p, vec3_t color, qboolean bump);
void R_PushDlights(void);


void GL_PicsColorScaleARB(qboolean enable);
void SetLevelOverbright(void);
void R_Register2(void);
void R_InitLightgrid(void);
void R_RenderFlares(void);

void R_DrawShadowVolume(entity_t * e);

void R_DrawParticles(qboolean WaterCheck);
void GL_DrawRadar(void);
void R_DrawAlphaPoly(void);
void R_RenderDecals(void);
void R_LightColor(vec3_t org, vec3_t color);
void MyGlPerspective(GLdouble fov, GLdouble aspectr, GLdouble zNear);
qboolean R_CullAliasModel(vec3_t bbox[8], entity_t *e);
void RenderLavaSurfaces(msurface_t * fa);
int CL_PMpointcontents2(vec3_t point, struct model_s * ignore);
void R_DrawAliasDistortModel (entity_t *e);
void VID_MenuInit( void );
void AnglesToMat3(const vec3_t angles, mat3_t m);
void Mat3_TransposeMultiplyVector (const mat3_t m, const vec3_t in, vec3_t out);
void R_LightPointDynamics (vec3_t p, vec3_t color, m_dlight_t *list, int *amount, int max);
void R_ShutdownPrograms(void);
void GL_BindNullProgram(void) ;
void GL_BindRect(int texnum);
void GL_MBindRect(GLenum target, int texnum);

//====================================================================

#define MAX_POLY_VERT		128
#define	MAX_BATCH_SURFS		21845

extern vec3_t	wVertexArray[MAX_BATCH_SURFS];

extern float	wTexArray[MAX_BATCH_SURFS][2];
extern float	wLMArray[MAX_BATCH_SURFS][2];


extern vec3_t	nTexArray[MAX_BATCH_SURFS];
extern vec3_t	tTexArray[MAX_BATCH_SURFS];
extern vec3_t	bTexArray[MAX_BATCH_SURFS];

extern float   wTmu0Array[MAX_BATCH_SURFS][2];
extern float   wTmu1Array[MAX_BATCH_SURFS][2];
extern float   wTmu2Array[MAX_BATCH_SURFS][2];
extern float   wTmu3Array[MAX_BATCH_SURFS][2];
extern float   wTmu4Array[MAX_BATCH_SURFS][2];
extern float   wTmu5Array[MAX_BATCH_SURFS][2];
extern float   wTmu6Array[MAX_BATCH_SURFS][2];

extern WORD	indexArray[MAX_BATCH_SURFS*3];
extern unsigned	numVertices, numIndeces;

extern model_t *r_worldmodel;

extern unsigned d_8to24table[256];
extern float	d_8to24tablef[256][3];

extern int registration_sequence;

extern qboolean is_sky;
extern qboolean inlava;
extern qboolean inslime;
extern qboolean inwater;
extern qboolean nodrawmodel;
extern unsigned char lightmap_gammatable[256];


void V_AddBlend(float r, float g, float b, float a, float *v_blend);

int R_Init(void *hinstance, void *hWnd);
void R_Shutdown(void);

void R_RenderView(refdef_t * fd);
void GL_ScreenShot_f(void);
void R_DrawAliasModel(entity_t * e, qboolean weapon_model);
void R_DrawBrushModel(entity_t * e);
void R_DrawSpriteModel(entity_t * e);
void R_DrawBeam();
void R_DrawBSP(void);
void R_RenderDlights(void);
void R_RenderBrushPoly(msurface_t * fa);
void R_InitEngineTextures(void);
void R_LoadFont(void);
void GL_SubdivideSurface(msurface_t * fa);
void GL_SubdivideLightmappedSurface(msurface_t * fa, float subdivide_size);	// Heffo 
																			// surface 
																			// subdivision
qboolean R_CullBox(vec3_t mins, vec3_t maxs);
void R_RotateForEntity(entity_t * e);
void R_MarkLeaves(void);
void R_EnableScreenMatrix(void);
void R_DisableScreenMatrix(void);
glpoly_t *WaterWarpPolyVerts(glpoly_t * p);
void EmitWaterPolys(msurface_t * fa);
void R_AddSkySurface(msurface_t * fa);
void R_ClearSkyBox(void);
void R_DrawSkyBox(void);
void R_MarkLights(dlight_t * light, int bit, mnode_t * node);

void COM_StripExtension(char *in, char *out);

void Draw_GetPicSize(int *w, int *h, char *name);
void Draw_Pic(int x, int y, char *name);
void Draw_StretchPic(int x, int y, int w, int h, char *name);
void Draw_TileClear(int x, int y, int w, int h, char *name);
void Draw_Fill(int x, int y, int w, int h, int c);
void Draw_FadeScreen(void);
void Draw_StretchRaw(int x, int y, int w, int h, int cols, int rows,
					 byte * data);

void R_BeginFrame();
void R_SwapBuffers(int);
void R_SetPalette(const unsigned char *palette);

int Draw_GetPalette(void);

void GL_ResampleTexture(unsigned *in, int inwidth, int inheight,
						unsigned *out, int outwidth, int outheight);

struct image_s *R_RegisterSkin(char *name);

void LoadPCX(char *filename, byte ** pic, byte ** palette, int *width,
			 int *height);

image_t *GL_LoadPic(char *name, byte * pic, int width, int height,
					imagetype_t type, int bits);

image_t *GL_FindImage(char *name, imagetype_t type);

image_t *R_AutoRelief(char *name, imagetype_t type);
void GL_TextureMode(char *string);
void GL_ImageList_f(void);

void GL_InitImages(void);
void GL_ShutdownImages(void);

void GL_FreeUnusedImages(void);

void GL_TextureAlphaMode(char *string);
void GL_TextureSolidMode(char *string);
qboolean R_CullOrigin(vec3_t origin);
void R_InitSun();
void R_RenderSun();
byte *R_HeightToNormal(byte *data, int width, int height);
/*
** GL extension emulation functions
*/
void GL_DrawParticles();
void R_TransformToScreen_Vec3(vec3_t in, vec3_t out);
void GL_Blend(qboolean on, int dst, int src);

void R_DesaturateImage(byte *data, int size, float amount);
void R_AutoLevelImage(byte *data, int size);
byte *R_HeightToNormal(byte *data, int width, int height);

void  VLight_Init (void);
float VLight_LerpLight ( int index1, int index2, float ilerp, vec3_t dir, vec3_t angles, qboolean dlight );

void GL_DrawAliasFrameLerpArbBump (dmdl_t *paliashdr);
void GL_SetupLightMatrix();
void GL_CleanLightMatrix();
void R_DrawShadowWorld(void);
qboolean SurfInFrustum(msurface_t *s);
extern vec3_t currentShadowLight;

int GL_MsgGLError(char* Info);
qboolean HasSharedLeafs(byte *v1, byte *v2);
float SphereInFrustum(vec3_t o, float radius);
void GL_DrawAliasFrameLerpArb(dmdl_t *paliashdr, vec3_t light, float rad, vec3_t lightColor);
/*
** GL config stuff
*/


typedef struct {
	int renderer;
	const char	*renderer_string;
	const char	*vendor_string;
	const char	*version_string;
	const char	*extensions_string;
	int		screenTextureSize;
	const char	*wglExtensionsString;

	const char	*shadingLanguageVersionString;
	int			maxVertexUniformComponents;		// GLSL info
	int			maxVaryingFloats;
	int			maxVertexTextureImageUnits;
	int			maxCombinedTextureImageUnits;
	int			maxFragmentUniformComponents;
	int			maxVertexAttribs;
	int			maxTextureImageUnits;

} glconfig_t;


#define GLSTATE_DISABLE_ALPHATEST	if (gl_state.alpha_test) { qglDisable(GL_ALPHA_TEST); gl_state.alpha_test=(qboolean)false; }
#define GLSTATE_ENABLE_ALPHATEST	if (!gl_state.alpha_test) { qglEnable(GL_ALPHA_TEST); gl_state.alpha_test=(qboolean)true; }

#define GLSTATE_DISABLE_BLEND		if (gl_state.blend) { qglDisable(GL_BLEND); gl_state.blend=(qboolean)false; }
#define GLSTATE_ENABLE_BLEND		if (!gl_state.blend) { qglEnable(GL_BLEND); gl_state.blend=(qboolean)true; }

#define GLSTATE_DISABLE_TEXGEN		if (gl_state.texgen) { qglDisable(GL_TEXTURE_GEN_S); qglDisable(GL_TEXTURE_GEN_T); qglDisable(GL_TEXTURE_GEN_R); qglDisable(GL_TEXTURE_GEN_Q); gl_state.texgen=(qboolean)false; }
#define GLSTATE_ENABLE_TEXGEN		if (!gl_state.texgen) { qglEnable(GL_TEXTURE_GEN_S); qglEnable(GL_TEXTURE_GEN_T); qglEnable(GL_TEXTURE_GEN_R); qglEnable(GL_TEXTURE_GEN_Q); gl_state.texgen=(qboolean)true; }

typedef struct {
	float inverse_intensity;
	qboolean fullscreen;

	int prev_mode;


	int lightmap_textures;
	int deluxemap_texnum;

	int currenttextures[4];
	int num_tmu;
	int currenttmu;

// advanced state manager - MrG
	qboolean alpha_test;
	qboolean blend;
	qboolean texgen;
	qboolean gammaramp;
// End - MrG

	qboolean DrawRangeElements;
	qboolean separateStencil;
	qboolean texture_compression_arb;
	int displayrefresh;
	qboolean nv_multisample_hint;
	qboolean arb_occlusion;
	qboolean arb_multisample;
	qboolean wgl_nv_multisample_coverage;
	qboolean wgl_nv_multisample_coverage_aviable;
	qboolean vbo;
	qboolean nv_conditional_render;
	qboolean glsl;
	qboolean nPot;
	unsigned vboId;
	int	programId;
	int		lastdFactor;
	int		lastsFactor;
	float	color[4];
	unsigned char originalRedGammaTable[256];
	unsigned char originalGreenGammaTable[256];
	unsigned char originalBlueGammaTable[256];

// ----------------------------------------------------------------


} glstate_t;

#ifndef BIT
#define BIT(num)				(1 << (num))
#endif

extern glconfig_t gl_config;
extern glstate_t gl_state;

extern int	g_numGlLights;

extern	vec3_t	lightspot;

#define VA_SetElem2(v,a,b)		((v)[0]=(a),(v)[1]=(b))
#define VA_SetElem3(v,a,b,c)	((v)[0]=(a),(v)[1]=(b),(v)[2]=(c))
#define VA_SetElem4(v,a,b,c,d)	((v)[0]=(a),(v)[1]=(b),(v)[2]=(c),(v)[3]=(d))

#define VA_SetElem2v(v,a)	((v)[0]=(a)[0],(v)[1]=(a)[1])
#define VA_SetElem3v(v,a)	((v)[0]=(a)[0],(v)[1]=(a)[1],(v)[2]=(a)[2])
#define VA_SetElem4v(v,a)	((v)[0]=(a)[0],(v)[1]=(a)[1],(v)[2]=(a)[2],(v)[3]=(a)[3])

#define MAX_VERTICES		16384
#define MAX_INDICES		MAX_VERTICES * 4
#define MAX_VERTEX_ARRAY	8192
#define MAX_SHADOW_VERTS	16384


extern vec3_t ShadowArray[MAX_SHADOW_VERTS];


#define Vector4Set(v, a, b, c, d)	((v)[0]=(a),(v)[1]=(b),(v)[2]=(c),(v)[3]=(d))
#define Vector4Copy(a,b) ((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])
#define PlaneDiff(point,plane) (((plane)->type < 3 ? (point)[(plane)->type] : DotProduct((point), (plane)->normal)) - (plane)->dist)

typedef byte color4ub_t[4];
#define clamp(a,b,c)	((a) < (b) ? (b) : (a) > (c) ? (c) : (a))

void Q_strncatz (char *dst, int dstSize, const char *src);


#define	MAXSHADOWLIGHTS		1024	//256 //Maximum number of (client side) lights in a map				/// FIXME: уменьшить для спец.карт для данного движка!
#define MAXUSEDSHADOWLIGHS	128		//64  //Maximum number of lights that can be used in a single frame	/// FIXME: уменьшить для спец.карт для данного движка!


#define LIGHTMAP_BYTES 4
#define	LIGHTMAP_SIZE	1024 //was 128
#define	MAX_LIGHTMAPS	8 //was 128

#define MAX_GL_DELUXEMAPS	256
#define TEXNUM_DELUXEMAPS	(TEXNUM_LIGHTMAPS + MAX_LIGHTMAPS)

typedef struct {
	int internal_format;
	int current_lightmap_texture;

	msurface_t *lightmap_surfaces[MAX_LIGHTMAPS];

	int allocated[LIGHTMAP_SIZE];
	
	// the lightmap texture data needs to be kept in
	// main memory so texsubimage can update properly
	
	byte lightmap_buffer[LIGHTMAP_BYTES * LIGHTMAP_SIZE * LIGHTMAP_SIZE];
	byte *direction_buffer;

} gllightmapstate_t;

gllightmapstate_t gl_lms;


// set GL_BGRA for fastest lightmaps upload
#define GL_LIGHTMAP_FORMAT GL_BGRA

/*
====================================================================

  PROGRAMS

====================================================================
*/
#define	MAX_PROGRAM_UNIFORMS	32
#define	MAX_PROGRAM_DEFS	8					// max permutation defs program can have
#define	MAX_PROGRAM_ID		(1 << MAX_PROGRAM_DEFS)		// max GL indices per program object

#define	MAX_UNIFORM_NAME	64
#define	MAX_DEF_NAME		32
#define	MAX_PROGRAMS		256

typedef struct {
	char			name[MAX_UNIFORM_NAME];
} glslUniform_t;

typedef struct glslProgram_s {
	
	struct glslProgram_s	*nextHash;

	char			name[MAX_QPATH];
	int				numDefs;
	unsigned		defBits[MAX_PROGRAM_DEFS];
	char			defStrings[MAX_PROGRAM_DEFS][MAX_DEF_NAME];
	int				numId;
	int				id[MAX_PROGRAM_ID];
	qboolean		valid;		// true if all permutations linked successfully

} glslProgram_t;

glslProgram_t r_programs[MAX_PROGRAMS];

glslProgram_t		*diffuseProgram;
glslProgram_t		*aliasAmbientProgram;
glslProgram_t		*aliasBumpProgram;
glslProgram_t		*bloomdsProgram;
glslProgram_t		*gaussXProgram;
glslProgram_t		*gaussYProgram;
glslProgram_t		*bloomfpProgram;
glslProgram_t		*refractProgram;
glslProgram_t		*thermalProgram;
glslProgram_t		*thermalfpProgram;
glslProgram_t		*waterProgram;
glslProgram_t		*radialProgram;
glslProgram_t		*dofProgram;
glslProgram_t		*particlesProgram;
glslProgram_t		*shadowProgram;
glslProgram_t		*genericProgram;
glslProgram_t		*cinProgram;
glslProgram_t		*loadingProgram;

void GL_BindProgram(glslProgram_t *program, int defBits);
void R_CaptureDepthBuffer();
void R_CaptureColorBuffer();

typedef struct {
	unsigned	CausticsBit;
	unsigned	ParallaxBit;
	unsigned	OverBrightBit;
	unsigned	LightmapBits;
	unsigned	VertexLightBits;
	unsigned	AlphaMaskBits;
	unsigned	BumpBits;
	unsigned	WaterTransBits;
	unsigned	ShellBits;
} 
worldDefs_t;

worldDefs_t worldDefs;

typedef enum glsl_attribute
{
	ATRB_POSITION = 0,
	ATRB_COLOR = 1,
	ATRB_TEX0 = 2,
	ATRB_TEX1 = 3,
	ATRB_TEX2 = 4,
	ATRB_TEX3 = 5,
	ATRB_TEX4 = 6,
	ATRB_TEX5 = 7,
	ATRB_TEX6 = 8,
	ATRB_TEX7 = 9,
	ATRB_NORMAL = 10,
	ATRB_TANGENT = 11,
	ATRB_BINORMAL = 12,


}
glsl_attrib;

#define	SRGB_EXT                                       0x8C40
#define	SRGB8_EXT                                      0x8C41
#define	SRGB_ALPHA_EXT                                 0x8C42
#define	SRGB8_ALPHA8_EXT                               0x8C43
#define	SLUMINANCE_ALPHA_EXT                           0x8C44
#define	SLUMINANCE8_ALPHA8_EXT                         0x8C45
#define	SLUMINANCE_EXT                                 0x8C46
#define	SLUMINANCE8_EXT                                0x8C47
#define	COMPRESSED_SRGB_EXT                            0x8C48
#define	COMPRESSED_SRGB_ALPHA_EXT                      0x8C49
#define	COMPRESSED_SLUMINANCE_EXT                      0x8C4A
#define	COMPRESSED_SLUMINANCE_ALPHA_EXT                0x8C4B

#define MAX_VBO_XYZs		65536

vec3_t	vbo_XyzBuffer[MAX_VBO_XYZs];
vec3_t	vbo_StBuffer[MAX_VBO_XYZs];
vec3_t	vbo_LmBuffer[MAX_VBO_XYZs];
vec3_t	vbo_NormalBuffer[MAX_VBO_XYZs];
vec3_t	vbo_BiNormalBuffer[MAX_VBO_XYZs];
vec3_t	vbo_TangentBuffer[MAX_VBO_XYZs];

typedef struct {
	unsigned		indexBuffer;
	unsigned		vertexBuffer;
} vbo_t;

vbo_t vbo_s;

#define	MAX_VERTEX_CACHES	4096

/*
====================================================================

IMPLEMENTATION SPECIFIC FUNCTIONS

====================================================================
*/

void GLimp_EndFrame(void);
int GLimp_Init(void *hinstance, void *hWnd);
void GLimp_Shutdown(void);
rserr_t GLimp_SetMode(unsigned *pwidth, unsigned *pheight, int mode,
				  qboolean fullscreen);
void GLimp_AppActivate(qboolean active);
void GLimp_EnableLogging(qboolean enable);
void GLimp_LogNewFrame(void);

#ifndef _WIN32
#  error You should not be including this file on this platform
#endif

#ifndef __GLW_WIN_H__
#define __GLW_WIN_H__

typedef struct
{
	HINSTANCE	hInstance;
	void	*wndproc;

	HDC     hDC;			// handle to device context
	HWND    hWnd;			// handle to window
	HGLRC   hGLRC;			// handle to GL rendering context

	HINSTANCE hinstOpenGL;	// HINSTANCE for the OpenGL library

	qboolean minidriver;
	qboolean allowdisplaydepthchange;
	qboolean mcd_accelerated;

	const char	*wglExtsString;
	const char	*wglRenderer;
	int desktopWidth, desktopHeight;

} glwstate_t;

extern glwstate_t glw_state;

#endif


#endif							/* R_LOCAL_H */
