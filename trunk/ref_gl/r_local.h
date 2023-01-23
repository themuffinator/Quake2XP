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

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;

#include "glcorearb.h"

#include <stdio.h>
#include <math.h>

#include "../client/ref.h"
#include "r_md3.h"

#ifdef _WIN32
#include "wglext.h"
#endif

#include "qgl.h"

#ifdef _WIN32
	#include "../win32/adl/adl_sdk.h"
	#include "../win32/nvapi/nvapi.h"
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
	uint width, height;		// coordinates from main game
} viddef_t;
#endif

extern viddef_t vid;

#define	MAX_GLTEXTURES		16384
#define	IMAGE_HASH_SIZE		MAX_GLTEXTURES
#define MAX_IDX				65536
#define BUFFER_OFFSET(i) ((byte *)NULL + (i))

// ===================================================================

typedef enum {
	rserr_ok,
	rserr_invalid_fullscreen,
	rserr_invalid_mode,
	rserr_unknown
} rserr_t;

typedef struct winScreenModes_s {
	int num;
	int w;
	int h;
	int hz;
	char *description;
} winScreenModes_t;

winScreenModes_t winScreenModes[64];
#define NUM_WINSCREENMODES ( sizeof( winSreenModes ) / sizeof( winSreenModes[0] ) )
char** vid_winModes;

#include "r_model.h"

typedef struct hdri_s {
	float	*data;
	uint	width, height;
} hdri_t;
qboolean R_LoadHdri(const char* fileName, hdri_t* res);

void GL_SetDefaultState (void);
void GL_UpdateSwapInterval (void);

extern double gldepthmin, gldepthmax;

#define BACKFACE_EPSILON	0.01


//====================================================

#define		MAX_CAUSTICS		32
image_t *r_caustic[MAX_CAUSTICS];

#define		MAX_WATER_NORMALS		32
image_t *r_waterNormals[MAX_WATER_NORMALS];

#define		MAX_FLY		2
image_t *fly[MAX_FLY];

#define		MAX_FLAMEANIM		5
image_t *flameanim[MAX_FLAMEANIM];

#define		MAX_BLOOD 6
image_t *r_blood[MAX_BLOOD];

#define		MAX_xBLOOD 6
image_t *r_xblood[MAX_BLOOD];

#define	MAX_SHELLS 6
image_t	*r_texshell[MAX_SHELLS];

#define		MAX_EXPLODE 8
image_t *r_explode[MAX_EXPLODE];

#define		MAX_BFG_EXPL		32
image_t *r_bfg_expl[MAX_BFG_EXPL];

#define		MAX_FILTERS 256
image_t* r_lightCubeMap[MAX_FILTERS];
#define		MAX_GLOBAL_FILTERS	38

//#define		MAX_LUTS 8
//image_t* r_3dLut[MAX_LUTS];
//int			lutCount;

image_t gltextures[MAX_IDX];
int numgltextures;

image_t *r_notexture;
image_t *r_distort;
//image_t *r_depthTex;
image_t	*cinMap;

image_t *r_particleTexture[PT_MAX];
image_t *r_decalTexture[DECAL_MAX];
image_t	* r_rail_normal;
image_t* r_laser_normal;

image_t *r_flare;

image_t *draw_chars, *draw_charsRu, *draw_charsInt;
image_t *r_DSTTex;

image_t	*r_defBump;
image_t	*r_envTex;
image_t	*r_randomNormalTex;
image_t	*r_conBump;

image_t	*r_whiteMap;
image_t *skinBump;

image_t *r_miniDepthTex;
image_t *r_ssaoColorTex[2];

image_t* r_hdrScreen;
image_t* r_hdrScreenCopy;
image_t* r_depthStencilTexture;
image_t* r_hdrScreenCopy2d;
image_t* r_finalScreen;
image_t* r_linearDepth;
image_t* r_hdr64image;

image_t	*r_cinImage;
image_t	*r_hdrBloomImage;
image_t	*r_thermalImage;

