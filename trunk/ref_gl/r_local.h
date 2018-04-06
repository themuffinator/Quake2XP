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

#ifdef _WIN32
#include "glcorearb.h"
#include <math.h>
#include "wglext.h"

#include "imagelib/il.h"
#include "imagelib/ilu.h"
#include "imagelib/ilut.h"

#else
#include "glcorearb.h"
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

typedef void ILvoid;
#define _inline inline
#endif

#include "../client/ref.h"
#include "r_md3.h"
#include "qgl.h"

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



#define	TEXNUM_SCRAPS		4096
#define	TEXNUM_IMAGES		4097

#define	TEXNUM_LIGHTMAPS	16384
#define	MAX_GLTEXTURES		16384 

#define BUFFER_OFFSET(i) ((byte *)NULL + (i))

#define MAX_DRAW_STRING_LENGTH  256
#define MAX_IDX 65536
// ===================================================================

typedef enum {
	rserr_ok,

	rserr_invalid_fullscreen,
	rserr_invalid_mode,

	rserr_unknown
} rserr_t;

#include "r_model.h"

void GL_SetDefaultState (void);
void GL_UpdateSwapInterval (void);

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

#define		MAX_WATER_NORMALS		32
extern image_t *r_waterNormals[MAX_WATER_NORMALS];

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

extern qboolean drawFlares;
extern image_t gltextures[MAX_GLTEXTURES];
extern int numgltextures;
extern image_t *r_notexture;
extern image_t *r_distort;
extern image_t *r_predator;
extern image_t *depthMap;

extern image_t *r_particletexture[PT_MAX];
extern image_t *r_decaltexture[DECAL_MAX];

extern	image_t *r_radarmap;
extern	image_t *r_around;
extern	image_t *r_flare;

extern	image_t *draw_chars;
extern	image_t *r_DSTTex;

extern	image_t	*r_defBump;
extern	image_t	*ScreenMap;
extern	image_t	*r_envTex;
extern	image_t	*r_randomNormalTex;
extern	image_t	*shadowMask;
extern	image_t	*r_conBump;
extern	image_t	*weaponHack;
extern	image_t *fxaaMap;
extern	image_t *fboScreen;
extern	image_t	*r_whiteMap;
extern	image_t *skinBump;

#define MAX_FILTERS 256
extern	image_t	*r_lightCubeMap[MAX_FILTERS];
#define	MAX_GLOBAL_FILTERS	37

extern image_t *fboDN;
extern image_t *fboColor[2];
extern image_t *ScreenCapture;

extern uint bloomtex;
extern uint thermaltex;
extern uint fxaatex;
extern uint fovCorrTex;
extern unsigned int	skyCube;

extern uint fboId;
extern byte fboColorIndex;

extern entity_t *currententity;
extern model_t *currentmodel;
extern int r_visframecount;
extern int r_framecount;
extern cplane_t frustum[6];

extern	int gl_filter_min, gl_filter_max;
extern	int flareQueries[MAX_WORLD_SHADOW_LIHGTS];

//
// view origin
//
extern vec3_t vup;
extern vec3_t vpn;
extern vec3_t vright;
extern vec3_t r_origin;
extern entity_t r_worldentity;

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
cvar_t *r_noBind;
cvar_t *r_cull;
cvar_t *r_vsync;
cvar_t *r_textureMode;

cvar_t *r_imageAutoBump;
cvar_t *r_imageAutoBumpScale;
cvar_t *r_imageAutoSpecularScale;

cvar_t *r_lockPvs;
cvar_t *r_fullScreen;

cvar_t *r_brightness;
cvar_t *r_contrast;
cvar_t *r_saturation;
cvar_t *r_gamma;

cvar_t *vid_ref;

cvar_t	*r_causticIntens;

cvar_t	*r_displayRefresh;

cvar_t	*r_screenShot;
cvar_t	*r_screenShotJpegQuality;
cvar_t	*r_screenShotGamma;
cvar_t	*r_screenShotContrast;

cvar_t	*r_textureColorScale;
cvar_t	*r_textureCompression;
cvar_t	*r_anisotropic;
cvar_t	*r_maxAnisotropy;
cvar_t	*r_textureLodBias;

cvar_t	*r_shadows;
cvar_t	*r_playerShadow;

cvar_t	*r_multiSamples;
cvar_t	*r_fxaa;
cvar_t	*deathmatch;

