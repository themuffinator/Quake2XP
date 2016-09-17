
typedef struct modelMatrix_s
{
	float				x1[3];
	float				x2[3];
	float				x3[3];
	float				o[3];
} modelMatrix_t;

typedef struct modelTangent_s
{
	int					numNormals;
	modelMatrix_t		tangentMatrix;
} modelTangent_t;

typedef struct modelVert_s
{
	float				x;
	float				y;
	float				z;
} modelVert_t;

typedef struct modelVertWInfo_s
{
	int					weightIndex;
	int					numWeights;
} modelVertWInfo_t;

typedef struct modelTexCoord_s
{
	float				u;
	float				v;
} modelTexCoord_t;

typedef struct modelTriFace_s
{
	WORD				a;
	WORD				b;
	WORD				c;
	WORD				flag;
} modelTriFace_t;

typedef struct modelTriFaceGLFormat_s
{
	int					a;
	int					b;
	int					c;
} modelTriFaceGLFormat_t;

typedef struct modelRGBA_s
{
	float				rgba[4];
} modelRGBA_t;


typedef struct modelRGB_s
{
	float				rgb[3];
} modelRGB_t;

typedef struct modelVertWeight_s
{
	int					index;
	int					boneIndex;
	float				weightFactor;
	float				pos[3];
} modelVertWeight_t;

typedef struct modelBone_s modelBone_t;
typedef struct modelBoneExData_s
{
	modelBone_t			*parent;
	modelBone_t			*next;
	modelMatrix_t		finalMat;

	int					renderCount;
} modelBoneExData_t;

typedef struct modelBone_s
{
	int					index;
	char				name[32];

	modelMatrix_t		mat;
	char				parentName[32];

	modelBoneExData_t	eData;
} modelBone_t;

typedef struct modelTriMesh_s
{
	int							numVerts;
	byte						*vertData;
	int							numFaces;
	byte						*faceData;
	float						*vertNormals;
	float						*vertNormalsTransformed;
	byte						*vertNormalIndexes;
	char						skinName[1024];
	char						skinNameNormal[1024];

	int							numWeights;
	modelVertWeight_t			*vertWeights;
	int							numBones;
	modelBone_t					*bones;

	modelVertWInfo_t			*vertWInfo;

	byte						*faceMatList;
	byte						*uvCoord;
	byte						*smoothingGrp;
	byte						*coordOffset;

	image_t						*skin;
	image_t						*skin_normal;

	//extra
	modelVert_t					*vertDataTransformed;
	modelTriFaceGLFormat_t		*faceDataFormatted;
	modelRGBA_t					*colorPointer;
	modelRGB_t					*colorPointerVec;
	modelTangent_t				*tangents;
	float						*lightVectors;

	int							renderCountCurrent;
} modelTriMesh_t;

typedef struct modelHierData_s
{
	WORD				flag1;
	WORD				flag2;
	WORD				father;
} modelHierData_t;

typedef struct modelMeshObject_s modelMeshObject_t;
typedef struct modelMeshObject_s
{
	char						*objName;
	modelTriMesh_t				meshData;
	modelHierData_t				*hierarchy;
	WORD						hierarchyPos;
	float						*pivotPoint;

	modelMeshObject_t			*parent;
	modelMeshObject_t			*next;
} modelMeshObject_t;

typedef struct modelAnim_s modelAnim_t;
typedef struct modelAnim_s
{
	char						name[32];

	float						startTime;
	float						endTime;
	float						frameRate;

	int							rangeEnd;
	int							numKeys;

	float						*xyz;
	float						*angles;

	int							inuse;
	int							completed;

	modelAnim_t					*next;
} modelAnim_t;