int i_stencilView;
uint64_t i_stencilView_handle;

int			skyCube;
uint64_t	skyCube_handle;

uint fboId;
byte r_ssaoColorTexIndex;
uint fboDps;

extern entity_t *currententity;
extern model_t *currentmodel;
extern int r_visframecount;
extern int r_framecount;
extern cplane_t frustum[6];

extern	int gl_filter_min, gl_filter_max;

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

cvar_t *r_imageAutoBump;
cvar_t *r_imageAutoBumpScale;
cvar_t *r_imageAutoSpecularScale;

cvar_t *r_lockPvs;
cvar_t *r_fullScreen;

cvar_t	*r_brightness;
cvar_t	*r_contrast;
cvar_t	*r_saturation;
cvar_t	*r_gamma;

cvar_t* r_hdrAutoExposure;
cvar_t* r_hdrKey;
cvar_t* r_hdrMinLuminance;
cvar_t* r_hdrMaxLuminance;

cvar_t	*r_hdrExposure;
cvar_t	*r_hdrLightScale;
cvar_t	*r_hdrGlarePasses;
cvar_t	*r_hdrGlareIntens;
cvar_t	*r_hdrGlare;

cvar_t	*r_colorVibrance;
cvar_t	*r_colorBalanceRed;
cvar_t	*r_colorBalanceGreen;
cvar_t	*r_colorBalanceBlue;
cvar_t	*r_useColorCorrection;

cvar_t *vid_ref;

cvar_t	*r_displayRefresh;

cvar_t	*r_screenShot;

cvar_t	*r_textureCompression;
cvar_t	*r_anisotropic;
cvar_t	*r_maxAnisotropy;
cvar_t	*r_textureLodBias;
cvar_t	*r_maxTextureSize;

cvar_t	*r_shadows;
cvar_t	*r_playerShadow;

cvar_t	*r_multiSamples;
cvar_t	*r_fxaa;
extern cvar_t	*deathmatch;

cvar_t	*r_drawFlares;
cvar_t	*r_scaleAutoLightColor;

cvar_t	*r_customWindowWidth;
cvar_t	*r_customWindowHeight;

cvar_t	*r_ssao;
cvar_t	*r_ssaoIntensity;
cvar_t	*r_ssaoScale;
cvar_t	*r_ssaoBlur;

cvar_t	*r_transSurfShading;
cvar_t	*r_blinnPhongLighting;
cvar_t	*r_skipStaticLights;
cvar_t	*r_lightmapScale;
cvar_t	*r_debugLights;
cvar_t	*r_lightScissors;
cvar_t	*r_depthBoundsTest;
cvar_t	*r_radiositySpecularScale;
cvar_t	*r_radiosityNormalMapping;
cvar_t	*r_zNear;
cvar_t	*r_zFar;

cvar_t	*hunk_bsp;
cvar_t	*hunk_md2;
cvar_t	*hunk_md3;

cvar_t	*r_parallaxMapping;
cvar_t	*r_parallaxScale;
cvar_t	*r_selfShadowingParallax;
cvar_t	*r_selfShadowOffset;
cvar_t	*r_selfShadowBlur;

cvar_t	*r_dof;
cvar_t	*r_dofBias;
cvar_t	*r_dofFocus;

cvar_t	*r_motionBlur;
cvar_t	*r_motionBlurSamples;
cvar_t	*r_motionBlurFrameLerp;

cvar_t	*r_radialBlur;
cvar_t	*r_radialBlurFov;

cvar_t	*r_globalFog;
cvar_t	*r_fogEditor;

cvar_t	*r_debugTbn;
cvar_t	*r_debugTbnLen;
cvar_t	*r_bspSmoothTbn;

cvar_t	*r_glDebugOutput;
cvar_t	*r_glMinorVersion;
cvar_t	*r_glMajorVersion;
cvar_t	*r_glCoreProfile;
cvar_t	*r_contextNoError;
cvar_t	*r_debug;
cvar_t	*r_lightEditor;
cvar_t	*r_cameraSpaceLightMove;

