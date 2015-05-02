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

#ifdef _WIN32
#include "glext.h"
#include <math.h>
#include "wglext.h"

#include "imagelib/il.h"
#include "imagelib/ilu.h"
#include "imagelib/ilut.h"

#else
#include <GL/glext.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

typedef void ILvoid;
#define _inline inline
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

#define BUFFER_OFFSET(i) ((byte *)NULL + (i))

// ===================================================================

typedef enum {
	rserr_ok,

	rserr_invalid_fullscreen,
	rserr_invalid_mode,

	rserr_unknown
} rserr_t;

#include "r_model.h"

void GL_BeginRendering (int *x, int *y, int *width, int *height);
void GL_EndRendering (void);

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
extern	image_t *r_blackTexture;

extern	image_t	*r_defBump;
extern	image_t	*ScreenMap;
extern	image_t	*r_envTex;
extern	image_t	*shadowMask;
extern	image_t	*r_scanline;
extern	image_t	*r_lightAttenMap;
extern	image_t	*weaponHack;
extern	image_t *fxaaMap;
extern	image_t *fboScreen;

#define MAX_FILTERS 256
extern	image_t	*r_lightCubeMap[MAX_FILTERS];
#define	MAX_GLOBAL_FILTERS	128

extern unsigned int bloomtex;
extern unsigned int thermaltex;
extern unsigned int fxaatex;
extern uint fboDepth;
extern uint fboColor0;
extern uint fboColor1;

extern entity_t *currententity;
extern model_t *currentmodel;
extern int r_visframecount;
extern int r_framecount;
extern cplane_t frustum[5];

extern	int gl_filter_min, gl_filter_max;
extern	int flareQueries[MAX_WORLD_SHADOW_LIHGTS];

//extern 	worldShadowLight_t *shadowLight;

//
// view origin
//
extern vec3_t vup;
extern vec3_t vpn;
extern vec3_t vright;
extern vec3_t r_origin;
extern entity_t r_worldentity;

typedef vec_t mat4x4_t[16];


mat4x4_t r_world_matrix;
mat4x4_t r_project_matrix;
mat4x4_t r_modelViewInv;
mat4x4_t r_modelViewProjection;
mat4x4_t r_oldModelViewProjection;

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
cvar_t *r_polyBlend;
cvar_t *r_vsync;
cvar_t *r_textureMode;
cvar_t *r_lockPvs;
cvar_t *r_fullScreen;

cvar_t *r_gamma;
cvar_t *r_brightness;
cvar_t *r_contrast;
cvar_t *r_saturation;

cvar_t *vid_ref;
cvar_t *r_finish;

cvar_t	*r_causticIntens;

cvar_t	*r_displayRefresh;

cvar_t	*r_screenShot;
cvar_t	*r_screenShotJpegQuality;
cvar_t	*r_screenShotGamma;
cvar_t	*r_screenShotContrast;

cvar_t	*r_worldColorScale;
cvar_t	*r_textureCompression;
cvar_t	*r_anisotropic;
cvar_t	*r_maxAnisotropy;

cvar_t	*r_shadows;
cvar_t	*r_playerShadow;
cvar_t	*r_useLightOccluders;

cvar_t	*r_arbSamples;
cvar_t	*r_nvSamplesCoverange;
cvar_t	*r_fxaa;
cvar_t	*deathmatch;

cvar_t	*r_drawFlares;
cvar_t	*r_flaresIntens;
cvar_t	*r_flareWeldThreshold;
cvar_t	*r_useConditionalRender;

cvar_t	*r_customWidth;
cvar_t	*r_customHeight;

cvar_t	*r_bloom;
cvar_t	*r_bloomThreshold;
cvar_t	*r_bloomIntens;
cvar_t	*r_bloomBright;
cvar_t	*r_bloomExposure;
cvar_t	*r_bloomStarIntens;

cvar_t	*r_ssao;
cvar_t	*r_ssaoIntensity;
cvar_t	*r_ssaoQuality;
cvar_t	*r_ssaoScale;
cvar_t	*r_ssaoBlur;

cvar_t	*sys_priority;

cvar_t	*r_DrawRangeElements;

cvar_t	*r_skipStaticLights;
cvar_t	*r_ambientLevel;
cvar_t	*r_lightsWeldThreshold;
cvar_t	*r_debugLights;
cvar_t	*r_occLightBoundsSize;
cvar_t	*r_debugOccLightBoundsSize;
cvar_t	*r_useLightScissors;
cvar_t	*r_useDepthBounds;
cvar_t	*r_specularScale;

