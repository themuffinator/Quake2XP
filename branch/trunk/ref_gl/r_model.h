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

#define MAX_LIGHTS 4096
#define MAX_SHADOWLIHGTS 1024

typedef struct					// WORLDSHADOWTEST
{
	vec3_t origin;
	float intensity;
	void *surf;
	vec3_t spotlight;
	vec3_t color;
	vec3_t radius;
	byte vis[MAX_MAP_LEAFS / 8];
	int area;
} worldLight_t;

worldLight_t r_worldLights[MAX_LIGHTS];
int r_numWorldLights;


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
	qboolean ignore;
	byte vis[MAX_MAP_LEAFS / 8];
	int area;

} flare_t;
int r_numflares;
int r_numIgnoreflares;
flare_t r_flares[MAX_FLARES];
#define MAX_FLARES_VERTEX MAX_FLARES*4


byte			viewvis[MAX_MAP_LEAFS/8];

typedef struct shadowlight_s {

	vec3_t	origin, mins, maxs;		//position of light source

	vec3_t	baseColor, color;		//light color, animated color

	float	radius;		//radius of light source (doesn't light anyting out of that circle)

						//so we can "clip" our shadowvolumes against that circle

	float	brightness;

	qboolean visible;	//light is "visible" this frame

	qboolean isStatic;	//light is static and has a precalc volume

	qboolean castShadow;//lights casts shadows

	qboolean halo;		//light has a halo

//	mleaf_t	*leaf;		//leaf this light is in

	byte vis[MAX_MAP_LEAFS/8];//redone pvs for light, only poly's in nodes

	byte entvis[MAX_MAP_LEAFS/8];//original pvs a light origin

	msurface_t	**visSurf; //the surfaces that should cast shadows for this light (only when static)

	int		style;

	entity_t *owner;

	
	int		filtercube;	//texture object of this light's cubemap filter

	vec3_t	angles; //angles of the cubemap filter

	float	rspeed; //rotation speed of cube map;

	float	cubescale; //scale factor of cube map;

	float	haloalpha; //alpha of halo

	vec3_t	oldlightorigin; 
	qboolean ignore;
	int area;

} shadowlight_t;

#define	MAXSHADOWLIGHTS		1024	//256 //Maximum number of (client side) lights in a map				/// FIXME: СѓРјРµРЅСЊС€РёС‚СЊ РґР»СЏ СЃРїРµС†.РєР°СЂС‚ РґР»СЏ РґР°РЅРЅРѕРіРѕ РґРІРёР¶РєР°!
#define MAXUSEDSHADOWLIGHS	128		//64  //Maximum number of lights that can be used in a single frame	/// FIXME: СѓРјРµРЅСЊС€РёС‚СЊ РґР»СЏ СЃРїРµС†.РєР°СЂС‚ РґР»СЏ РґР°РЅРЅРѕРіРѕ РґРІРёР¶РєР°!


int numShadowLights;
int numStaticShadowLights;
int numUsedShadowLights; //number of shadow lights acutally drawn this frame

shadowlight_t shadowlights[MAXSHADOWLIGHTS];
shadowlight_t *usedshadowlights[MAXUSEDSHADOWLIGHS];
shadowlight_t *currentshadowlight;

shadowlight_t* AllocShadowLight(void);
void R_InitShadowsForFrame(void);
/*
==============================================================================

BRUSH MODELS

==============================================================================
*/
typedef struct {
	vec3_t color;				// ���� ������
	vec3_t origin;				// for bbox
	vec3_t origin2;				// for bbox
	float density;
	float minmax[6];
} fog_t;

#define  MAX_FOG_AREAS 64		// ������������ ���������� ��������
								// �������� �� �����
int numFogs;
fog_t fog_infos[MAX_FOG_AREAS];
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
	int nummarksurfaces;
} mleaf_t;


//===================================================================

//
// Whole model
//


typedef enum { mod_bad, mod_brush, mod_sprite, mod_alias,
		mod_lensflare, mod_p_shadow } modtype_t;


// typedef enum {mod_bad, mod_brush, mod_sprite, mod_alias } modtype_t;

typedef struct
{
	float s;
	float t;
} fstvert_t;


typedef struct {
	int n[3];
} neighbors_t;

//char *COM_Parse (char **data_p);


/*
====================================================================

  VERTEX BUFFERS

====================================================================
*/

typedef struct model_s {
	char name[MAX_QPATH];

	int registration_sequence;

	modtype_t type;
	int numframes;

	int flags;

	//
	// volume occupied by the model graphics
	//
	vec3_t mins, maxs, center;
	float radius;

	//
	// solid volume for clipping
	//
	qboolean clipbox;
	vec3_t clipmins, clipmaxs;
	
	//
	// brush model
	//
	int firstmodelsurface, nummodelsurfaces;
	int lightmap;				// only for submodels

	int numsubmodels;
	mmodel_t *submodels;

	int numplanes;
	cplane_t *planes;

	int numleafs;				// number of visible leafs, not counting 0
	mleaf_t *leafs;

	int numvertexes;
	mvertex_t *vertexes;

	int numedges;
	medge_t *edges;

	int numnodes;
	int firstnode;
	mnode_t *nodes;

	int numtexinfo;
	mtexinfo_t *texinfo;

	int numsurfaces;
	msurface_t *surfaces;

	int numsurfedges;
	int *surfedges;

	int nummarksurfaces;
	msurface_t **marksurfaces;
	
	int lightmap_scale;
	qboolean deluxeMapping;

	dvis_t *vis;

	byte *lightdata;


	// for alias models and skins
	image_t *skins[MAX_MD2SKINS];
	image_t *skins_normal[MAX_MD2SKINS];
	image_t *skins_specular[MAX_MD2SKINS];
	image_t	*glowtexture[MAX_MD2SKINS];
	image_t	*skin_env[MAX_MD2SKINS];
	
	int extradatasize;
	void *extradata;
	int triangles[MAX_TRIANGLES];
	float	*st;
	neighbors_t *neighbors;
	
	float ambient;
	float diffuse;
	float specular;
	float alphaShift;
	float glowCfg[3];
	float envScale;
	float modelScale;
	qboolean noselfshadow;
	qboolean envmap;

	byte		*normals;
	byte		*binormals;
	byte		*tangents;

	int			*indexArray;
	int			numIndices;

	int			*indexArray2;
	int			numIndices2;

//	struct		vertCache_s	*vbo_st;
	int memorySize;

} model_t;



#define SHELL_SCALE		        0.5F
#define WEAPON_SHELL_SCALE		0.2F


//============================================================================

void Mod_Init(void);
void Mod_ClearAll(void);
model_t *Mod_ForName(char *name, qboolean crash);
mleaf_t *Mod_PointInLeaf(float *p, model_t * model);
byte *Mod_ClusterPVS(int cluster, model_t * model);

void Mod_Modellist_f(void);

void *Hunk_Begin(int maxsize, char *name);
void *Hunk_Alloc(int size);
int Hunk_End(void);
void Hunk_Free(void *base);

void Mod_FreeAll(void);
void Mod_Free(model_t * mod);