cvar_t	*r_fontsShadow;
cvar_t	*r_hudLighting;
cvar_t	*r_bump2D;

cvar_t	*r_filmFilter;
cvar_t	*r_filmFilterVignetSize;

cvar_t	*r_fixFovStrength; // 0.0 = no hi-fov perspective correction
cvar_t	*r_fixFovDistroctionRatio; // 0.0 = cylindrical distortion ratio. 1.0 = spherical

cvar_t	*r_screenBlend;
cvar_t	* r_screenBlendIntensity;

cvar_t	*r_useShaderCache;
cvar_t	*r_particlesOverdraw;

cvar_t	*r_colorTempK;

cvar_t *r_earthSky, *r_earthSunAzimuth, *r_earthSunIntens;

int CL_PMpointcontents (vec3_t point);
qboolean outMap;

extern float ref_realtime;

extern int r_visframecount;

qboolean xhargar2hack;
qboolean RA_Frame;

float	hdrAverageLuminance;
float	hdrMaxLuminance;
float	hdrTime;
float	hdrKey;

qboolean STB_LoadLdr(const char* name, byte** pic, int* width, int* height);
qboolean STB_LoadHdr(const char* name, float** pic, int* width, int* height);

void R_CreateScreenFbo();
void CreateHDR64Buffer(void);
void R_FboFinal();
void R_FxaaFbo();
void CreateBloomBuffer(void);
void CreateThermalBuffer(void);
void CreateLinearDepthBuffer(void);
void R_LinearDepth(void);
void R_DrawLightWorldRA(void);
void GL_SetBindlessTexture(int loc, uint64 handle);
void GL_DrawElements(int mode, uint numIdx, int type, GLvoid* idxArray);
void GL_DrawArrays(int mode, int first, int count);

void R_LightPoint (vec3_t p, vec3_t color);

void R_InitLightgrid (void);

worldShadowLight_t *R_AddNewWorldLight (vec3_t origin, vec3_t color, float radius[3], int style, int filter, vec3_t angles, vec3_t speed,
	qboolean isStatic, int isShadow, int isAmbient, float cone, qboolean ingame, int flare, vec3_t flareOrg,
	float flareSize, char target[MAX_QPATH], int start_off, int fog, float fogDensity, vec3_t occOrg, vec3_t occRad);
void R_DrawParticles (void);
void R_RenderDecals (qboolean twoside);
void R_LightColor (vec3_t org, vec3_t color);
qboolean R_CullAliasModel (vec3_t bbox[8], entity_t *e);
int CL_PMpointcontents2 (vec3_t point, struct model_s * ignore);
void VID_MenuInit (void);
void AnglesToMat3 (const vec3_t angles, mat3_t m);
void Mat3_TransposeMultiplyVector (const mat3_t m, const vec3_t in, vec3_t out);
void R_ShutdownPrograms (void);
void R_Bloom (void);
void R_ThermalVision (void);
void R_RadialBlur (void);
void R_DofBlur (void);
void R_FXAA (void);
void R_FilmFilter (void);
void R_FixFov(void);
void R_FixFov(void);
void R_ToneMaping(void);
void R_ListPrograms_f (void);
void R_InitPrograms (void);
void R_ClearWorldLights (void);
qboolean R_CullSphere (const vec3_t centre, const float radius);
void R_CastBspShadowVolumes (void);
void R_CastAliasShadowVolumes (qboolean player);
void R_DrawAliasModelLightPass (qboolean weapon_model);
void R_SetupEntityMatrix (entity_t * e);
void R_SSAO(void);
void R_DrawDepthScene(void);
void R_ScreenBlend(void);
void R_GlobalFog();
void R_MotionBlur(void);

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

extern qboolean flareEdit, occEdit;

