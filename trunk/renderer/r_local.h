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

typedef struct vertexObject_s {

	char	name[MAX_QPATH];
	GLuint	id;
} vertexObject_t;

typedef struct vertexBuffer_s {

	char	name[MAX_QPATH];
	GLuint	id;
	GLuint	size;
	const void *data;
	GLuint	usage;
	GLuint	target;
} vertexBuffer_t;

#define MAX_VERTEX_OBJECTS 2048
vertexObject_t	r_vertexObject[MAX_VERTEX_OBJECTS];
int	r_numVertexObject;

vertexBuffer_t	r_vertexBuffer[MAX_VERTEX_OBJECTS];
int	r_numVertexBuffers;

typedef struct {
	vertexObject_t *sky;
	vertexObject_t *md2;
	vertexObject_t *consoleText;
	vertexObject_t *tessStream;
	vertexObject_t *md3;
	vertexObject_t *tessStreamVaoQuad;
	vertexObject_t *tess2dArray;
	vertexObject_t *tess2d;
	vertexObject_t *textArray;
	vertexObject_t *fsq;
	vertexObject_t *md3shadow;
	vertexObject_t *md2shadow;
	vertexObject_t *dynamic;
	vertexObject_t *bsp;
	vertexObject_t *depthBsp;
	vertexObject_t *dynamicCube_verts;
	vertexObject_t *drawLine;
}vao_t;
vao_t vao;

vertexObject_t *R_Alloc_VAO(const char *name, int flags);
void GL_BindVAO(vertexObject_t *va);
void GL_BindNullVAO(void);
void R_DeleteVAO(vertexObject_t *vain);
void R_VaoListing_f(void);
void R_ShotdownVAO(void);
int  GL_GetVaoBinding();

vertexBuffer_t *R_Alloc_VBO(const char *name, GLuint target, GLuint size, const void *data, GLuint usage);
void GL_BindVBO(vertexBuffer_t *vb);
void R_VboListing_f(void);
void R_DeleteVBO(vertexBuffer_t *vbin);
void R_ShotdownVBO(void);
char *q_pretifymem(float value);

typedef struct {

	vertexBuffer_t *md2AliasVbo;
	vertexBuffer_t *quadIbo;
	vertexBuffer_t *quadStringIbo;
	vertexBuffer_t *tess2dVbo;
	vertexBuffer_t *tess2dArrayVbo;
	vertexBuffer_t *fsqVbo;
	vertexBuffer_t *md3ShadowVbo;
	vertexBuffer_t *md2ShadowVbo;
	vertexBuffer_t *md2ShadowIbo;
	vertexBuffer_t *md3ShadowIbo;
	vertexBuffer_t *dynamicVbo;
	vertexBuffer_t *dynamicIbo;
	vertexBuffer_t *cubeIbo;
	vertexBuffer_t *skyBoxVbo;
	vertexBuffer_t *bspVbo;
	vertexBuffer_t *twoPointLineIbo;

	int xyz_offset;
	int st_offset;
	int lm_offset;
	int tg_offset;
	int bn_offset;
	int nm_offset;
	int col_offset;
}vbo_t;
vbo_t vbo;

#include "r_model.h"

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

image_t r_textures[MAX_GLTEXTURES];
int		r_numTextures;

image_t *r_blackTexture1x1;
image_t	*r_missingTexture;
image_t *r_distort;
image_t	*cinMap;

image_t *r_particleTexture[PT_MAX];
image_t *r_decalTexture[DECAL_MAX];
image_t	*r_rail_normal;
image_t	*r_laser_normal;

image_t *menuFont, *consFont;
image_t *r_DSTTex;

image_t	*r_defBump;
image_t	*r_envTex;
image_t	*r_randomNormalTex;
image_t	*r_conBump;

image_t	*r_whiteMap;
image_t *skinBump;

image_t *r_miniDepthTex;
image_t *r_ssaoColorTex[2];

image_t	*r_hdrScreen;
image_t	*r_hdrScreenCopy;
image_t	*r_depthStencilTexture;
image_t	*r_hdrScreenCopy2d;
image_t	*r_finalScreen;
image_t	*r_linearDepth;
image_t	*r_hdr64image;
image_t *r_hdrLuminance[2]; //current, prev 
image_t *r_fogMask;