cvar_t	*r_drawFlares;
cvar_t	*r_scaleAutoLightColor;
cvar_t	*r_lightWeldThreshold;

cvar_t	*r_customWidth;
cvar_t	*r_customHeight;

cvar_t	*r_bloom;
cvar_t	*r_bloomThreshold;
cvar_t	*r_bloomIntens;
cvar_t	*r_bloomWidth;

cvar_t	*r_ssao;
cvar_t	*r_ssaoIntensity;
cvar_t	*r_ssaoScale;
cvar_t	*r_ssaoBlur;

cvar_t	*r_skipStaticLights;
cvar_t	*r_lightmapScale;
cvar_t	*r_lightsWeldThreshold;
cvar_t	*r_debugLights;
cvar_t	*r_useLightScissors;
cvar_t	*r_useDepthBounds;
cvar_t	*r_specularScale;
cvar_t	*r_ambientSpecularScale;
cvar_t	*r_useRadiosityBump;
cvar_t	*r_zNear;
cvar_t	*r_zFar;

cvar_t	*hunk_bsp;
cvar_t	*hunk_md2;
cvar_t	*hunk_md3;
cvar_t	*hunk_sprite;

cvar_t	*r_maxTextureSize;

cvar_t	*r_reliefMapping;
cvar_t	*r_reliefScale;

cvar_t	*r_dof;
cvar_t	*r_dofBias;
cvar_t	*r_dofFocus;

cvar_t	*r_motionBlur;
cvar_t	*r_motionBlurSamples;
cvar_t	*r_motionBlurFrameLerp;

cvar_t	*r_radialBlur;
cvar_t	*r_radialBlurFov;

cvar_t	*r_tbnSmoothAngle;

cvar_t	*r_glDebugOutput;
cvar_t	*r_glMinorVersion;
cvar_t	*r_glMajorVersion;
cvar_t	*r_glCoreProfile;

cvar_t	*r_lightEditor;
cvar_t	*r_cameraSpaceLightMove;

cvar_t	*r_hudLighting;
cvar_t	*r_bump2D;

cvar_t	*r_filmFilter;
cvar_t	*r_filmFilterType; // 0 - technicolor; 1 - sepia
cvar_t	*r_filmFilterNoiseIntens;
cvar_t	*r_filmFilterScratchIntens;
cvar_t	*r_filmFilterVignetIntens;

cvar_t	*r_fixFovStrength; // 0.0 = no hi-fov perspective correction
cvar_t	*r_fixFovDistroctionRatio; // 0.0 = cylindrical distortion ratio. 1.0 = spherical

int CL_PMpointcontents (vec3_t point);
qboolean outMap;

extern float ref_realtime;

extern int r_visframecount;

qboolean xhargar2hack;

void GL_Bind (int texnum);
void GL_MBind (GLenum target, int texnum);
void GL_SelectTexture (GLenum);
void GL_MBindCube (GLenum target, int texnum);

void R_LightPoint (vec3_t p, vec3_t color);

void R_InitLightgrid (void);

void R_GenEnvCubeMap();

worldShadowLight_t *R_AddNewWorldLight (vec3_t origin, vec3_t color, float radius[3], int style, int filter, vec3_t angles, vec3_t speed,
	qboolean isStatic, int isShadow, int isAmbient, float cone, qboolean ingame, int flare, vec3_t flareOrg,
	float flareSize, char target[MAX_QPATH], int start_off, int fog, float fogDensity);
void R_DrawParticles ();
void R_RenderDecals (void);
void R_LightColor (vec3_t org, vec3_t color);
qboolean R_CullAliasModel (vec3_t bbox[8], entity_t *e);
int CL_PMpointcontents2 (vec3_t point, struct model_s * ignore);
void VID_MenuInit (void);
void AnglesToMat3 (const vec3_t angles, mat3_t m);
void Mat3_TransposeMultiplyVector (const mat3_t m, const vec3_t in, vec3_t out);
void R_ShutdownPrograms (void);
void GL_BindRect (int texnum);
void GL_MBindRect (GLenum target, int texnum);
void R_Bloom (void);
void R_ThermalVision (void);
void R_RadialBlur (void);
void R_DofBlur (void);
void R_FXAA (void);
void R_FilmFilter (void);
void R_FixFov(void);
void R_ListPrograms_f (void);
void R_InitPrograms (void);
void R_ClearWorldLights (void);
qboolean R_CullSphere (const vec3_t centre, const float radius);
void R_CastBspShadowVolumes (void);
void R_CastAliasShadowVolumes (qboolean player);
void R_DrawAliasModelLightPass (qboolean weapon_model);
void R_SetupEntityMatrix (entity_t * e);
void GL_MBind3d (GLenum target, int texnum);
void R_SSAO(void);
void R_DrawDepthScene(void);
void R_DownsampleDepth(void);
void R_ScreenBlend(void);