void R_OccBBoxEdit_f(void);
void R_ResetOccBBox_f(void);

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
void R_SetFrustum (qboolean zpass);
void SetFarClip(void);
void R_SetViewLightScreenBounds ();
qboolean BoundsIntersect (const vec3_t mins1, const vec3_t maxs1, const vec3_t mins2, const vec3_t maxs2);
void R_DrawLightFlare ();
void R_DrawLightBounds(void);

void R_ShutDownVertexBuffers();

extern const mat3_t	mat3_identity;
extern const mat4_t	mat4_identity;

void Mat3_Identity (mat3_t m);
void Mat3_Copy (const mat3_t in, mat3_t out);
qboolean Mat3_Compare(const mat3_t a, const mat3_t b);

qboolean Mat4_Compare(const mat4_t a, const mat4_t b);
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

qboolean Frustum_CullBoundsProjection(const vec3_t mins, const vec3_t maxs, const vec3_t projOrigin, const int planeBits);
qboolean Frustum_CullLocalBoundsProjection(const vec3_t mins, const vec3_t maxs, const vec3_t origin, const mat3_t axis, const vec3_t projOrigin, const int planeBits);

qboolean Mat3_IsIdentity(const mat3_t mat);
void Mat3_MultiplyVector(const mat3_t m, const vec3_t in, vec3_t out);

void SetPlaneType (cplane_t *plane);
void SetPlaneSignBits (cplane_t *plane);

trace_t CL_PMTraceWorld (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int mask, qboolean checkAliases);
void AddBoundsToBounds(const vec3_t mins1, const vec3_t maxs1, vec3_t mins2, vec3_t maxs2);

void R_DrawSurfacesRA(qboolean bmodel);
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
void R_ColorTemperatureCorrection(void);

image_t* R_CreateTexture(char* texName, uint targetTex,
	uint intFormat, uint format,
	uint type, uint width, uint height,
	uint warpS, uint warpT,
	uint filterMin, uint filterMag,
	uint imageType, qboolean mipmap,
	uint* pixdata);
//====================================================================
mleaf_t* Mod_PointInLeaf(vec3_t p, model_t* model);

#define MAX_POLY_VERT		128
#define	MAX_BATCH_SURFS		21845

vec3_t	wVertexArray[MAX_BATCH_SURFS];
vec2_t	wTexArray[MAX_BATCH_SURFS];
vec2_t	wLMArray[MAX_BATCH_SURFS];
vec4_t   wColorArray[MAX_BATCH_SURFS];

vec3_t	nTexArray[MAX_BATCH_SURFS];
vec3_t	tTexArray[MAX_BATCH_SURFS];
vec3_t	bTexArray[MAX_BATCH_SURFS];
uint	indexArray[MAX_MAP_VERTS * 3];

extern	model_t *r_worldmodel;
vec3_t	BmodelViewOrg;

extern unsigned d_8to24table[256];
extern float	d_8to24tablef[256][3];

extern int registration_sequence;

extern float skyrotate;
extern vec3_t skyaxis;

int			numAlphaSurfaces;
int			numReflectiveSurfaces;
msurface_t* r_alphaSurfaces[MAX_MAP_FACES/4];
msurface_t* r_reflectiveSurfaces[MAX_MAP_FACES/4];
msurface_t* r_reflSurfChains;

msurface_t* r_heatHazeSurfaces[MAX_MAP_FACES / 4];
int numHeatHazeSurfaces;

int R_Init (void *hinstance, void *hWnd);
void R_Shutdown (void);
void GL_CheckError(const char *fileName, int line, const char *subr);

void R_RenderView (refdef_t * fd);
void GL_ScreenShot_f (void);
void R_DrawAliasModel (entity_t * e);
void R_DrawBrushModel ();
void R_DrawSpriteModel (entity_t * e);
void R_DrawBSP (void);
void R_InitEngineTextures (void);
void R_Init2D (void);