image_t	*r_cinImage;
image_t	*r_hdrBloomImage;
image_t	*r_thermalImage;
image_t	*r_lensDirt;
image_t *r_levelSkyBox;

int			i_stencilView;
uint64_t	i_stencilView_handle;

byte r_ssaoColorTexIndex;

extern entity_t *currententity;
extern model_t *currentmodel;
extern int r_visframecount;
extern int r_framecount;
cplane_t frustum[6];

void SetPlaneType(cplane_t *plane);
void SetPlaneSignBits(cplane_t *plane);
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

cvar_t	*r_noRefresh;
cvar_t	*r_drawEntities;
cvar_t	*r_drawWorld;
cvar_t	*r_speeds;
cvar_t	*r_noVis;
cvar_t	*r_noCull;
cvar_t	*r_leftHand;
cvar_t	*r_lightLevel;
cvar_t	*r_mode;
cvar_t	*r_noBind;
cvar_t	*r_cull;
cvar_t	*r_vsync;

cvar_t	*r_imageAutoBump;
cvar_t	*r_imageAutoBumpScale;
cvar_t	*r_imageAutoSpecularScale;

cvar_t	*r_lockPvs;
cvar_t	*r_fullScreen;

cvar_t	*r_brightness;
cvar_t	*r_contrast;
cvar_t	*r_saturation;
cvar_t	*r_gamma;

cvar_t	*r_hdrAutoExposure;
cvar_t	*r_hdrExposure;
cvar_t	*r_hdrLightScale;
cvar_t	*r_hdrGlarePasses;
cvar_t	*r_hdrGlareIntens;
cvar_t	*r_hdrBloom;
cvar_t	*r_hdrBloomIntens;
cvar_t	*r_hdrBloomBlurPasses;
cvar_t	*r_hdrKey;
cvar_t	*r_hdrTime;

cvar_t	*r_colorVibrance;
cvar_t	*r_colorBalanceRed;
cvar_t	*r_colorBalanceGreen;
cvar_t	*r_colorBalanceBlue;
cvar_t	*r_useColorCorrection;

cvar_t *vid_ref;

cvar_t	*r_displayRefresh;

cvar_t	*r_screenShot;

cvar_t	*r_textureAnisotropy;
cvar_t	*r_textureLodBias;
cvar_t	*r_textureQuality;

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
cvar_t	*r_showTris;

cvar_t	*r_glDebugOutput;
cvar_t	*r_glMinorVersion;
cvar_t	*r_glMajorVersion;
cvar_t	*r_glCoreProfile;
cvar_t	*r_contextNoError;
cvar_t	*r_debug;
cvar_t	*r_lightEditor;
cvar_t	*r_cameraSpaceLightMove;

cvar_t	*r_hudLighting;
cvar_t	*r_bump2D;

cvar_t	*r_filmicFx;
cvar_t	*r_filmicFxVignetSize;

cvar_t	*r_fixFovStrength; // 0.0 = no hi-fov perspective correction
cvar_t	*r_fixFovDistroctionRatio; // 0.0 = cylindrical distortion ratio. 1.0 = spherical

cvar_t	*r_screenBlend;
cvar_t	* r_screenBlendIntensity;

cvar_t	*r_useShaderCache;
cvar_t	*r_particlesOverdraw;

cvar_t	*r_colorTempK;
cvar_t	*r_nsightDebug;

float	hdrAverageLuminance;
float	hdrMaxLuminance;
float	hdrTime;
float	hdrKey;

int CL_PMpointcontents (vec3_t point);
qboolean outMap;

extern float ref_realtime;

extern int r_visframecount;

qboolean xhargar2hack;
qboolean RA_Frame;

qboolean STB_LoadTexture(const char* name, byte** pic, int* width, int* height);

void R_CreateScreenFbo();
void R_FboFinal();
void R_Tex2dFbo();
void CreateBloomBuffer(void);
void CreateThermalBuffer(void);
void CreateLinearDepthBuffer(void);
void R_LinearDepth(void);
void R_DrawLightWorldRA(void);
void GL_SetBindlessTexture(int loc, uint64_t handle);
void GL_DrawElements(int mode, uint numIdx, int type, GLvoid* idxArray);
void GL_DrawArrays(int mode, int first, int count);
void R_HdrLumFbo();
void R_PboInit();