void R_SaveLights_f (void);
void R_Light_Spawn_f (void);
void R_Light_Delete_f (void);
void R_EditSelectedLight_f (void);
void R_MoveLightToRight_f (void);
void R_MoveLightForward_f (void);
void R_MoveLightUpDown_f (void);
void R_Light_SpawnToCamera_f (void);
void R_ChangeLightRadius_f (void);
void R_Light_Clone_f (void);
void R_ChangeLightCone_f (void);
void R_Light_UnSelect_f (void);
void R_FlareEdit_f (void);
void R_ResetFlarePos_f (void);
void R_Copy_Light_Properties_f (void);
void R_Paste_Light_Properties_f (void);

extern qboolean flareEdit;

void R_CalcCubeMapMatrix (qboolean model);
void DeleteShadowVertexBuffers (void);
void MakeFrustum4Light (worldShadowLight_t *light, qboolean ingame);
qboolean R_CullConeLight (vec3_t mins, vec3_t maxs, cplane_t *frust);
void GL_DrawAliasFrameLerpLight (dmdl_t *paliashdr);
qboolean SurfInFrustum (msurface_t *s);
qboolean HasSharedLeafs (byte *v1, byte *v2);
qboolean InLightVISEntity ();
void R_DrawLightBrushModel ();
void UpdateLightEditor (void);
void Load_LightFile ();
qboolean BoundsIntersectsPoint (vec3_t mins, vec3_t maxs, vec3_t p);
extern int lightsQueries[MAX_WORLD_SHADOW_LIHGTS];
extern int numLightQ;
extern int numFlareOcc;
extern qboolean FoundReLight;
qboolean PF_inPVS (vec3_t p1, vec3_t p2);
void R_SetFrustum (void);
void R_SetViewLightScreenBounds ();
qboolean BoundsIntersect (const vec3_t mins1, const vec3_t maxs1, const vec3_t mins2, const vec3_t maxs2);
void R_DrawLightFlare ();
void R_DrawLightBounds(void);

void R_ShutDownVertexBuffers();

extern const mat3_t	mat3_identity;
extern const mat4_t	mat4_identity;

void Mat3_Identity (mat3_t m);
void Mat3_Copy (const mat3_t in, mat3_t out);

void Mat4_Multiply (const mat4_t a, const mat4_t b, mat4_t out);
void Mat4_Copy (const mat4_t in, mat4_t out);
void Mat4_Transpose (const mat4_t in, mat4_t out);
void Mat4_MultiplyVector (const mat4_t m, const vec3_t in, vec3_t out);
void Mat4_Translate (mat4_t m, float x, float y, float z);
void Mat4_Scale (mat4_t m, float x, float y, float z);
qboolean Mat4_Invert (const mat4_t in, mat4_t out);
void Mat4_TransposeMultiply (const mat4_t a, const mat4_t b, mat4_t out);
void Mat4_SetOrientation (mat4_t m, const mat3_t rotation, const vec3_t translation);
void Mat4_Identity (mat4_t mat);
void Mat4_Rotate (mat4_t m, float angle, float x, float y, float z);
void Mat4_AffineInvert(const mat4_t in, mat4_t out);
void Mat4_SetupTransform(mat4_t m, const mat3_t rotation, const vec3_t translation);
void Mat3_Set(mat3_t mat, vec3_t x, vec3_t y, vec3_t z);
void Mat4_Set(mat4_t mat, vec4_t x, vec4_t y, vec4_t z, vec4_t w);
void VectorLerp(const vec3_t from, const vec3_t to, float frac, vec3_t out);

void SetPlaneType (cplane_t *plane);
void SetPlaneSignBits (cplane_t *plane);

