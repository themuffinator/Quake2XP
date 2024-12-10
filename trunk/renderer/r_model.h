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

/*

d*_t structures are on-disk representations
m*_t structures are in-memory

*/
/*
==============================================================================

WORLD LIGHTS

==============================================================================
*/
msurface_t* interactionRA[MAX_MAP_FACES / 4];
int			numInteractionSurfsRA;

msurface_t* interaction[MAX_MAP_FACES / 4];
int			numInteractionSurfs;

msurface_t *shadowMapSurfaces[MAX_MAP_FACES / 4];
int			numShadowMapSurfaces;

int		r_lightTimeStamp, r_lightTimeStampRA, r_shadowTimeStamp;
int		r_numWorlsShadowLights;

#define INFITITY_VIEW
#define LIGHT_ZNEAR		0.01f
#define SHADOWMAP_SIZE	1024
#define	MAX_SHADOW_LODS	5
#define	Q_INFINITY	1e30f
#define	MAX_WORLD_SHADOW_LIGHTS	2048
#define	EQUAL_EPSILON		0.000001f

typedef struct frustum_s {
	cplane_t	planes[6];		// right, left, top, bottom, near, far
	vec3_t		corners[8];
}frustum_t;

typedef struct worldShadowLight_s {
	vec3_t		origin;
	vec3_t		angles;
	vec3_t		speed;
	vec3_t		color, startColor;
	vec3_t		mins, maxs;
	vec3_t		corners[8];
	vec3_t		lsOrg;

	mat3_t		axis;
	mat4_t		orMatrix;		// same as in entity_t
	mat4_t		mvMatrix;		// world space -> projector space
	mat4_t		projMatrix;
	mat4_t		tpMatrix;		// world space -> 2D texture space in (0; 1) + depth in (-1; 1)
	mat4_t		spotMatrix;
	frustum_t	frustum;		// for interaction culling

	mat4_t		attenMatrix;
	mat4_t		cubeMapMatrix;

	vec3_t		flareOrigin;
	float		flareSize;
	int			flare;

	float		radius[3];

	bool		projector;
	float		coneExp;
	float		hotSpot;
	float		distance;
	vec2_t		fov;

	float		depthBounds[2];
	float		maxRad;
	float		fogDensity;

	int			filter, style, area;
	int			isShadow;
	int			isStatic;
	int			isNoWorldModel;
	int			isAmbient;
	int			isFog;
	int			scissor[4];
	int			start_off;
	uint		numStaticShadowTris;

	bool	spherical;
	bool	castCaustics, castCaustics2;

	cplane_t	frust[6];
	msurface_t	*interaction[MAX_MAP_FACES / 4];
	int			numInteractionSurfs;

	msurface_t	*interactionRA[MAX_MAP_FACES / 4];
	int			numInteractionSurfsRA;

	char		targetname[MAX_QPATH];

	byte		vis[MAX_MAP_LEAFS / 8];

	vertexBuffer_t	*vbo;
	vertexBuffer_t	*ibo;
	vertexObject_t	*vao;

	int	iboNumIndices;
	int	lod;

	struct worldShadowLight_s *next;
	struct worldShadowLight_s *s_next;
	
} worldShadowLight_t;

extern		worldShadowLight_t *currentShadowLight;
extern int	numPreCachedLights;


typedef struct {
	vec3_t origin;
	vec3_t color;
	float outcolor[4];
	float size;
	float sizefull;

	int style;

	float lightIntens;
	msurface_t *surf;
	vec3_t lightsurf_origin;
	bool ignore;

} autoLight_t;

int r_numAutoLights;
int r_numIgnoreAutoLights;
autoLight_t r_lightSpawnSurf[MAX_WORLD_SHADOW_LIGHTS];

#define MAX_FLARE_VERTS 6


byte	viewvis[MAX_MAP_LEAFS / 8];

// global fog struct
typedef struct fog_s {

	int type;
	vec3_t color;
	float bias;
	float density;
}fog_t;
fog_t fog;

/*
==============================================================================

BRUSH MODELS

==============================================================================
*/