void R_LightPoint (vec3_t p, vec3_t color);

void R_InitLightgrid (void);

worldShadowLight_t *R_AddNewWorldLight(vec3_t origin, vec3_t color, float radius[3], int style,
	int filter, vec3_t angles, vec3_t speed, qboolean isStatic,
	int isShadow, int isAmbient, qboolean ingame,
	int flare, vec3_t flareOrg, float flareSize, char target[MAX_QPATH],
	int flags, int fogLight, float fogDensity, vec3_t occOrg, vec3_t occRad,
	qboolean proj, float fovX, float fovY, float distance);

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
void R_FilmFx (void);
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

image_t *R_CreateTexture(char *texName, uint targetTex,
	uint intFormat, uint format,
	uint flags, uint width, uint height,
	uint warpS, uint warpT,
	uint filterMin, uint filterMag,
	uint imageType,
	uint *pixdata);
//====================================================================
mleaf_t* Mod_PointInLeaf(vec3_t p, model_t* model);

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
void LoadPCX(char *filename, byte **pic, byte **palette, int *width, int *height);

qboolean R_CullBox (vec3_t mins, vec3_t maxs);
void R_MarkLeaves (void);
void R_DrawSkyBox();

void COM_StripExtension (char *in, char *out);

void Draw_GetPicSize (int *w, int *h, char *name);
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
	int			maxPatchVertices;

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

	uint64_t	bindlessCache[8192];
	uint64_t	currentBindlessHandle;
	int		currentBindlessLocation;

	qboolean	texture_compression_bptc;

	qboolean	wgl_no_error;
	qboolean	wgl_swap_control_tear;
	qboolean	depthBoundsTest;
	qboolean	depthClamp;

	int			numFormats, binaryFormats;
	int			programId;
	int			vaoId;
	int			vboId;
	int			vboType;
	GLenum		matrixMode;

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

	int				viewportX,
					viewportY,
					viewportWidth,
					viewportHeight;
	vec4_t			fontColor;
	int				numDrawBuffers;
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
void GL_Viewport(GLint x, GLint y, GLint w, GLint h);
void GL_DrawBuffers(int num);

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

#define CUBE_INDICES	36
#define CUBE_VERTS		8
#define QUAD_INDICES	6

#define MAX_VERTICES	65536
#define MAX_INDICES		MAX_VERTICES * 3

uint	indexArray[MAX_INDICES];
vec3_t	s_lerped[MAX_VERTS];

#define MAX_POLY_VERT	128

typedef struct tess_s {

	vec4_t	position[MAX_VERTICES];
	vec2_t	texCoord[MAX_VERTICES];
	vec4_t	color[MAX_VERTICES];

	vec3_t	tangent[MAX_VERTICES];
	vec3_t	binormal[MAX_VERTICES];
	vec3_t	normal[MAX_VERTICES];
	uint	indices[MAX_INDICES];
} tess_t;
tess_t tess;

typedef struct {
	vec4_t pos;
	vec2_t tc;
	vec4_t color;
	vec3_t tangent;
	vec3_t binormal;
	vec3_t normal;
}vertex_t;

typedef struct {
	vertex_t	v[4096];
	uint		indices[4096*3];
}tesselator_t;
tesselator_t tess2;

#define	TESS_OFFSET_POS			((byte *)(NULL)+0)  
#define	TESS_OFFSET_TC			((byte *)(NULL)+sizeof(vec4_t))
#define	TESS_OFFSET_COLOR		((byte *)(NULL)+sizeof(vec4_t)+sizeof(vec2_t))
#define	TESS_OFFSET_TANHENT		((byte *)(NULL)+sizeof(vec4_t)+sizeof(vec2_t)+sizeof(vec4_t))
#define	TESS_OFFSET_BINORMAL	((byte *)(NULL)+sizeof(vec4_t)+sizeof(vec2_t)+sizeof(vec4_t)+sizeof(vec3_t))
#define	TESS_OFFSET_NORMAL		((byte *)(NULL)+sizeof(vec4_t)+sizeof(vec2_t)+sizeof(vec4_t)+sizeof(vec3_t)+sizeof(vec3_t))