trace_t CL_PMTraceWorld (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int mask, qboolean checkAliases);
void AddBoundsToBounds(const vec3_t mins1, const vec3_t maxs1, vec3_t mins2, vec3_t maxs2);

void R_DrawChainsRA(qboolean bmodel);
void R_DrawBrushModelRA(void);

void R_DrawMD3Mesh(qboolean weapon);
void R_DrawMD3MeshLight(qboolean weapon);
void R_DrawMD3ShellMesh(qboolean weapon);
void CheckEntityFrameMD3(md3Model_t *paliashdr);
qboolean R_CullMD3Model(vec3_t bbox[8], entity_t *e);

qboolean R_AliasInLightBound();
void R_UpdateLightAliasUniforms();

void R_InitVertexBuffers();

void SetModelsLight();
extern float shadelight[3];
byte Normal2Index(const vec3_t vec);
extern int	occ_framecount;

//====================================================================

#define MAX_POLY_VERT		128
#define	MAX_BATCH_SURFS		21845

extern vec3_t	wVertexArray[MAX_BATCH_SURFS];

extern float	wTexArray[MAX_BATCH_SURFS][2];
extern float	wLMArray[MAX_BATCH_SURFS][2];
extern vec4_t   wColorArray[MAX_BATCH_SURFS];


extern vec3_t	nTexArray[MAX_BATCH_SURFS];
extern vec3_t	tTexArray[MAX_BATCH_SURFS];
extern vec3_t	bTexArray[MAX_BATCH_SURFS];

extern float   wTmu0Array[MAX_BATCH_SURFS][2];
extern float   wTmu1Array[MAX_BATCH_SURFS][2];
extern float   wTmu2Array[MAX_BATCH_SURFS][2];

extern uint		indexArray[MAX_MAP_VERTS * 3];

extern model_t *r_worldmodel;

extern unsigned d_8to24table[256];
extern float	d_8to24tablef[256][3];

extern int registration_sequence;

extern float skyrotate;
extern vec3_t skyaxis;

int R_Init (void *hinstance, void *hWnd);
void R_Shutdown (void);

void R_RenderView (refdef_t * fd);
void GL_ScreenShot_f (void);
void R_DrawAliasModel (entity_t * e);
void R_DrawBrushModel ();
void R_DrawSpriteModel (entity_t * e);
void R_DrawBeam ();
void R_DrawBSP (void);
void R_InitEngineTextures (void);
void R_LoadFont (void);

qboolean R_CullBox (vec3_t mins, vec3_t maxs);
void R_MarkLeaves (void);
void R_DrawWaterPolygons (msurface_t * fa, qboolean bmodel);
void R_AddSkySurface (msurface_t * fa);
void R_ClearSkyBox (void);
void R_DrawSkyBox (qboolean color);

void COM_StripExtension (char *in, char *out);

void Draw_GetPicSize (int *w, int *h, char *name);
void Draw_Pic (int x, int y, char *name);
void Draw_StretchPic (int x, int y, int w, int h, char *name);
void Draw_TileClear (int x, int y, int w, int h, char *name);
void Draw_Fill (int x, int y, int w, int h, float r, float g, float b, float a);
void Draw_StretchRaw (int x, int y, int w, int h, int cols, int rows,
	byte * data);

void R_BeginFrame ();
void R_SetPalette (const unsigned char *palette);

int Draw_GetPalette (void);

struct image_s *R_RegisterSkin (char *name);

image_t *GL_LoadPic (char *name, byte * pic, int width, int height,
	imagetype_t type, int bits);

image_t *GL_FindImage (char *name, imagetype_t type);

void GL_TextureMode (char *string);
void GL_ImageList_f (void);

void GL_InitImages (void);
void GL_ShutdownImages (void);

void GL_FreeUnusedImages (void);
qboolean R_CullOrigin (vec3_t origin);
qboolean IsExtensionSupported(const char *name);

/*
** GL extension emulation functions
*/

void	CreateSSAOBuffer();
void CreateFboBuffer (void);

/*
** GL config stuff
*/