qboolean R_CullBox (vec3_t mins, vec3_t maxs);
void R_MarkLeaves (void);
void R_AddSkySurface (msurface_t * fa);
void R_ClearSkyBox (void);
void R_DrawSkyBox (qboolean color);

void COM_StripExtension (char *in, char *out);

void Draw_GetPicSize (int *w, int *h, char *name);
void Draw_Pic (int x, int y, char *name);
void Draw_StretchPic (int x, int y, int w, int h, char *name);
void Draw_TileClear (int x, int y, int w, int h, char *name);
void Draw_Fill (int x, int y, int w, int h, float r, float g, float b, float a, qboolean loading);
void Draw_StretchRaw (int x, int y, int w, int h, int cols, int rows,
	byte * data);

void R_BeginFrame ();
void R_SetPalette (const unsigned char *palette);

int Draw_GetPalette (void);

struct image_s *R_RegisterSkin (char *name);

image_t *GL_LoadPic (char *name, byte * pic, int width, int height,
	imagetype_t type, int bits, uint _hash);

image_t *GL_FindImage (char *name, imagetype_t type);

void GL_ImageList_f (void);

void GL_InitImages (void);
void GL_ShutdownImages (void);

void GL_FreeUnusedImages (void);
qboolean R_CullOrigin (vec3_t origin);
qboolean IsExtensionSupported(const char *name);

int CalcMipmapCount(int w, int h);

/*
** GL extension emulation functions
*/

void	CreateSSAOBuffer();

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
	int			maxUniformLocations;

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

	uint64	bindlessCache[8192];
	uint64	currentBindlessHandle;
	int		currentBindlessLocation;

	qboolean	texture_compression_bptc;

	qboolean	wgl_no_error;
	qboolean	wgl_swap_control_tear;
	qboolean	depthBoundsTest;
	qboolean	depthClamp;

	int			numFormats, binaryFormats;
	int			programId;
	GLenum		matrixMode;
	
	int			vaoBuffer, vboBuffer;

	mat4_t		projectionMatrix;
	mat4_t		modelViewMatrix;		// ready to load

	float		screenScale;
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
	GLenum			alphaFunc;
	GLclampf		alphaRef;

	GLboolean		colorMask[4];

	qboolean		depthTest;
	GLenum			depthFunc;
	GLboolean		depthMask;
	GLclampd		depthRange[2];

	qboolean		polygonOffsetFill;
	GLfloat			polygonOffsetFactor;
	GLfloat			polygonOffsetUnits;

	qboolean		lineSmooth;
	qboolean		alphaTest;

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

	qboolean		att_position, 
					att_normal, 
					att_tangent, 
					att_bitangent, 
					att_tex0, 
					att_tex1, 
					att_tex2, 
					att_color;
	int				viewportX,
					viewportY,
					viewportWidth,
					viewportHeight;
	vec4_t			fontColor;
} glstate_t;

typedef struct {
	vec3_t pos;
	vec3_t view;
	vec4_t color;
	int ambient;
	int fog;
	float fogDensity;
	mat4_t entAttenMat, entSpotMat;
	vec3_t spotParams;
	mat4_t mvp, cubeMat, entMat;


}lightUniforms_t;
lightUniforms_t lightUniforms;

void GL_UpdateLightPos(vec3_t pos);
void GL_UpdateLightColor(vec3_t color);

typedef struct {

GLuint	vbo_fullScreenQuadF;
GLuint	vbo_fullScreenQuad;
GLuint	vbo_halfScreenQuad;
GLuint	vbo_quarterScreenQuad;
GLuint	ibo_quadString;
GLuint	ibo_quad;

GLuint	vbo_BSP;
GLuint	vbo_aliasShadow;
GLuint	ibo_md2Shadow;
GLuint	ibo_md3Shadow;
GLuint	vbo_dynamic;
GLuint	ibo_dynamic;
GLuint	ibo_cube;
GLuint	vbo_draw2d;
GLuint	vbo_draw2dString;

int xyz_offset;

int st_offset;
int lm_offset;

int tg_offset;
int bn_offset;
int nm_offset;

int col_offset;
}vbo_t;