//
// in memory representation
//
// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct {
	vec3_t position;
} mvertex_t;

typedef struct {
	vec3_t mins, maxs;
	vec3_t origin;				// for sounds or lights
	float radius;
	int headnode;
	int visleafs;				// not including the solid leaf 0
	int firstface, numfaces;
} mmodel_t;


#define	SIDE_FRONT	0
#define	SIDE_BACK	1
#define	SIDE_ON		2


// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct {
	unsigned short v[2];
	unsigned int cachededgeoffset;
} medge_t;

#define CONTENTS_NODE -1

typedef struct mleaf_s {
	// common with node
	int contents;				// wil be a negative contents number
	int visframe;				// node needs to be traversed if current

	float minmaxs[6];			// for bounding box culling

	struct mnode_s *parent;

	// leaf specific
	int cluster;
	int area;

	msurface_t **firstmarksurface;
	int numMarkSurfaces;
} mleaf_t;


//===================================================================

//
// Whole model
//


typedef enum {
	mod_brush, mod_sprite, 
	mod_alias, mod_alias_md3	
} modtype_t;


// typedef enum {mod_bad, mod_brush, mod_sprite, mod_alias } modtype_t;

typedef struct {
	float s, t;
} md2StVerts_t;

typedef struct {
	vec3_t *normals;
	vec3_t *binormals;
	vec3_t *tangents;
} md2Verts_t;

typedef struct model_s {

	char		name[MAX_QPATH];
	int			registration_sequence;
	int			max_meshes;
	modtype_t	type;
	int			numFrames;
	int			flags;
	uint		hash;
	//
	// volume occupied by the model graphics
	//
	vec3_t		mins, maxs, center;
	vec3_t		bbox[8];
	float		radius;
	//
	// solid volume for clipping
	//
	bool	clipbox;
	vec3_t		clipmins, clipmaxs;
	//
	// brush model
	//
	int			firstModelSurface,
		numModelSurfaces,
		lightmap;				// only for subModels

	int			numSubModels;
	mmodel_t	*subModels;

	int			numPlanes;
	cplane_t	*planes;

	int			numLeafs;				// number of visible leafs, not counting 0
	mleaf_t		*leafs;

	int			numVertexes;
	mvertex_t	*vertexes;

	int			numEdges;
	medge_t		*edges;

	int			numNodes;
	int			firstNode;
	mnode_t		*nodes;

	int			numTexInfo;
	mtexInfo_t	*texInfo;

	int			numSurfaces;
	msurface_t	*surfaces;

	int			numSurfEdges;
	int			*surfEdges;

	int			numMarkSurfaces;
	msurface_t	**markSurfaces;

	dvis_t		*vis;

	int			lightmap_scale;
	byte		*lightData;
	bool	useXPLM;	// 3-vector basis lightmap
	
	// for alias models and skins
	image_t		*albedo		[MAX_MD2SKINS];
	image_t		*normalmap	[MAX_MD2SKINS];
	image_t		*pbr		[MAX_MD2SKINS];
	image_t		*emissive	[MAX_MD2SKINS];
	image_t		*envmap		[MAX_MD2SKINS];

	int			extraDataSize;
	void		*extraData;
	int			triangles[MAX_TRIANGLES];

	int		num_tris;
	uint16_t *indexes;

	float		ambient;
	float		diffuse;
	float		specular;
	float		alphaShift;
	float		glowCfg[3];
	float		envScale;
	float		modelScale;
	bool	noSelfShadow;
	bool	envMap;

	vec3_t	*normals;
	vec3_t	*binormals;
	vec3_t	*tangents;

	uint16_t	*indexArray;
	uint16_t	numIndices;

	vertexBuffer_t	*ibo;
	mat3_t	axis;
} model_t;


#define SHELL_SCALE		        0.5F
#define WEAPON_SHELL_SCALE		0.2F

//============================================================================

void Mod_Init (void);
model_t *Mod_ForName (char *name, bool crash);
byte *Mod_ClusterPVS (int cluster, model_t * model);

void Mod_Modellist_f (void);



void Mod_FreeAll (void);
void Mod_Free (model_t * mod);