typedef struct {
	int renderer;
	const char	*renderer_string;
	const char	*vendor_string;
	const char	*version_string;
	const char	*extensions3_string;

	int			screenTextureSize;
	const char	*wglExtensionsString;

	const char	*shadingLanguageVersionString;
	int			maxVertexUniformComponents;		// GLSL info
	int			maxVaryingFloats;
	int			maxVertexTextureImageUnits;
	int			maxCombinedTextureImageUnits;
	int			maxFragmentUniformComponents;
	int			maxVertexAttribs;
	int			maxTextureImageUnits;

	int			glMajorVersion;
	int			glMinorVersion;

	int			colorBits;
	int			alphaBits;
	int			depthBits;
	int			stencilBits;
	int			samples;
	int			maxSamples;
} glconfig_t;


typedef struct {
	qboolean fullscreen;

	int prev_mode;

	int lightmap_textures;
	int currenttextures[32]; // max gl_texturesXX
	int currenttmu;

	qboolean	texture_compression_bptc;
	int			displayrefresh;
	int			monitorWidth, monitorHeight;

	qboolean	wgl_no_error;
	qboolean	wgl_swap_control_tear;
	qboolean	depthBoundsTest;

	int			programId;
	GLenum		matrixMode;
	
	int			vaoBuffer, vboBuffer;

	mat4_t		projectionMatrix;
	mat4_t		modelViewMatrix;		// ready to load

	// frame buffer
	int			maxRenderBufferSize;
	int			maxColorAttachments;
	int			maxSamples;
	int			maxDrawBuffers;

	// gl state cache
	qboolean		cullFace;
	GLenum			cullMode;
	GLenum			frontFace;

	qboolean		blend;
	GLenum			blendSrc;
	GLenum			blendDst;

	GLboolean		colorMask[4];

	qboolean		depthTest;
	GLenum			depthFunc;
	GLboolean		depthMask;
	GLclampd		depthRange[2];

	qboolean		polygonOffsetFill;
	GLfloat			polygonOffsetFactor;
	GLfloat			polygonOffsetUnits;

	qboolean		lineSmooth;

	qboolean		depthClamp;

	qboolean		stencilTest;
	GLenum			stencilFunc;
	GLenum			stencilFace;
	GLuint			stencilMask;
	GLint			stencilRef;
	GLuint			stencilRefMask;
	GLenum			stencilFail;
	GLenum			stencilZFail;
	GLenum			stencilZPass;

	qboolean		scissorTest;
	GLint			scissor[4];

	qboolean		glDepthBoundsTest;
	GLfloat			depthBoundsMins;
	GLfloat			depthBoundsMax;

	vec4_t			fontColor;
} glstate_t;

typedef struct {

GLuint	vbo_fullScreenQuad;
GLuint	vbo_halfScreenQuad;
GLuint	vbo_quarterScreenQuad;
GLuint	ibo_quadTris;
GLuint	vbo_Dynamic;
GLuint	ibo_Dynamic;
GLuint	vbo_BSP;
GLuint	ibo_BSP;

int xyz_offset;
int st_offset;
int lm_offset;
int nm_offset;
int tg_offset;
int bn_offset;
}vbo_t;

vbo_t vbo;

typedef struct {
	GLuint	bsp_a; 
	GLuint	bsp_l;
	GLuint	alias_shadow;
	GLuint	fonts;
	GLuint	fullscreenQuad;
	GLuint	halfScreenQuad;
	GLuint	quaterScreenQuad;
}vao_t;

vao_t vao;

void GL_CullFace (GLenum mode);
void GL_FrontFace (GLenum mode);

void GL_DepthFunc (GLenum func);
void GL_DepthMask (GLboolean flag);
void GL_BlendFunc (GLenum src, GLenum dst);
void GL_ColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);

void GL_StencilMask (GLuint mask);
void GL_StencilFunc (GLenum func, GLint ref, GLuint mask);
void GL_StencilOp (GLenum fail, GLenum zFail, GLenum zPass);
void GL_StencilFuncSeparate (GLenum face, GLenum func, GLint ref, GLuint mask);
void GL_StencilOpSeparate (GLenum face, GLenum fail, GLenum zFail, GLenum zPass);

void GL_Scissor (GLint x, GLint y, GLint width, GLint height);
void GL_DepthRange (GLclampd n, GLclampd f);
void GL_PolygonOffset (GLfloat factor, GLfloat units);
void GL_DepthBoundsTest (GLfloat mins, GLfloat maxs);

void GL_Enable (GLenum cap);
void GL_Disable (GLenum cap);

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
#define MAX_INDICES			65536
#define MAX_VERTEX_ARRAY	8192
#define MAX_SHADOW_VERTS	16384