// 2D VBO stuff
#define MAX_VERTICES_2D 16384
#define MAX_INDICES_2D MAX_VERTICES_2D * 3
uint16_t ibo_quadString[MAX_INDICES_2D];

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
	vec2_t tc;
	vec4_t color;
}vertex2d_t;

typedef struct {
	vertex2d_t v[4];
}tess2d_t;
tess2d_t tess2d;

typedef struct {
	vertex2d_t v[MAX_VERTICES_2D];
	int numVerts, numSymbols;
	uint64_t handle;
}tess2dArray_t;
tess2dArray_t tess2dArray;


void CL_AddString(int x, int y, int scale, char *s, image_t *inTex);

void R_AddCharsToList(int x, int y, int scale, unsigned char num, image_t *inTex);
void R_Flush2D();
void R_DrawTexturedQuad();

#define VID_CENTER_W (vid.width * 0.5)
#define VID_CENTER_H (vid.height * 0.5)

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
	uint64_t	handle[3];

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
glslProgram_t		*bloomBrightProgram;
glslProgram_t		*bloomFinalProgram;
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
glslProgram_t		*showTrisProgram;

void GL_BindProgram (glslProgram_t *program);
void R_CaptureColorBuffer ();
void R_DrawLightWorld ();
void R_SetupOrthoMatrix(void);



void R_ShowTrisBSP(qboolean bmodel, uint numIndices, float r, float g, float b, glslProgram_t *program);

typedef enum {
	ATT_POSITION,
	ATT_NORMAL,
	ATT_TANGENT,
	ATT_BINORMAL,
	ATT_COLOR,
	ATT_TEX0,
	ATT_TEX1,
	ATT_TEX2,
}
glsl_attrib;

typedef enum {
	ATTF_POS		= BIT(0),
	ATTF_ST0		= BIT(1),
	ATTF_ST1		= BIT(2),
	ATTF_ST2		= BIT(3),
	ATTF_COLOR		= BIT(4),
	ATTF_TANGENT	= BIT(5),
	ATTF_BINORMAL	= BIT(6),
	ATTF_NORMAL		= BIT(7)
} attFlags_t;

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

///  DDS Support
#define DDS_MAKEFOURCC(a, b, c, d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))

// surface description flags
#define DDSF_CAPS			0x00000001l
#define DDSF_HEIGHT			0x00000002l
#define DDSF_WIDTH			0x00000004l
#define DDSF_PITCH			0x00000008l
#define DDSF_PIXELFORMAT	0x00001000l
#define DDSF_MIPMAPCOUNT	0x00020000l
#define DDSF_LINEARSIZE		0x00080000l
#define DDSF_DEPTH			0x00800000l

// pixel format flags
#define DDSF_ALPHAPIXELS	0x00000001l
#define DDSF_FOURCC			0x00000004l
#define DDSF_RGB			0x00000040l
#define DDSF_RGBA			0x00000041l

// our extended flags
#define DDSF_ID_INDEXCOLOR	0x10000000l
#define DDSF_ID_MONOCHROME	0x20000000l

// dwCaps1 flags
#define DDSF_COMPLEX		0x00000008l
#define DDSF_TEXTURE		0x00001000l
#define DDSF_MIPMAP			0x00400000l

#define DDSCAPS2_CUBEMAP	0x00000200

typedef struct {
	unsigned int dwSize;
	unsigned int dwFlags;
	unsigned int dwFourCC;
	unsigned int dwRGBBitCount;
	unsigned int dwRBitMask;
	unsigned int dwGBitMask;
	unsigned int dwBBitMask;
	unsigned int dwABitMask;
} ddsFilePixelFormat_t;

typedef struct
{
	unsigned int dwSize;
	unsigned int dwFlags;
	unsigned int dwHeight;
	unsigned int dwWidth;
	unsigned int dwPitchOrLinearSize;
	unsigned int dwDepth;
	unsigned int dwMipMapCount;
	unsigned int dwReserved1[11];
	ddsFilePixelFormat_t ddspf;
	unsigned int dwCaps1;
	unsigned int dwCaps2;
	unsigned int dwReserved2[3];
} ddsFileHeader_t;