cvar_t	*r_zNear;
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

cvar_t	*r_motionBlur;
cvar_t	*r_motionBlurSamples;
cvar_t	*r_motionBlurFrameLerp;

cvar_t	*r_radialBlur;
cvar_t	*r_radialBlurFov;

cvar_t	*r_tbnSmoothAngle;

cvar_t	*r_filmGrain;

cvar_t	*r_softParticles;
cvar_t	*r_ignoreGlErrors;

cvar_t	*r_lightEditor;
cvar_t	*r_CameraSpaceLightMove;

cvar_t	*r_allowIntel;
//cvar_t	*r_stereoVision;
//cvar_t	*r_stereoSeparation;


int CL_PMpointcontents (vec3_t point);
qboolean outMap;

extern float ref_realtime;

extern int gl_lightmap_format;
extern int gl_solid_format;
extern int gl_alpha_format;
extern int gl_tex_solid_format;
extern int gl_tex_alpha_format;

extern qboolean inwaterfognode;
extern int r_visframecount;

extern int radarOldTime;

extern qboolean spacebox;

extern qboolean arbMultisampleSupported;

void GL_Bind (int texnum);
void GL_MBind (GLenum target, int texnum);
void GL_TexEnv (GLenum value);
void GL_EnableMultitexture (qboolean enable);
void GL_SelectTexture (GLenum);
void GL_MBindCube (GLenum target, int texnum);

void R_LightPoint (vec3_t p, vec3_t color);

void R_InitLightgrid (void);
void R_RenderFlares (void);

void R_DrawShadowVolume (entity_t * e);
worldShadowLight_t *R_AddNewWorldLight (vec3_t origin, vec3_t color, float radius[3], int style, int filter, vec3_t angles, vec3_t speed,
	qboolean isStatic, int isShadow, int isAmbient, float cone, qboolean ingame, int flare, vec3_t flareOrg,
	float flareSize, char target[MAX_QPATH], int start_off, int fog, float fogDensity);
void R_DrawParticles ();
void GL_DrawRadar (void);
void R_DrawAlphaPoly (void);
void R_DrawReflectivePoly (void);
void R_RenderDecals (void);
void R_LightColor (vec3_t org, vec3_t color);
void MyGlPerspective (GLdouble fov, GLdouble aspectr, GLdouble zNear);
qboolean R_CullAliasModel (vec3_t bbox[8], entity_t *e);
void RenderLavaSurfaces (msurface_t * fa);
int CL_PMpointcontents2 (vec3_t point, struct model_s * ignore);
void R_DrawAliasDistortModel (entity_t *e);
void VID_MenuInit (void);
void AnglesToMat3 (const vec3_t angles, mat3_t m);
void Mat3_TransposeMultiplyVector (const mat3_t m, const vec3_t in, vec3_t out);
void R_ShutdownPrograms (void);
void GL_BindNullProgram (void);
void GL_BindRect (int texnum);
void GL_MBindRect (GLenum target, int texnum);
void Matrix4_Multiply (const mat4x4_t m1, const mat4x4_t m2, mat4x4_t out);
void Matrix4_Copy (const mat4x4_t m1, mat4x4_t m2);
qboolean InvertMatrix (const mat4x4_t m, mat4x4_t invOut);
void R_BlobShadow (void);
void R_ShadowBlend ();
void R_Bloom (void);
void R_ThermalVision (void);
void R_RadialBlur (void);
void R_DofBlur (void);
void R_FXAA (void);
void R_FilmGrain (void);
void R_ListPrograms_f (void);
void R_InitPrograms (void);
void R_ClearWorldLights (void);
qboolean R_CullSphere (const vec3_t centre, const float radius);
void R_DebugLights (vec3_t lightOrg);
void R_CastBspShadowVolumes (void);
void R_CastAliasShadowVolumes (void);
void R_DrawAliasModelLightPass (qboolean weapon_model);
void R_RotateForEntity (entity_t * e);
void GL_MBind3d (GLenum target, int texnum);
void R_CapturePlayerWeapon ();
void R_LightScale (void);

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
qboolean R_CullBox_ (vec3_t mins, vec3_t maxs, cplane_t *frust);
void GL_DrawAliasFrameLerpLight (dmdl_t *paliashdr);
qboolean SurfInFrustum (msurface_t *s);
qboolean HasSharedLeafs (byte *v1, byte *v2);
qboolean InLightVISEntity ();
float SphereInFrustum (vec3_t o, float radius);
void R_DrawLightBrushModel ();
qboolean R_DrawLightOccluders ();
void UpdateLightEditor (void);
void Load_LightFile ();
void R_SetViewLightDepthBounds ();
qboolean BoundsIntersectsPoint (vec3_t mins, vec3_t maxs, vec3_t p);
extern int num_visLights;
extern int lightsQueries[MAX_WORLD_SHADOW_LIHGTS];
extern int numLightQ;
extern int numFlareOcc;
extern qboolean FoundReLight;
qboolean PF_inPVS (vec3_t p1, vec3_t p2);
void R_SetFrustum (void);
qboolean BoxOutsideFrustum (vec3_t mins, vec3_t maxs);
void R_SetViewLightScreenBounds ();
qboolean BoundsIntersect (const vec3_t mins1, const vec3_t maxs1, const vec3_t mins2, const vec3_t maxs2);
void R_DrawLightFlare ();
void GL_LoadMatrix (GLenum mode, const mat4_t matrix);