void R_PrepareShadowLightFrame (qboolean weapon);
extern worldShadowLight_t *shadowLight_static, *shadowLight_frame;
qboolean BoundsAndSphereIntersect (const vec3_t mins, const vec3_t maxs, const vec3_t origin, float radius);

#define Vector4Set(v, a, b, c, d)	((v)[0]=(a),(v)[1]=(b),(v)[2]=(c),(v)[3]=(d))
#define Vector4Copy(a,b) ((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])
#define PlaneDiff(point,plane) (((plane)->type < 3 ? (point)[(plane)->type] : DotProduct((point), (plane)->normal)) - (plane)->dist)

#define Vector4Scale(in,scale,out)		((out)[0]=(in)[0]*scale,(out)[1]=(in)[1]*scale,(out)[2]=(in)[2]*scale,(out)[3]=(in)[3]*scale)
#define Vector4Add(a,b,c)		((c)[0]=(((a[0])+(b[0]))),(c)[1]=(((a[1])+(b[1]))),(c)[2]=(((a[2])+(b[2]))),(c)[3]=(((a[3])+(b[3]))))
#define Vector4Sub(a,b,c)		((c)[0]=(((a[0])-(b[0]))),(c)[1]=(((a[1])-(b[1]))),(c)[2]=(((a[2])-(b[2]))),(c)[3]=(((a[3])-(b[3]))))


#define clamp(a,b,c)	((a) < (b) ? (b) : (a) > (c) ? (c) : (a))

void Q_strncatz (char *dst, int dstSize, const char *src);

#define	MAX_LIGHTMAPS		4		// max number of atlases
#define	LIGHTMAP_SIZE		2048
#define GL_LIGHTMAP_FORMAT	GL_RGB
#define XPLM_NUMVECS		3

typedef struct {
	int internal_format;
	int current_lightmap_texture;

	msurface_t *lightmap_surfaces[MAX_LIGHTMAPS];

	int allocated[LIGHTMAP_SIZE];

	// the lightmap texture data needs to be kept in
	// main memory so texsubimage can update properly
	byte lightmap_buffer[3][LIGHTMAP_SIZE * LIGHTMAP_SIZE * 3];
} gllightmapstate_t;

gllightmapstate_t gl_lms;
extern const vec3_t r_xplmBasisVecs[XPLM_NUMVECS];

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
//	char			defStrings[MAX_PROGRAM_DEFS][MAX_DEF_NAME];
	int				numId;
//	uint			id[MAX_PROGRAM_ID];
	uint			id;
	qboolean		valid;		// qtrue if all permutations linked successfully

} glslProgram_t;

glslProgram_t r_programs[MAX_PROGRAMS];

glslProgram_t		*ambientWorldProgram;
glslProgram_t		*lightWorldProgram;
glslProgram_t		*aliasAmbientProgram;
glslProgram_t		*md3AmbientProgram;
glslProgram_t		*aliasBumpProgram;
glslProgram_t		*gaussXProgram;
glslProgram_t		*gaussYProgram;
glslProgram_t		*glareProgram;
glslProgram_t		*bloomdsProgram;
glslProgram_t		*bloomfpProgram;
glslProgram_t		*motionBlurProgram;
glslProgram_t		*ssaoProgram;
glslProgram_t		*depthDownsampleProgram;
glslProgram_t		*ssaoBlurProgram;
glslProgram_t		*refractProgram;
glslProgram_t		*lightGlassProgram;
glslProgram_t		*thermalProgram;
glslProgram_t		*thermalfpProgram;
glslProgram_t		*waterProgram;
glslProgram_t		*lavaProgram;
glslProgram_t		*radialProgram;
glslProgram_t		*dofProgram;
glslProgram_t		*particlesProgram;
glslProgram_t		*shadowProgram;
glslProgram_t		*ssProgram;
glslProgram_t		*genericProgram;
glslProgram_t		*cinProgram;
glslProgram_t		*loadingProgram;
glslProgram_t		*fxaaProgram;
glslProgram_t		*filmGrainProgram;
glslProgram_t		*nullProgram;
glslProgram_t		*gammaProgram;
glslProgram_t		*FboProgram;
glslProgram_t		*light2dProgram;
glslProgram_t		*fixFovProgram;
glslProgram_t		*menuProgram;
glslProgram_t		*fbo2screenProgram;

