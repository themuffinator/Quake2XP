#define MAX_SAMPLES		25			// samples per texel limit

#define	MAX_LSTYLES		256		// overall
#define	MAX_STYLES		32		// per face

#define	SINGLEMAP		(64 * 64 * 4)	// sample map for single face, vanilla QRAD3 value

//#define	DIRECT_LIGHT	3000
#define	DIRECT_LIGHT	3

typedef enum {
	emit_surface,
	emit_point,
	emit_spotlight
} emittype_t;

typedef struct directlight_s
{
	struct directlight_s *next;
	emittype_t	type;

	float		intensity;
	int			style;
	vec3_t		origin;
	vec3_t		color;
	vec3_t		normal;		// for surfaces and spotlights
	float		stopdot;	// for spotlights
} directlight_t;

typedef struct {
	float		*origins;
	float		*directions;	// deluxe

	int			numstyles;
	int			stylenums[MAX_STYLES];

	int			numsamples;
	float		*samples[MAX_STYLES];
} facelight_t;

typedef struct
{
	vec_t	facedist;
	vec3_t	facenormal;

	int		numsurfpt;
	vec3_t	surfpt[SINGLEMAP];

	vec3_t	modelorg;		// for origined bmodels

	vec3_t	texorg;
	vec3_t	worldtotex[2];	// s = (world - texorg) . worldtotex[0]
	vec3_t	textoworld[2];	// world = texorg + s * textoworld[0]

	vec_t	exactmins[2], exactmaxs[2];
	
	int		texmins[2], texsize[2];
	int		surfnum;
	dface_t	*face;
} lightinfo_t;

extern const float	sampleofs[MAX_SAMPLES][2];
extern facelight_t	facelight[MAX_MAP_FACES];
extern directlight_t	*directlights[MAX_MAP_LEAFS];
extern int			numdlights;

void CalcFaceVectors (lightinfo_t *l);
void CalcFaceExtents (lightinfo_t *l);
void GatherSampleLight (vec3_t pos, vec3_t normal, float **styletable, int offset, int mapsize, float lightscale, float *direction);
void AddSampleToPatch (vec3_t pos, vec3_t color, int facenum);
void CalcPoints (lightinfo_t *l, float sofs, float tofs);

/*
=================================================================

  POINT TRIANGULATION

=================================================================
*/

typedef struct triedge_s
{
	int			p0, p1;
	vec3_t		normal;
	vec_t		dist;
	struct triangle_s	*tri;
} triedge_t;

typedef struct triangle_s
{
	triedge_t	*edges[3];
} triangle_t;

#define	MAX_TRI_POINTS		1024
#define	MAX_TRI_EDGES		(MAX_TRI_POINTS*6)
#define	MAX_TRI_TRIS		(MAX_TRI_POINTS*2)

typedef struct
{
	int			numpoints;
	int			numedges;
	int			numtris;
	dplane_t	*plane;
	triedge_t	*edgematrix[MAX_TRI_POINTS][MAX_TRI_POINTS];
	struct patch_s		*points[MAX_TRI_POINTS];
	triedge_t	edges[MAX_TRI_EDGES];
	triangle_t	tris[MAX_TRI_TRIS];
} triangulation_t;

extern int			facelinks[MAX_MAP_FACES];
extern int			planelinks[2][MAX_MAP_PLANES];

qboolean PointInTriangle (const vec3_t point, triangle_t *t);