// TODO: move to common/q_math.h
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

void SetPlaneType (cplane_t *plane);
void SetPlaneSignBits (cplane_t *plane);
void R_SetLightPlanes ();
trace_t CL_PMTraceWorld (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int mask, qboolean checkAliases);
void DrawTextureChains2 ();

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
extern float   wTmu3Array[MAX_BATCH_SURFS][2];
extern float   wTmu4Array[MAX_BATCH_SURFS][2];
extern float   wTmu5Array[MAX_BATCH_SURFS][2];
extern float   wTmu6Array[MAX_BATCH_SURFS][2];

extern model_t *r_worldmodel;

extern unsigned d_8to24table[256];
extern float	d_8to24tablef[256][3];

extern int registration_sequence;

int R_Init (void *hinstance, void *hWnd);
void R_Shutdown (void);

void R_RenderView (refdef_t * fd);
void GL_ScreenShot_f (void);
void R_DrawAliasModel (entity_t * e, qboolean weapon_model);
void R_DrawBrushModel ();
void R_DrawSpriteModel (entity_t * e);
void R_DrawBeam ();
void R_DrawBSP (void);
void R_RenderDlights (void);
void R_RenderBrushPoly (msurface_t * fa);
void R_InitEngineTextures (void);
void R_LoadFont (void);

qboolean R_CullBox (vec3_t mins, vec3_t maxs);
void R_MarkLeaves (void);
void R_DrawWaterPolygons (msurface_t * fa);
void R_AddSkySurface (msurface_t * fa);
void R_ClearSkyBox (void);
void R_DrawSkyBox (qboolean color);
void R_MarkLights (dlight_t * light, int bit, mnode_t * node);

void COM_StripExtension (char *in, char *out);

void Draw_GetPicSize (int *w, int *h, char *name);
void Draw_Pic (int x, int y, char *name);
void Draw_StretchPic (int x, int y, int w, int h, char *name);
void Draw_TileClear (int x, int y, int w, int h, char *name);
void Draw_Fill (int x, int y, int w, int h, float r, float g, float b, float a);
void Draw_FadeScreen (void);
void Draw_StretchRaw (int x, int y, int w, int h, int cols, int rows,
	byte * data);

void R_BeginFrame ();
void R_SetPalette (const unsigned char *palette);

int Draw_GetPalette (void);

struct image_s *R_RegisterSkin (char *name);

void LoadPCX (char *filename, byte ** pic, byte ** palette, int *width,
	int *height);

image_t *GL_LoadPic (char *name, byte * pic, int width, int height,
	imagetype_t type, int bits);

image_t *GL_FindImage (char *name, imagetype_t type);

void GL_TextureMode (char *string);
void GL_ImageList_f (void);

void GL_InitImages (void);
void GL_ShutdownImages (void);

void GL_FreeUnusedImages (void);
qboolean R_CullOrigin (vec3_t origin);

/*
** GL extension emulation functions
*/
void GL_DrawParticles ();
void R_TransformToScreen_Vec3 (vec3_t in, vec3_t out);
void GL_Blend (qboolean on, int dst, int src);

int GL_MsgGLError (char* Info);
void CreateWeaponRect (void);
void Create_FBO (void);
//void CreateFboBuffer (void);
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