void GL_BindProgram (glslProgram_t *program);
void R_CaptureDepthBuffer ();
void R_CaptureColorBuffer ();
void R_DrawLightWorld ();
void R_SetupOrthoMatrix(void);

typedef struct {
	unsigned	CausticsBit;
	unsigned	ParallaxBit;
	unsigned	LightParallaxBit;
	unsigned	AmbientBits;
	unsigned	AmbientAliasBits;
	unsigned	LightmapBits;
	unsigned	AlphaMaskBits;
	unsigned	WaterTransBits;
	unsigned	ShellBits;
	unsigned	EnvBits;
	unsigned	AttribColorBits;
	unsigned	ConsoleBits;
}
worldDefs_t;

worldDefs_t worldDefs;

typedef enum {
	ATT_POSITION = 0,
	ATT_NORMAL = 1,
	ATT_TANGENT = 2,
	ATT_BINORMAL = 3,
	ATT_COLOR = 4,
	ATT_TEX0 = 5,
	ATT_TEX1 = 6,
	ATT_TEX2 = 7,
}
glsl_attrib;

typedef enum {
	U_MODELVIEW = 0,
	U_PROJ = 1,
	U_MODEL_VIEW_PROJ = 2,
	U_ORTO_PROJ = 3,
	U_VIEW_ORG = 4,
	U_LIGHT_ORG = 5,
	U_DEPTH_PARAMS = 6,
	U_SCREEN_SIZE = 7,
	U_COLOR = 8,
	U_COLOR_SCALE = 9,
}
glsl_uniform;

uint null_mvp;
uint fbo2screen_orthoMatrix;

uint ambientWorld_lightmapType;
uint ambientWorld_ssao;
uint ambientWorld_parallaxParams;
uint ambientWorld_colorScale;
uint ambientWorld_specularScale;
uint ambientWorld_viewOrigin;
uint ambientWorld_parallaxType;
uint ambientWorld_ambientLevel;
uint ambientWorld_scroll;
uint ambientWorld_mvp;
uint ambientWorld_lava;

uint lightWorld_parallaxParams;
uint lightWorld_colorScale;
uint lightWorld_viewOrigin;
uint lightWorld_parallaxType;
uint lightWorld_lightOrigin;
uint lightWorld_lightColor;
uint lightWorld_fog;
uint lightWorld_fogDensity;
uint lightWorld_causticsIntens;
uint lightWorld_caustics;
uint lightWorld_specularScale;
uint lightWorld_roughnessScale;
uint lightWorld_ambient;
uint lightWorld_attenMatrix;
uint lightWorld_cubeMatrix;
uint lightWorld_scroll;
uint lightWorld_mvp;
uint lightWorld_isRgh;
uint lightWorld_spotLight;
uint lightWorld_spotParams;
uint lightWorld_spotMatrix;
uint lightWorld_autoBump;
uint lightWorld_autoBumpParams;

uint ambientAlias_ssao;
uint ambientAlias_colorModulate;
uint ambientAlias_addShift;
uint ambientAlias_isEnvMaping;
uint ambientAlias_envScale;
uint ambientAlias_isShell;
uint ambientAlias_shellParams;
uint ambientAlias_mvp;
uint ambientAlias_viewOrg;

uint ambientMd3_texRotation;
uint ambientMd3_colorModulate;
uint ambientMd3_addShift;
uint ambientMd3_isEnvMaping;
uint ambientMd3_isTransluscent;
uint ambientMd3_envScale;
uint ambientMd3_isShell;
uint ambientMd3_shellParams;
uint ambientMd3_mvp;
uint ambientMd3_viewOrg;
uint ambientMd3_texRotation;

uint lightAlias_colorScale;
uint lightAlias_viewOrigin;
uint lightAlias_lightOrigin;
uint lightAlias_lightColor;
uint lightAlias_fog;
uint lightAlias_fogDensity;
uint lightAlias_causticsIntens;
uint lightAlias_isCaustics;
uint lightAlias_isRgh;
uint lightAlias_specularScale;
uint lightAlias_ambient;
uint lightAlias_attenMatrix;
uint lightAlias_cubeMatrix;
uint lightAlias_mvp;
uint lightAlias_mv;
uint lightAlias_spotLight;
uint lightAlias_spotParams;
uint lightAlias_spotMatrix;
uint lightAlias_autoBump;
uint lightAlias_autoBumpParams;