typedef enum DXGI_FORMAT {
	DXGI_FORMAT_UNKNOWN = 0,
	DXGI_FORMAT_R32G32B32A32_TYPELESS = 1,
	DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
	DXGI_FORMAT_R32G32B32A32_UINT = 3,
	DXGI_FORMAT_R32G32B32A32_SINT = 4,
	DXGI_FORMAT_R32G32B32_TYPELESS = 5,
	DXGI_FORMAT_R32G32B32_FLOAT = 6,
	DXGI_FORMAT_R32G32B32_UINT = 7,
	DXGI_FORMAT_R32G32B32_SINT = 8,
	DXGI_FORMAT_R16G16B16A16_TYPELESS = 9,
	DXGI_FORMAT_R16G16B16A16_FLOAT = 10,
	DXGI_FORMAT_R16G16B16A16_UNORM = 11,
	DXGI_FORMAT_R16G16B16A16_UINT = 12,
	DXGI_FORMAT_R16G16B16A16_SNORM = 13,
	DXGI_FORMAT_R16G16B16A16_SINT = 14,
	DXGI_FORMAT_R32G32_TYPELESS = 15,
	DXGI_FORMAT_R32G32_FLOAT = 16,
	DXGI_FORMAT_R32G32_UINT = 17,
	DXGI_FORMAT_R32G32_SINT = 18,
	DXGI_FORMAT_R32G8X24_TYPELESS = 19,
	DXGI_FORMAT_D32_FLOAT_S8X24_UINT = 20,
	DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
	DXGI_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
	DXGI_FORMAT_R10G10B10A2_TYPELESS = 23,
	DXGI_FORMAT_R10G10B10A2_UNORM = 24,
	DXGI_FORMAT_R10G10B10A2_UINT = 25,
	DXGI_FORMAT_R11G11B10_FLOAT = 26,
	DXGI_FORMAT_R8G8B8A8_TYPELESS = 27,
	DXGI_FORMAT_R8G8B8A8_UNORM = 28,
	DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
	DXGI_FORMAT_R8G8B8A8_UINT = 30,
	DXGI_FORMAT_R8G8B8A8_SNORM = 31,
	DXGI_FORMAT_R8G8B8A8_SINT = 32,
	DXGI_FORMAT_R16G16_TYPELESS = 33,
	DXGI_FORMAT_R16G16_FLOAT = 34,
	DXGI_FORMAT_R16G16_UNORM = 35,
	DXGI_FORMAT_R16G16_UINT = 36,
	DXGI_FORMAT_R16G16_SNORM = 37,
	DXGI_FORMAT_R16G16_SINT = 38,
	DXGI_FORMAT_R32_TYPELESS = 39,
	DXGI_FORMAT_D32_FLOAT = 40,
	DXGI_FORMAT_R32_FLOAT = 41,
	DXGI_FORMAT_R32_UINT = 42,
	DXGI_FORMAT_R32_SINT = 43,
	DXGI_FORMAT_R24G8_TYPELESS = 44,
	DXGI_FORMAT_D24_UNORM_S8_UINT = 45,
	DXGI_FORMAT_R24_UNORM_X8_TYPELESS = 46,
	DXGI_FORMAT_X24_TYPELESS_G8_UINT = 47,
	DXGI_FORMAT_R8G8_TYPELESS = 48,
	DXGI_FORMAT_R8G8_UNORM = 49,
	DXGI_FORMAT_R8G8_UINT = 50,
	DXGI_FORMAT_R8G8_SNORM = 51,
	DXGI_FORMAT_R8G8_SINT = 52,
	DXGI_FORMAT_R16_TYPELESS = 53,
	DXGI_FORMAT_R16_FLOAT = 54,
	DXGI_FORMAT_D16_UNORM = 55,
	DXGI_FORMAT_R16_UNORM = 56,
	DXGI_FORMAT_R16_UINT = 57,
	DXGI_FORMAT_R16_SNORM = 58,
	DXGI_FORMAT_R16_SINT = 59,
	DXGI_FORMAT_R8_TYPELESS = 60,
	DXGI_FORMAT_R8_UNORM = 61,
	DXGI_FORMAT_R8_UINT = 62,
	DXGI_FORMAT_R8_SNORM = 63,
	DXGI_FORMAT_R8_SINT = 64,
	DXGI_FORMAT_A8_UNORM = 65,
	DXGI_FORMAT_R1_UNORM = 66,
	DXGI_FORMAT_R9G9B9E5_SHAREDEXP = 67,
	DXGI_FORMAT_R8G8_B8G8_UNORM = 68,
	DXGI_FORMAT_G8R8_G8B8_UNORM = 69,
	DXGI_FORMAT_BC1_TYPELESS = 70,
	DXGI_FORMAT_BC1_UNORM = 71,
	DXGI_FORMAT_BC1_UNORM_SRGB = 72,
	DXGI_FORMAT_BC2_TYPELESS = 73,
	DXGI_FORMAT_BC2_UNORM = 74,
	DXGI_FORMAT_BC2_UNORM_SRGB = 75,
	DXGI_FORMAT_BC3_TYPELESS = 76,
	DXGI_FORMAT_BC3_UNORM = 77,
	DXGI_FORMAT_BC3_UNORM_SRGB = 78,
	DXGI_FORMAT_BC4_TYPELESS = 79,
	DXGI_FORMAT_BC4_UNORM = 80,
	DXGI_FORMAT_BC4_SNORM = 81,
	DXGI_FORMAT_BC5_TYPELESS = 82,
	DXGI_FORMAT_BC5_UNORM = 83,
	DXGI_FORMAT_BC5_SNORM = 84,
	DXGI_FORMAT_B5G6R5_UNORM = 85,
	DXGI_FORMAT_B5G5R5A1_UNORM = 86,
	DXGI_FORMAT_B8G8R8A8_UNORM = 87,
	DXGI_FORMAT_B8G8R8X8_UNORM = 88,
	DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
	DXGI_FORMAT_B8G8R8A8_TYPELESS = 90,
	DXGI_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
	DXGI_FORMAT_B8G8R8X8_TYPELESS = 92,
	DXGI_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
	DXGI_FORMAT_BC6H_TYPELESS = 94,
	DXGI_FORMAT_BC6H_UF16 = 95,
	DXGI_FORMAT_BC6H_SF16 = 96,
	DXGI_FORMAT_BC7_TYPELESS = 97,
	DXGI_FORMAT_BC7_UNORM = 98,
	DXGI_FORMAT_BC7_UNORM_SRGB = 99,
	DXGI_FORMAT_AYUV = 100,
	DXGI_FORMAT_Y410 = 101,
	DXGI_FORMAT_Y416 = 102,
	DXGI_FORMAT_NV12 = 103,
	DXGI_FORMAT_P010 = 104,
	DXGI_FORMAT_P016 = 105,
	DXGI_FORMAT_420_OPAQUE = 106,
	DXGI_FORMAT_YUY2 = 107,
	DXGI_FORMAT_Y210 = 108,
	DXGI_FORMAT_Y216 = 109,
	DXGI_FORMAT_NV11 = 110,
	DXGI_FORMAT_AI44 = 111,
	DXGI_FORMAT_IA44 = 112,
	DXGI_FORMAT_P8 = 113,
	DXGI_FORMAT_A8P8 = 114,
	DXGI_FORMAT_B4G4R4A4_UNORM = 115,
	DXGI_FORMAT_P208 = 130,
	DXGI_FORMAT_V208 = 131,
	DXGI_FORMAT_V408 = 132,
	DXGI_FORMAT_FORCE_UINT = 0xffffffff
} DXGI_FORMAT;

typedef enum D3D10_RESOURCE_DIMENSION {
	D3D10_RESOURCE_DIMENSION_UNKNOWN = 0,
	D3D10_RESOURCE_DIMENSION_BUFFER = 1,
	D3D10_RESOURCE_DIMENSION_TEXTURE1D = 2,
	D3D10_RESOURCE_DIMENSION_TEXTURE2D = 3,
	D3D10_RESOURCE_DIMENSION_TEXTURE3D = 4
} D3D10_RESOURCE_DIMENSION;

typedef struct {
	DXGI_FORMAT              dxgiFormat;
	D3D10_RESOURCE_DIMENSION resourceDimension;
	uint                     miscFlag;
	uint                     arraySize;
	uint                     miscFlags2;
} ddsFileHeaderDXT10_t;

image_t *f16;

image_t* R_LoadDDS(char* texName, uint type);

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

void Sys_CheckWindowsVersion();
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