vbo_t vbo;

typedef struct {
	GLuint	bsp;
	GLuint	depthBSP;
	GLuint	dynamic;
	GLuint	md2Shadow;
	GLuint	md3Shadow;
	GLuint	fullscreenQuadF;
	GLuint	fullscreenQuad;
	GLuint	halfScreenQuad;
	GLuint	quaterScreenQuad;
	GLuint	draw2d;
	GLuint	draw2dString;
}vao_t;

vao_t vao;

// 2D VBO stuff
#define MAX_DRAW_STRING_LENGTH 512
#define QUADVERT 4
index_t	ibo_quadString[6 * MAX_DRAW_STRING_LENGTH];

#define	VERT2D_POS		((byte *)(NULL)+0)
#define	VERT2D_TC		((byte *)(NULL)+8)
#define VERT2D_COLOR	((byte *)(NULL)+16)

///	fill array
/// xy0, st0, rgba0
/// xy1, st1, rgba1
/// xy2, st2, rgba2
/// xy3, st3, rgba3

typedef struct {
	vec2_t pos;
	vec2_t texCoord;
	vec4_t colorCoord;
}vertex2d_t;

typedef struct {
	vertex2d_t data[QUADVERT];
}tess2d_t;
tess2d_t tess2d;

typedef struct {
	vertex2d_t data[QUADVERT * MAX_DRAW_STRING_LENGTH];
}tess2dString_t;
tess2dString_t tess2dString;

#define MAX_2D_VERTS 2048 //QUADVERT * MAX_DRAW_STRING_LENGTH
vec2_t	texCoord[MAX_2D_VERTS];
vec2_t	texCoord1[MAX_2D_VERTS];
vec2_t	vertCoord[MAX_2D_VERTS];
vec4_t	colorCoord[MAX_2D_VERTS];

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
void GL_EnableVertexAttribArray(GLenum cap);
void GL_DisableVertexAttribArray(GLenum cap);
void GL_Viewport(GLint x, GLint y, GLint w, GLint h);

#ifndef BIT
#define BIT(num)				(1 << (num))
#endif


extern glconfig_t gl_config;
extern glstate_t gl_state;

#define VA_SetElem2(v,a,b)		((v)[0]=(a),(v)[1]=(b))
#define VA_SetElem3(v,a,b,c)	((v)[0]=(a),(v)[1]=(b),(v)[2]=(c))
#define VA_SetElem4(v,a,b,c,d)	((v)[0]=(a),(v)[1]=(b),(v)[2]=(c),(v)[3]=(d))

#define VA_SetElem2v(v,a)	((v)[0]=(a)[0],(v)[1]=(a)[1])
#define VA_SetElem3v(v,a)	((v)[0]=(a)[0],(v)[1]=(a)[1],(v)[2]=(a)[2])
#define VA_SetElem4v(v,a)	((v)[0]=(a)[0],(v)[1]=(a)[1],(v)[2]=(a)[2],(v)[3]=(a)[3])

#define MAX_VERTICES		16384
#define MAX_INDICES			65536

#define MAX_STREAM_VBO_VERTS MD3_MAX_VERTS * MD3_MAX_MESHES
#define MAX_STREAM_IBO_IDX	 MD3_MAX_VERTS * MD3_MAX_MESHES

#define CUBE_INDICES 36

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

#define	MAX_LIGHTMAPS		4
#define	LIGHTMAP_SIZE		4096
#define XPLM_NUMVECS		3	// Do not change
#define LM_BLOCKLIGHTS_SIZE LIGHTMAP_SIZE * LIGHTMAP_SIZE * 3

typedef struct {
	// Atlas texId for each vector.
	int		texnum[3];
	uint64	handle[3];

	// The lightmap texture data needs to be kept in
	// main memory so texsubimage can update properly.
	byte		lightmap_buffer[3][LM_BLOCKLIGHTS_SIZE];
	// Block loading.
	int allocated[LIGHTMAP_SIZE];
} gllightmapstate_t;