uint gen_attribConsole;
uint gen_attribColors;
uint gen_colorModulate;
uint gen_color;
uint gen_sky;
uint gen_mvp;
uint gen_orthoMatrix;
uint gen_3d;
uint gen_light;

uint cin_params;
uint cin_orthoMatrix;

uint ls_fade;
uint ls_orthoMatrix;

uint gamma_control;
uint gamma_orthoMatrix;

uint menu_params;
uint menu_orthoMatrix;

uint fxaa_screenSize;
uint fxaa_orthoMatrix;

uint particle_depthParams;
uint particle_mask;
uint particle_thickness;
uint particle_colorModulate;
uint particle_mvp;
uint particle_mv;
uint particle_projMat;
uint particle_texRotMat;
uint particle_distort;

uint water_deformMul;
uint water_alpha;
uint water_thickness;
uint water_screenSize;
uint water_depthParams;
uint water_colorModulate;
uint water_ambient;
uint water_trans;
uint water_entity2world;
uint water_mvp;
uint water_mv;
uint water_pm;
uint water_mirror;

uint gaussx_matrix;
uint gaussy_matrix;

uint glare_params;
uint glare_matrix;

uint bloomDS_threshold;
uint bloomDS_matrix;

uint bloomFP_params;
uint bloom_FP_matrix;

uint ref_deformMul;
uint ref_mvp;
uint ref_mvm;
uint ref_pm;
uint ref_alpha;
uint ref_thickness;
uint ref_thickness2;
uint ref_viewport;
uint ref_depthParams;
uint ref_ambientScale;
uint ref_mask;
uint ref_alphaMask;

uint rb_params;
uint rb_matrix;
uint rb_cont;

uint dof_screenSize;
uint dof_params;
uint dof_orthoMatrix;

uint film_screenRes;
uint film_rand;
uint film_frameTime;
uint film_params;
uint film_matrix;

uint mb_params;
uint mb_orthoMatrix;

uint depthDS_params;
uint depthDS_orthoMatrix;

uint ssao_params;
uint ssao_vp;
uint ssao_orthoMatrix;

uint ssaoB_sapmles;
uint ssaoB_axisMask;
uint ssaoB_orthoMatrix;

uint therm_matrix;
uint thermf_matrix;

uint sv_mvp;
uint sv_lightOrg;

uint ss_orthoMatrix;
uint ss_tex;

uint light2d_orthoMatrix;
uint light2d_params;

uint fixfov_orthoMatrix;
uint fixfov_params;

#define	MAX_VERTEX_CACHES	4096

void R_DrawFullScreenQuad();
static GLenum	drawbuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };

qboolean nvApiInit;

void R_GpuInfo_f(void);
void ADL_PrintGpuInfo();
void ADL_Shutdown();

/*
====================================================================

IMPLEMENTATION SPECIFIC FUNCTIONS

====================================================================
*/

void GLimp_EndFrame (void);
qboolean GLimp_Init (void *hinstance, void *hWnd);
void GLimp_Shutdown (void);
rserr_t GLimp_SetMode (unsigned *pwidth, unsigned *pheight, int mode,
	qboolean fullscreen);
void GLimp_AppActivate (qboolean active);


#ifndef __GLW_H__
#define __GLW_H__

typedef struct {
#ifdef _WIN32
	HINSTANCE	hInstance;
	void	*wndproc;

	HDC     hDC;			// handle to device context
	HWND    hWnd;			// handle to window
	HGLRC   hGLRC;			// handle to GL rendering context
	
	HWND	hWndFake;
	HDC		hDCFake;
	HGLRC	hGLRCFake;

	HINSTANCE hinstOpenGL;	// HINSTANCE for the OpenGL library

	const char	*wglExtsString;
	const char	*wglRenderer;
	int desktopWidth, desktopHeight;
	int monitorWidth, monitorHeight;
	int desktopBitPixel;
	
	int desktopPosX, desktopPosY;
	int virtualX, virtualY;
	int virtualWidth, virtualHeight;
	int borderWidth, borderHeight;

	qboolean pixelFormatSet;
	char	 desktopName[32];		// no monitor specified if empty, drawing on primary display

#else
	void *hinstOpenGL;
#endif
} glwstate_t;

extern glwstate_t glw_state;

#endif

#endif							/* R_LOCAL_H */