typedef struct {
	qboolean fullscreen;

	int prev_mode;


	int lightmap_textures;

	int currenttextures[4];
	int num_tmu;
	int currenttmu;

	qboolean	DrawRangeElements;
	qboolean	separateStencil;
	qboolean	texture_compression_arb;
	int			displayrefresh;
	qboolean	nv_multisample_hint;
	qboolean	arb_occlusion;
	qboolean	arb_occlusion2;
	unsigned	query_passed;
	qboolean	arb_multisample;
	qboolean	wgl_swap_control_tear;
	qboolean	conditional_render;
	qboolean	glsl;
	qboolean	nPot;
	qboolean	glslBinary;
	qboolean	depthBoundsTest;
	qboolean	shader5;
	int			programId;
	int			lastdFactor;
	int			lastsFactor;
	float		color[4];
	int			x, y, w, h;
	int			numFormats;
	GLenum		binaryFormats;
	GLenum		matrixMode;

	unsigned char originalRedGammaTable[256];
	unsigned char originalGreenGammaTable[256];
	unsigned char originalBlueGammaTable[256];

	GLuint	vbo_fullScreenQuad;
	GLuint	vbo_halfScreenQuad;
	GLuint	vbo_quarterScreenQuad;
	GLuint	ibo_quadTris;
	GLuint	vbo_Dynamic;
	GLuint	ibo_Dynamic;
	GLuint	vbo_BSP;
	int xyz_offset;
	int st_offset;
	int lm_offset;
	int nm_offset;
	int tg_offset;
	int bn_offset;
	int vertex_size;

	mat4_t			projectionMatrix;
	mat4_t			modelViewMatrix;		// ready to load

	// frame buffer
	int			maxRenderBufferSize;
	int			maxColorAttachments;
	int			maxSamples;
	int			maxDrawBuffers;

	// FIXME: move somewhere else
	uint		fboId, dpsId, fbo_weaponMask;

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

	GLfloat			rgba[4];

	qboolean		glDepthBoundsTest;
	GLfloat			depthBoundsMins;
	GLfloat			depthBoundsMax;

	vec4_t			fontColor;
} glstate_t;


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
void GL_Color4f (GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void GL_Color3f (GLfloat r, GLfloat g, GLfloat b);
void GL_Color4fv (vec4_t rgba);
void GL_Color3fv (vec3_t rgb);
void GL_LoadIdentity (GLenum mode);

void GL_Enable (GLenum cap);
void GL_Disable (GLenum cap);

void GL_BindFB (fbo_t *fb);
void R_FB_Init (void);
void R_FB_Shutdown (void);
void R_FB_List_f (void);

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

extern vec2_t texCoord[MAX_VERTEX_ARRAY];
extern vec2_t texCoord1[MAX_VERTEX_ARRAY];
extern vec3_t vertCoord[MAX_VERTEX_ARRAY];
extern vec4_t colorCoord[MAX_VERTEX_ARRAY];

void R_PrepareShadowLightFrame (qboolean weapon);
extern worldShadowLight_t *shadowLight_static, *shadowLight_frame;
qboolean BoundsAndSphereIntersect (const vec3_t mins, const vec3_t maxs, const vec3_t origin, float radius);

#define Vector4Set(v, a, b, c, d)	((v)[0]=(a),(v)[1]=(b),(v)[2]=(c),(v)[3]=(d))
#define Vector4Copy(a,b) ((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])
#define PlaneDiff(point,plane) (((plane)->type < 3 ? (point)[(plane)->type] : DotProduct((point), (plane)->normal)) - (plane)->dist)

typedef byte color4ub_t[4];
#define clamp(a,b,c)	((a) < (b) ? (b) : (a) > (c) ? (c) : (a))

void Q_strncatz (char *dst, int dstSize, const char *src);

#define	MAX_LIGHTMAPS		4		// max number of atlases
#define	LIGHTMAP_SIZE		1024
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
	int				numDefs;
	unsigned		defBits[MAX_PROGRAM_DEFS];
	char			defStrings[MAX_PROGRAM_DEFS][MAX_DEF_NAME];
	int				numId;
	int				id[MAX_PROGRAM_ID];
	qboolean		valid;		// true if all permutations linked successfully

} glslProgram_t;

glslProgram_t r_programs[MAX_PROGRAMS];