gllightmapstate_t gl_lms;
extern const vec3_t r_xplmBasisVecs[XPLM_NUMVECS];

/*
====================================================================

PROGRAMS

====================================================================
*/
#define	MAX_PROGRAM_DEFS	8					// max permutation defs program can have
#define	MAX_PROGRAM_ID		(1 << MAX_PROGRAM_DEFS)		// max GL indices per program object

#define	MAX_UNIFORM_NAME	64
#define	MAX_PROGRAMS		256

typedef struct {
	char			name[MAX_UNIFORM_NAME];
} glslUniform_t;

typedef struct glslProgram_s {

	struct glslProgram_s	*nextHash;

	char			name[MAX_QPATH];
	int				id;
	qboolean		valid;		// qtrue if all permutations linked successfully

} glslProgram_t;

glslProgram_t r_programs[MAX_PROGRAMS];

glslProgram_t		*ambientWorldProgram;
glslProgram_t		*lightWorldProgram;
glslProgram_t		*aliasAmbientProgram;
glslProgram_t		*md3AmbientProgram;
glslProgram_t		*aliasBumpProgram;
glslProgram_t		*glareProgram;
glslProgram_t		*bloomdsProgram;
glslProgram_t		*bloomfpProgram;
glslProgram_t		*bloomBlurProgram;
glslProgram_t		*motionBlurProgram;
glslProgram_t		*ssaoProgram;
glslProgram_t		*depthDownsampleProgram;
glslProgram_t		*linearDepthProgram;
glslProgram_t		*ssaoBlurProgram;
glslProgram_t		*glassProgram;
glslProgram_t		*lightGlassProgram;
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
glslProgram_t		*fxaaProgram;
glslProgram_t		*filmicFxProgram;
glslProgram_t		*nullProgram;
glslProgram_t		*lutProgram;
glslProgram_t		*whiteBalanceProgram;
glslProgram_t		*light2dProgram;
glslProgram_t		*fixFovProgram;
glslProgram_t		*menuProgram;
glslProgram_t		*skyProgram;
glslProgram_t		*colorProgram;
glslProgram_t		*flareProgram;
glslProgram_t		*globalFogProgram;
glslProgram_t		*spriteProgram;
glslProgram_t		*screenFlashProgram;
glslProgram_t		*tbnDebugProgram;
glslProgram_t		*tonemapProgram;
glslProgram_t		*finalPassProgram;
glslProgram_t		*heatHazeProgram;

void GL_BindProgram (glslProgram_t *program);
void R_CaptureColorBuffer ();
void R_DrawLightWorld ();
void R_SetupOrthoMatrix(void);

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
	U_MVP_MATRIX,
	U_MODELVIEW_MATRIX,
	U_PROJ_MATRIX,
	U_ORTHO_MATRIX,

	U_TEXTURE0_MATRIX,
	U_TEXTURE1_MATRIX,
	U_TEXTURE2_MATRIX,
	U_TEXTURE3_MATRIX,
	U_TEXTURE4_MATRIX,
	U_TEXTURE5_MATRIX,
	U_TEXTURE6_MATRIX,

	U_ATTEN_MATRIX,
	U_SPOT_MATRIX,
	U_CUBE_MATRIX,

	U_SCREEN_SIZE,
	U_DEPTH_PARAMS,
	U_COLOR,
	U_COLOR_OFFSET,	// glow shift
	U_COLOR_MUL,	// color multipler

	U_SCROLL,
	U_AMBIENT_LEVEL,
	U_LM_TYPE,
	U_PARALLAX_TYPE,
	U_PARALLAX_PARAMS,
	U_USE_SSAO,
	U_LAVA_PASS,
	U_SHELL_PASS,
	U_SHELL_PARAMS,
	U_ENV_PASS,
	U_ENV_SCALE,
	
	U_LIGHT_POS,
	U_VIEW_POS,
	U_USE_FOG,
	U_FOG_DENSITY,
	U_USE_CAUSTICS,
	U_CAUSTICS_SCALE,
	U_AMBIENT_LIGHT,
	U_SPOT_LIGHT,
	U_SPOT_PARAMS,
	U_USE_AUTOBUMP,
	U_AUTOBUMP_PARAMS,
	U_USE_RGH_MAP,
	U_RGH_SCALE,
	U_SPECULAR_SCALE,

	U_TRANS_PASS,

	U_COLOR_PARAMS,
	U_COLOR_VIBRANCE,

	U_PARTICLE_THICKNESS,
	U_PARTICLE_MASK,
	U_TEXCOORD_OFFSET,
	U_PARTICLE_ANIM,

	U_PARAM_VEC2_0,
	U_PARAM_VEC2_1,
	U_PARAM_VEC2_2,
	U_PARAM_VEC2_3,
	U_PARAM_VEC2_4,
	U_PARAM_VEC2_5,

	U_PARAM_VEC3_0,
	U_PARAM_VEC3_1,
	U_PARAM_VEC3_2,
	U_PARAM_VEC3_3,
	U_PARAM_VEC3_4,
	U_PARAM_VEC3_5,

	U_PARAM_VEC4_0,
	U_PARAM_VEC4_1,
	U_PARAM_VEC4_2,
	U_PARAM_VEC4_3,
	U_PARAM_VEC4_4,
	U_PARAM_VEC4_5,

	U_PARAM_FLOAT_0,
	U_PARAM_FLOAT_1,
	U_PARAM_FLOAT_2,
	U_PARAM_FLOAT_3,
	U_PARAM_FLOAT_4,
	U_PARAM_FLOAT_5,

	U_PARAM_INT_0,
	U_PARAM_INT_1,
	U_PARAM_INT_2,
	U_PARAM_INT_3,
	U_PARAM_INT_4,
	U_PARAM_INT_5,

	U_REFR_ALPHA,
	U_REFR_DEFORM_MUL,
	U_REFR_THICKNESS0,
	U_REFR_THICKNESS1,
	U_REFR_ALPHA_MASK,
	U_REFR_MASK,

	U_WATER_DEFORM_MUL,
	U_WATER_ALPHA,
	U_WATHER_THICKNESS,
	U_WATER_TRANS,
	U_WATER_MIRROR,

	U_CONSOLE_BACK,
	U_2D_PICS,
	U_FRAG_COLOR,
	U_BINDLESS_ARRAY,
		
	U_PARAM_iVEC2_0,
	U_PARAM_iVEC2_1,
	U_PARAM_iVEC2_2,
	U_PARAM_iVEC2_3,
	U_PARAM_iVEC2_4,
	U_PARAM_iVEC2_5,

	U_TMU0,
	U_TMU1,
	U_TMU2,
	U_TMU3,
	U_TMU4,
	U_TMU5,
	U_TMU6,
	U_TMU7,
	U_TMU8,
	U_TMU9,
	U_TMU10,
}
glsl_uniform;

void R_DrawFullScreenQuad();
static GLenum	drawbuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };

qboolean nvApiInit;

void R_GpuInfo_f(void);
void ADL_PrintGpuInfo();
void ADL_Shutdown();

typedef enum {
	PLANE_ON,		// used by point check only
	PLANE_FRONT,
	PLANE_BACK,
	PLANE_CLIP
}plane_t;

typedef struct img_s {
	byte*	pixels;
	int		width;
	int		height;
} img_t;
void R_FlipImage(int idx, img_t* pix, byte* dst);

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

qboolean Sys_CheckWindowsVersion();
void Sys_CpuID();
void Sys_GetMemorySize();
void Sys_WindowsInfo();

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
	int desktopWidth, desktopHeight, desktopRefresh;
	int monitorWidth, monitorHeight;
	int desktopBitPixel;
	int dpi;
	
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