glslProgram_t		*ambientWorldProgram;
glslProgram_t		*lightWorldProgram;
glslProgram_t		*aliasAmbientProgram;
glslProgram_t		*aliasBumpProgram;
glslProgram_t		*gaussXProgram;
glslProgram_t		*gaussYProgram;
glslProgram_t		*blurStarProgram;
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
glslProgram_t		*genericProgram;
glslProgram_t		*cinProgram;
glslProgram_t		*loadingProgram;
glslProgram_t		*fxaaProgram;
glslProgram_t		*filmGrainProgram;
glslProgram_t		*nullProgram;
glslProgram_t		*gammaProgram;
glslProgram_t		*FboProgram;

void GL_BindProgram (glslProgram_t *program, int defBits);
void R_CaptureDepthBuffer ();
void R_CaptureColorBuffer ();
void R_DrawLightWorld ();

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

typedef enum glsl_attribute {
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

uint ambientWorld_diffuse;
uint ambientWorld_add;
uint ambientWorld_lightmap[3];
uint ambientWorld_lightmapType;
uint ambientWorld_normalmap;
uint ambientWorld_ssao;
uint ambientWorld_ssaoMap;
uint ambientWorld_parallaxParams;
uint ambientWorld_colorScale;
uint ambientWorld_specularScale;
uint ambientWorld_specularExp;
uint ambientWorld_viewOrigin;
uint ambientWorld_parallaxType;
uint ambientWorld_ambientLevel;

uint lightWorld_diffuse;
uint lightWorld_normal;
uint lightWorld_csm;
uint lightWorld_cube;
uint lightWorld_atten;
uint lightWorld_caustic;
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
uint lightWorld_specularExp;
uint lightWorld_ambient;
uint lightWorld_attenMatrix;
uint lightWorld_cubeMatrix;

uint ambientAlias_diffuse;
uint ambientAlias_normalmap;
uint ambientAlias_add;
uint ambientAlias_env;
uint ambientAlias_colorModulate;
uint ambientAlias_addShift;
uint ambientAlias_isEnvMaping;
uint ambientAlias_envScale;
uint ambientAlias_isShell;
uint ambientAlias_scroll;

uint lightAlias_diffuse;
uint lightAlias_normal;
uint lightAlias_cube;
uint lightAlias_atten;
uint lightAlias_caustic;
uint lightAlias_colorScale;
uint lightAlias_viewOrigin;
uint lightAlias_lightOrigin;
uint lightAlias_lightColor;
uint lightAlias_fog;
uint lightAlias_fogDensity;
uint lightAlias_causticsIntens;
uint lightAlias_isCaustics;
uint lightAlias_specularScale;
uint lightAlias_specularExp;
uint lightAlias_ambient;
uint lightAlias_attenMatrix;
uint lightAlias_cubeMatrix;

uint gen_attribConsole;
uint gen_attribColors;
uint gen_tex;
uint gen_tex1;
uint gen_colorModulate;
uint gen_color;
uint gen_sky;

uint gamma_screenMap;
uint gamma_control;

uint particle_texMap;
uint particle_depthMap;
uint particle_depthParams;
uint particle_mask;
uint particle_thickness;
uint particle_colorModulate;

uint refract_normalMap;
uint refract_baseMap;
uint refract_screenMap;
uint refract_depthMap;
uint refract_deformMul;
uint refract_alpha;
uint refract_thickness;
uint refract_screenSize;
uint refract_depthParams;
uint refract_thickness;
uint refract_thickness2;
uint refract_ambient;
uint refract_alphaMask;
uint refract_mask;

uint lava_diffuse;
uint lava_csm;
uint lava_parallaxParams;
uint lava_viewOrigin;
uint lava_parallaxType;

uint water_deformMap;
uint water_baseMap;
uint water_screenMap;
uint water_depthMap;
uint water_deformMul;
uint water_alpha;
uint water_thickness;
uint water_screenSize;
uint water_depthParams;
uint water_thickness;
uint water_colorModulate;
uint water_ambient;
uint water_trans;
uint water_entity2world;

#define	MAX_VERTEX_CACHES	4096

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
void GLimp_EnableLogging (qboolean enable);
void GLimp_LogNewFrame (void);

#ifndef __GLW_H__
#define __GLW_H__

typedef struct {
#ifdef _WIN32
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
#else
	void *hinstOpenGL;
#endif
} glwstate_t;

extern glwstate_t glw_state;

#endif

#endif							/* R_LOCAL_H */
