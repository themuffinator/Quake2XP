/*=======================================================
Author:				Rich Whitehouse
Web site:			http://www.telefragged.com/thefatal/
=========================================================*/

#include "r_local.h"

//i'm sure there are some duplications of existing quake functionality in here.
//some of this code is ported directly from my model conversion utility (that
//i use to convert to my own engine's format) and i didn't feel like making sure
//all the quake math functions operate the same and changing them and blah.
modelMatrix_t g_identityMatrix =
{
	{ 1.0f, 0.0f, 0.0f },
	{ 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 1.0f },
	{ 0.0f, 0.0f, 0.0f }
};

//standard plane equation formula
void Math_CalcPlaneEq(float *x, float *y, float *z, float *planeEq)
{
	planeEq[0] = x[1] * (y[2] - z[2]) + y[1] * (z[2] - x[2]) + z[1] * (x[2] - y[2]);
	planeEq[1] = x[2] * (y[0] - z[0]) + y[2] * (z[0] - x[0]) + z[2] * (x[0] - y[0]);
	planeEq[2] = x[0] * (y[1] - z[1]) + y[0] * (z[1] - x[1]) + z[0] * (x[1] - y[1]);
	planeEq[3] = -(x[0] * (y[1] * z[2] - z[1] * y[2]) + y[0] * (z[1] * x[2] - x[1] * z[2]) + z[0] * (x[1] * y[2] - y[1] * x[2]));
}

//vector normalize func
float Math_VecNorm(float *v)
{
	float	length, ilength;

	length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	length = (float)sqrt(length);

	if (length)
	{
		ilength = 1 / length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}

	return length;
}

//vector length func
float Math_VecLen(float *a)
{
	return (float)sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
}

//vector copy func
void Math_VecCopy(float *a, float *out)
{
	out[0] = a[0];
	out[1] = a[1];
	out[2] = a[2];
}

//vector subtract func
void Math_VecSub(float *a, float *b, float *out)
{
	out[0] = a[0] - b[0];
	out[1] = a[1] - b[1];
	out[2] = a[2] - b[2];
}

//vector add func
void Math_VecAdd(float *a, float *b, float *out)
{
	out[0] = a[0] + b[0];
	out[1] = a[1] + b[1];
	out[2] = a[2] + b[2];
}

//vector scale func
void Math_VecScale(float *a, float scale)
{
	a[0] *= scale;
	a[1] *= scale;
	a[2] *= scale;
}

//transform a point by a given matrix
void Math_TransformPointByMatrix(modelMatrix_t *matrix, float *in, float *out)
{
	out[0] = matrix->x1[0] * in[0] + matrix->x1[1] * in[1] + matrix->x1[2] * in[2] + matrix->o[0];
	out[1] = matrix->x2[0] * in[0] + matrix->x2[1] * in[1] + matrix->x2[2] * in[2] + matrix->o[1];
	out[2] = matrix->x3[0] * in[0] + matrix->x3[1] * in[1] + matrix->x3[2] * in[2] + matrix->o[2];
}

//transform a point by a given matrix
void Math_TransformPointByMatrix3x3(modelMatrix_t *matrix, float *in, float *out)
{
	out[0] = matrix->x1[0] * in[0] + matrix->x1[1] * in[1] + matrix->x1[2] * in[2];
	out[1] = matrix->x2[0] * in[0] + matrix->x2[1] * in[1] + matrix->x2[2] * in[2];
	out[2] = matrix->x3[0] * in[0] + matrix->x3[1] * in[1] + matrix->x3[2] * in[2];
}

//matrix inverse
void Math_MatrixInverse(modelMatrix_t *in, modelMatrix_t *out)
{
	int i, j;
	float *mat;
	float *srcMat;

	for (i = 0; i < 3; i++)
	{
		if (i == 0)
		{
			mat = out->x1;
		}
		else if (i == 1)
		{
			mat = out->x2;
		}
		else
		{
			mat = out->x3;
		}

		for (j = 0; j < 3; j++)
		{
			if (j == 0)
			{
				srcMat = in->x1;
			}
			else if (j == 1)
			{
				srcMat = in->x2;
			}
			else
			{
				srcMat = in->x3;
			}
			mat[j] = srcMat[i];
		}
	}
	for (i = 0; i < 3; i++)
	{
		if (i == 0)
		{
			mat = out->x1;
		}
		else if (i == 1)
		{
			mat = out->x2;
		}
		else
		{
			mat = out->x3;
		}

		out->o[i] = 0.0f;
		for (j = 0; j < 3; j++)
		{
			out->o[i] -= (mat[j] * in->o[j]);
		}
	}
}

//multiply a matrix by another matrix into out
void Math_MatrixMultiply(modelMatrix_t *in, modelMatrix_t *in2, modelMatrix_t *out)
{
	out->x1[0] = (in->x1[0] * in2->x1[0]) + (in->x1[1] * in2->x2[0]) + (in->x1[2] * in2->x3[0]);
	out->x1[1] = (in->x1[0] * in2->x1[1]) + (in->x1[1] * in2->x2[1]) + (in->x1[2] * in2->x3[1]);
	out->x1[2] = (in->x1[0] * in2->x1[2]) + (in->x1[1] * in2->x2[2]) + (in->x1[2] * in2->x3[2]);
	out->o[0] = (in->x1[0] * in2->o[0]) + (in->x1[1] * in2->o[1]) + (in->x1[2] * in2->o[2]) + in->o[0];

	out->x2[0] = (in->x2[0] * in2->x1[0]) + (in->x2[1] * in2->x2[0]) + (in->x2[2] * in2->x3[0]);
	out->x2[1] = (in->x2[0] * in2->x1[1]) + (in->x2[1] * in2->x2[1]) + (in->x2[2] * in2->x3[1]);
	out->x2[2] = (in->x2[0] * in2->x1[2]) + (in->x2[1] * in2->x2[2]) + (in->x2[2] * in2->x3[2]);
	out->o[1] = (in->x2[0] * in2->o[0]) + (in->x2[1] * in2->o[1]) + (in->x2[2] * in2->o[2]) + in->o[1];

	out->x3[0] = (in->x3[0] * in2->x1[0]) + (in->x3[1] * in2->x2[0]) + (in->x3[2] * in2->x3[0]);
	out->x3[1] = (in->x3[0] * in2->x1[1]) + (in->x3[1] * in2->x2[1]) + (in->x3[2] * in2->x3[1]);
	out->x3[2] = (in->x3[0] * in2->x1[2]) + (in->x3[1] * in2->x2[2]) + (in->x3[2] * in2->x3[2]);
	out->o[2] = (in->x3[0] * in2->o[0]) + (in->x3[1] * in2->o[1]) + (in->x3[2] * in2->o[2]) + in->o[2];
}

//multiply a matrix by another matrix into out
void Math_MatrixMultiply3x3(modelMatrix_t *in, modelMatrix_t *in2, modelMatrix_t *out)
{
	out->x1[0] = (in->x1[0] * in2->x1[0]) + (in->x1[1] * in2->x2[0]) + (in->x1[2] * in2->x3[0]);
	out->x1[1] = (in->x1[0] * in2->x1[1]) + (in->x1[1] * in2->x2[1]) + (in->x1[2] * in2->x3[1]);
	out->x1[2] = (in->x1[0] * in2->x1[2]) + (in->x1[1] * in2->x2[2]) + (in->x1[2] * in2->x3[2]);

	out->x2[0] = (in->x2[0] * in2->x1[0]) + (in->x2[1] * in2->x2[0]) + (in->x2[2] * in2->x3[0]);
	out->x2[1] = (in->x2[0] * in2->x1[1]) + (in->x2[1] * in2->x2[1]) + (in->x2[2] * in2->x3[1]);
	out->x2[2] = (in->x2[0] * in2->x1[2]) + (in->x2[1] * in2->x2[2]) + (in->x2[2] * in2->x3[2]);

	out->x3[0] = (in->x3[0] * in2->x1[0]) + (in->x3[1] * in2->x2[0]) + (in->x3[2] * in2->x3[0]);
	out->x3[1] = (in->x3[0] * in2->x1[1]) + (in->x3[1] * in2->x2[1]) + (in->x3[2] * in2->x3[1]);
	out->x3[2] = (in->x3[0] * in2->x1[2]) + (in->x3[1] * in2->x2[2]) + (in->x3[2] * in2->x3[2]);
}

//calculate crossproduct between 2 vectors
void Math_CrossProduct(const float *v1, const float *v2, float *cross)
{
	cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
	cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
	cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

//calculate dot product of 2 vectors
float Math_DotProduct(const float *v1, const float *v2)
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

//creates a matrix given pos and angles
void Math_CreateMatrixFromOrientation(float *origin, float *angles, modelMatrix_t *mat)
{
	vec3_t axis[3];

	AngleVectors(angles, axis[0], axis[1], axis[2]);
	VectorInverse(axis[1]);

	mat->x1[0] = axis[0][0];
	mat->x2[0] = axis[0][1];
	mat->x3[0] = axis[0][2];

	mat->x1[1] = axis[1][0];
	mat->x2[1] = axis[1][1];
	mat->x3[1] = axis[1][2];

	mat->x1[2] = axis[2][0];
	mat->x2[2] = axis[2][1];
	mat->x3[2] = axis[2][2];

	mat->o[0] = origin[0];
	mat->o[1] = origin[1];
	mat->o[2] = origin[2];
}

//build a rotation matrix
void Math_RotationMatrix(float phi, int axis, modelMatrix_t *mat)
{
	switch (axis)
	{
	case 0: //x
		Math_VecCopy(g_identityMatrix.x1, mat->x1);
		mat->x2[0] = 0.0f;
		mat->x2[1] = cos(phi);
		mat->x2[2] = sin(phi);
		mat->x3[0] = 0.0f;
		mat->x3[1] = -sin(phi);
		mat->x3[2] = cos(phi);
		break;
	case 1: //y
		mat->x1[0] = cos(phi);
		mat->x1[1] = 0.0f;
		mat->x1[2] = sin(phi);
		Math_VecCopy(g_identityMatrix.x2, mat->x2);
		mat->x3[0] = -sin(phi);
		mat->x3[1] = 0.0f;
		mat->x3[2] = cos(phi);
		break;
	case 2: //z
		mat->x1[0] = cos(phi);
		mat->x1[1] = sin(phi);
		mat->x1[2] = 0.0f;
		mat->x2[0] = -sin(phi);
		mat->x2[1] = cos(phi);
		mat->x2[2] = 0.0f;
		Math_VecCopy(g_identityMatrix.x3, mat->x3);
		break;

	default:
		break;
	}

	mat->o[0] = 0.0f;
	mat->o[1] = 0.0f;
	mat->o[2] = 0.0f;
}

//used to orthogonalize a vector
static void Mod_ClosestPointOnLine(float *a, float *b, float *p, float *res)
{
	float dif[3];
	float difx[3];
	float d, t;

	Math_VecSub(p, a, dif);
	Math_VecSub(b, a, difx);
	d = Math_VecLen(difx);
	Math_VecNorm(difx);
	t = Math_DotProduct(difx, dif);
	if (t < 0.0f)
	{
		Math_VecCopy(a, res);
	}
	if (t > d)
	{
		Math_VecCopy(b, res);
	}
	Math_VecScale(difx, t);
	Math_VecAdd(a, difx, res);
}

//orthogonalize vector (tangent/binormal) along normal
static void Mod_Orthogonalize(float *v1, float *v2, float *res)
{
	float v[3];
	float vx[3];
	Math_VecCopy(v1, vx);
	Math_VecScale(vx, -1.0f);
	Mod_ClosestPointOnLine(v1, vx, v2, v);
	Math_VecSub(v2, v, res);
	Math_VecNorm(res);
}

//do the tangent calculations
static void Mod_CreateTangentMatrices(modelMeshObject_t *obj, int numVerts, modelVert_t *verts, int numFaces, modelTriFace_t *faces,
	modelTangent_t *argTangentData, modelTexCoord_t *coords, float *vertNormals)
{
	int i = 0;
	modelTriFace_t *face = faces;
	modelTexCoord_t *coordBase = coords;

	modelTangent_t *tan = argTangentData;

	memset(tan, 0, sizeof(modelTangent_t)*numVerts);

	while (i < numFaces)
	{
		modelVert_t *v1, *v2, *v3;
		modelTexCoord_t *t1, *t2, *t3;
		modelTangent_t *tan1, *tan2, *tan3;
		float *v1use, *v2use, *v3use;
		float planes[3][3];
		int x;

		v1 = (verts + face->a);
		v2 = (verts + face->b);
		v3 = (verts + face->c);

		tan1 = (tan + face->a);
		tan2 = (tan + face->b);
		tan3 = (tan + face->c);

		v1use = &v1->x;
		v2use = &v2->x;
		v3use = &v3->x;

		t1 = (coordBase + face->a);
		t2 = (coordBase + face->b);
		t3 = (coordBase + face->c);

		x = 0;
		while (x < 3)
		{
			float sc1[3], sc2[3];
			sc1[0] = v2use[x] - v1use[x];
			sc1[1] = t2->u - t1->u;
			sc1[2] = t2->v - t1->v;

			sc2[0] = v3use[x] - v1use[x];
			sc2[1] = t3->u - t1->u;
			sc2[2] = t3->v - t1->v;

			Math_CrossProduct(sc1, sc2, planes[x]);
			x++;
		}

		if (planes[0][0] != 0.0f &&
			planes[1][0] != 0.0f &&
			planes[2][0] != 0.0f)
		{ //otherwise would divide by 0, so, looks like we have us here an invalid tangent.
			float dsdt[3];
			float dtdt[3];

			dsdt[0] = -planes[0][1] / planes[0][0];
			dsdt[1] = -planes[1][1] / planes[1][0];
			dsdt[2] = -planes[2][1] / planes[2][0];
			dtdt[0] = -planes[0][2] / planes[0][0];
			dtdt[1] = -planes[1][2] / planes[1][0];
			dtdt[2] = -planes[2][2] / planes[2][0];

			Math_VecAdd(tan1->tangentMatrix.x1, dsdt, tan1->tangentMatrix.x1);
			Math_VecAdd(tan1->tangentMatrix.x2, dtdt, tan1->tangentMatrix.x2);

			Math_VecAdd(tan2->tangentMatrix.x1, dsdt, tan2->tangentMatrix.x1);
			Math_VecAdd(tan2->tangentMatrix.x2, dtdt, tan2->tangentMatrix.x2);

			Math_VecAdd(tan3->tangentMatrix.x1, dsdt, tan3->tangentMatrix.x1);
			Math_VecAdd(tan3->tangentMatrix.x2, dtdt, tan3->tangentMatrix.x2);
		}

		tan1->numNormals++;
		tan2->numNormals++;
		tan3->numNormals++;

		face++;
		i++;
	}

	i = 0;
	while (i < numVerts)
	{
		float *vNorm;

		tan->tangentMatrix.x1[0] /= tan->numNormals;
		tan->tangentMatrix.x1[1] /= tan->numNormals;
		tan->tangentMatrix.x1[2] /= tan->numNormals;

		tan->tangentMatrix.x2[0] /= tan->numNormals;
		tan->tangentMatrix.x2[1] /= tan->numNormals;
		tan->tangentMatrix.x2[2] /= tan->numNormals;


		Math_VecNorm(tan->tangentMatrix.x1);
		Math_VecNorm(tan->tangentMatrix.x2);

		vNorm = vertNormals + (i * 3);

		Math_VecCopy(vNorm, tan->tangentMatrix.x3);

		Mod_Orthogonalize(tan->tangentMatrix.x3, tan->tangentMatrix.x1, tan->tangentMatrix.x1);
		Mod_Orthogonalize(tan->tangentMatrix.x3, tan->tangentMatrix.x2, tan->tangentMatrix.x2);

		tan++;
		i++;
	}
}

//recalculate light vectors etc
void Mod_TangentUpdates(entity_t *e, modelMeshObject_t *obj, float *lightPos)
{
	modelTangent_t *t;
	modelVert_t *verts;
	float *lightVectors;
	int i;

	Mod_CreateTangentMatrices(obj, obj->meshData.numVerts, obj->meshData.vertDataTransformed, obj->meshData.numFaces,
		(modelTriFace_t *)obj->meshData.faceData, obj->meshData.tangents, (modelTexCoord_t *)obj->meshData.uvCoord, obj->meshData.vertNormalsTransformed);

	//now that tangents are updated, calculate light vectors
	t = obj->meshData.tangents;
	verts = obj->meshData.vertDataTransformed;
	lightVectors = obj->meshData.lightVectors;
	i = 0;
	while (i < obj->meshData.numVerts)
	{
		float lightVertDir[3];

		Math_VecSub(lightPos, (float *)verts, lightVertDir);

		lightVectors[0] = -Math_DotProduct(lightVertDir, t->tangentMatrix.x1);
		lightVectors[1] = -Math_DotProduct(lightVertDir, t->tangentMatrix.x2);
		lightVectors[2] = -Math_DotProduct(lightVertDir, t->tangentMatrix.x3);
		Math_VecNorm(lightVectors);

		t++;
		verts++;
		lightVectors += 3;
		i++;
	}
}

#define CHAR_TAB 9 //^^;

//parse through a group for a given key value.
char *ParseMD5GroupForVal(char *group, const char *keyName, int parseSpaces)
{
	static char value[4096];
	int i = 0;

	if (group[i] != '{')
	{ //don't want to be given something that doesn't really start on a group
		return NULL;
	}

	value[0] = 0;
	while (group[i])
	{
		while (group[i] &&
			(group[i] == '{' || group[i] == ' ' || group[i] == '\r' || group[i] == '\n' || group[i] == CHAR_TAB))
		{
			i++;
		}

		if (group[i] && group[i] != '}')
		{
			static char checkKey[4096];
			int j = 0;
			int spaces = 0;

			while (group[i] && group[i] != '\r' && group[i] != '\n' && group[i] != CHAR_TAB)
			{
				if (group[i] == '}')
				{
					return NULL;
				}
				else if (group[i] == ' ')
				{
					spaces++;
					if (spaces > parseSpaces)
					{
						break;
					}
				}

				checkKey[j] = group[i];
				j++;
				i++;
			}
			checkKey[j] = 0;
			if (!stricmp(checkKey, keyName))
			{ //found it!
				if (parseSpaces == -1)
				{ //this means, the caller wants me to return the address of the key
					return &group[i];
				}

				while (group[i] &&
					(group[i] == ' ' || group[i] == '\r' || group[i] == '\n' || group[i] == CHAR_TAB))
				{
					i++;
				}
				if (group[i])
				{ //now we should be on the value, so parse it into the static char.
					int stopOnQuote = 0;
					j = 0;

					if (group[i] == '\"')
					{
						stopOnQuote = 1;
						i++;
					}

					while (group[i] && group[i] != '\r' && group[i] != '\n')
					{
						if (stopOnQuote && group[i] == '\"')
						{
							break;
						}
						value[j] = group[i];
						j++;
						i++;
					}
					value[j] = 0;

					//now we should have it, so return the address of our static.
					return value;
				}
				else
				{
					break;
				}
			}
			else
			{
				while (group[i] && group[i] != '\n' && group[i] != '\r')
				{
					i++;
				}
			}
		}
		else
		{
			break;
		}
		i++;
	}

	return NULL;
}

float *StringToVec(const char *val)
{
	static float v[3];
	sscanf(val, "%f %f %f", &v[0], &v[1], &v[2]);

	return v;
}

//create formatted face data
void Mod_CreateFormattedFaces(modelMeshObject_t *obj)
{
	int x = 0;
	modelTriFaceGLFormat_t *faceForm = obj->meshData.faceDataFormatted;
	modelTriFace_t *face = (modelTriFace_t *)obj->meshData.faceData;

	while (x < obj->meshData.numFaces)
	{
		faceForm->a = face->a;
		faceForm->b = face->b;
		faceForm->c = face->c;

		assert(faceForm->a >= 0 && faceForm->a < obj->meshData.numVerts);
		assert(faceForm->b >= 0 && faceForm->b < obj->meshData.numVerts);
		assert(faceForm->c >= 0 && faceForm->c < obj->meshData.numVerts);

		faceForm++;
		face++;
		x++;
	}
}

//calc vert normals
void Mod_CalculateVertNormals(modelMeshObject_t *obj)
{
	int i = 0;
	int j;
	modelVert_t *verts = (modelVert_t *)obj->meshData.vertData;
	modelVert_t *vert = verts;
	modelTriFace_t *face;
	float *normal = obj->meshData.vertNormals;

	modelVert_t *v;
	byte *normIndex;

	while (i < obj->meshData.numVerts)
	{
		normal[0] = 0.0f;
		normal[1] = 0.0f;
		normal[2] = 0.0f;
		j = 0;
		face = (modelTriFace_t *)obj->meshData.faceData;
		while (j < obj->meshData.numFaces)
		{
			modelVert_t *v1, *v2, *v3;
			v1 = (verts + face->a);
			v2 = (verts + face->b);
			v3 = (verts + face->c);

			if (vert == v1 ||
				vert == v2 ||
				vert == v3)
			{ //this vert is attached to this face, so add its plane into the will-be average
				float planeEq[4];

				Math_CalcPlaneEq(&v1->x, &v2->x, &v3->x, planeEq);
				Math_VecNorm(planeEq);
				Math_VecAdd(normal, planeEq, normal);
			}

			face++;
			j++;
		}
		Math_VecNorm(normal);

		normal += 3;
		vert++;
		i++;
	}

	//copy all the vert normals over the transformed vert normal array, for until we calculate the transformed normals.
	memcpy(obj->meshData.vertNormalsTransformed, obj->meshData.vertNormals, sizeof(modelVert_t)*obj->meshData.numVerts);

	//to be compliant with the quake vertex lighting system
	//this code is adapted from the modelgen code
	normIndex = obj->meshData.vertNormalIndexes;
	v = (modelVert_t *)obj->meshData.vertNormals;
	for (i = 0; i<obj->meshData.numVerts; i++)
	{
		int		j;
		float	maxdot;
		int		maxdotindex;

		maxdot = -999999.0;
		maxdotindex = -1;

		for (j = 0; j<NUM_VERTEX_NORMALS; j++)
		{
			float	dot;

			dot = DotProduct((&v->x), q_byteDirs[j]);
			if (dot > maxdot)
			{
				maxdot = dot;
				maxdotindex = j;
			}
		}

		*normIndex = maxdotindex;
		normIndex++;
		v++;
	}
}


//get the anim joint for a bone
modelAnim_t *Mod_GetAnimJointForBone(modelAnim_t *anim, modelBone_t *bone)
{
	modelAnim_t *a = anim;

	while (a)
	{
		if (a->completed &&
			!stricmp(a->name, bone->name))
		{
			return a;
		}

		a = a->next;
	}

	return NULL;
}

//get the matrix for the given frame on a bone
void Mod_FrameMatrixForBoneAnim(modelBone_t *bone, modelAnim_t *boneAnim, int frame, modelMatrix_t *out)
{
	float *xyz, *ang;
	modelMatrix_t baseMat;
	modelMatrix_t m1, m2, m3, tmp, tmp2;
	float transXYZ[3];
	const int ignoreOriginPos = 1;

	//it would actually be good to precalculate all this into a matrix.
	//however, with my current seperation of animation and model objects,
	//this isn't doable, since the animation object contains no actual
	//hierarchal data.
	//if i were doing this from scratch outside of quake architecture,
	//i would probably bind animations and models together to allow for
	//this. it would also prevent the necessary "animation joint" lookup
	//for every bone transform.

	if (ignoreOriginPos && !bone->eData.parent)
	{ //this is a bit of a hack, because i don't want to deal with models moving out of their bbox gameside
		static float zero[3] = { 0.0f, 0.0f, 0.0f };

		xyz = zero;
	}
	else
	{
		xyz = (boneAnim->xyz + (frame * 3));
	}
	ang = (boneAnim->angles + (frame * 3));

	//we need to use the parent's matrix as our base, since offsets
	//are stored additively. if no parent, identity will do.
	if (bone->eData.parent)
	{
		baseMat = bone->eData.parent->eData.finalMat;
	}
	else
	{
		baseMat = g_identityMatrix;
	}

	//quite frankly, i'm not sure why i have to invert the yaw and roll.
	//but i do. so, bling.
	Math_RotationMatrix((-ang[1])*0.0174532925f, 2, &m1);
	Math_RotationMatrix((ang[0])*0.0174532925f, 1, &m2);
	Math_RotationMatrix((-ang[2])*0.0174532925f, 0, &m3);

	//the origin for the bone needs to be transformed by the parent's
	//matrix (the 3x3 portion only, because bone positions are
	//additive on their own)
	Math_TransformPointByMatrix3x3(&baseMat, xyz, transXYZ);
	Math_VecAdd(baseMat.o, transXYZ, baseMat.o);

	//now multiply the matrix by the new rotation matrix into the
	//output matrix.
	Math_MatrixMultiply(&baseMat, &m1, &tmp);
	Math_MatrixMultiply(&tmp, &m2, &tmp2);
	Math_MatrixMultiply(&tmp2, &m3, out);
}

//transform a bone, making sure its parent is transformed as well.
void Mod_TransformBone(modelMeshObject_t *obj, modelBone_t *bone, modelAnim_t *animRoot, int frame, int oldframe, float lerpAmt)
{
	modelMatrix_t matCur, matLast;
	modelAnim_t *boneAnim;

	if (bone->eData.parent)
	{ //make sure parent is transformed first.
		Mod_TransformBone(obj, bone->eData.parent, animRoot, frame, oldframe, lerpAmt);
	}

	if (bone->eData.renderCount == obj->meshData.renderCountCurrent)
	{ //already transformed
		return;
	}
	bone->eData.renderCount = obj->meshData.renderCountCurrent;

	boneAnim = Mod_GetAnimJointForBone(animRoot, bone);
	if (!boneAnim)
	{ //well then.
		if (bone->eData.parent)
		{
			bone->eData.finalMat = bone->eData.parent->eData.finalMat;
		}
		else
		{
			bone->eData.finalMat = g_identityMatrix;
		}
		return;
	}

	//make sure frame and oldframe are within range
	if (frame >= boneAnim->numKeys)
	{
		frame = boneAnim->numKeys - 1;
	}
	if (frame < 0)
	{
		frame = 0;
	}
	if (oldframe >= boneAnim->numKeys)
	{
		oldframe = boneAnim->numKeys - 1;
	}
	if (oldframe < 0)
	{
		oldframe = 0;
	}

	//lets get the matrices for the current frame and last frame
	Mod_FrameMatrixForBoneAnim(bone, boneAnim, frame, &matCur);
	Mod_FrameMatrixForBoneAnim(bone, boneAnim, oldframe, &matLast);

	//now, lets lerp between them.
	if (!lerpAmt)
	{ //looks like it's totally up to date, so don't lerp at all
		bone->eData.finalMat = matCur;
	}
	else
	{
		modelMatrix_t m;
		float lerpFrac = 1.0f - lerpAmt;

		Math_VecSub(matCur.x1, matLast.x1, m.x1);
		Math_VecSub(matCur.x2, matLast.x2, m.x2);
		Math_VecSub(matCur.x3, matLast.x3, m.x3);
		Math_VecSub(matCur.o, matLast.o, m.o);
		Math_VecScale(m.x1, lerpFrac);
		Math_VecScale(m.x2, lerpFrac);
		Math_VecScale(m.x3, lerpFrac);
		Math_VecScale(m.o, lerpFrac);

		bone->eData.finalMat = matLast;
		Math_VecAdd(bone->eData.finalMat.x1, m.x1, bone->eData.finalMat.x1);
		Math_VecAdd(bone->eData.finalMat.x2, m.x2, bone->eData.finalMat.x2);
		Math_VecAdd(bone->eData.finalMat.x3, m.x3, bone->eData.finalMat.x3);
		Math_VecAdd(bone->eData.finalMat.o, m.o, bone->eData.finalMat.o);
	}
}

//make sure all bones are transformed.
void Mod_TransformBones(entity_t *e, modelMeshObject_t *obj, modelAnim_t *anim)
{
	modelBone_t *b = obj->meshData.bones;
	int numB = 0;
	while (numB < obj->meshData.numBones)
	{
		Mod_TransformBone(obj, b, anim, e->frame, e->oldframe, e->backlerp);

		b++;
		numB++;
	}
}

//create transformed verts for animations etc
void Mod_CreateTransformedVerts(entity_t *e, modelMeshObject_t *obj, modelMeshObject_t *root, model_t *anm)
{
	modelAnim_t *anim;
	modelVert_t *transNormals;
	modelVert_t *firstNormals;
	modelVert_t *v;
	modelVertWInfo_t *vWInfo;
	int i;

	if (!e || !anm || !anm->md5anim)
	{ //just copy the verts across and be done with it.
		modelVert_t *t = obj->meshData.vertDataTransformed;
		modelVert_t *o = (modelVert_t *)obj->meshData.vertData;

		memcpy(t, o, sizeof(modelVert_t)*obj->meshData.numVerts);
		return;
	}

	anim = anm->md5anim;

	//ok, transform the bones.
	Mod_TransformBones(e, obj, anim);

	//now set up the verts
	transNormals = (modelVert_t *)obj->meshData.vertNormalsTransformed;
	firstNormals = (modelVert_t *)obj->meshData.vertNormals;
	v = obj->meshData.vertDataTransformed;
	vWInfo = obj->meshData.vertWInfo;
	i = 0;
	while (i < obj->meshData.numVerts)
	{
		int w = 0;

		v->x = 0.0f;
		v->y = 0.0f;
		v->z = 0.0f;
		if (transNormals)
		{
			transNormals->x = 0.0f;
			transNormals->y = 0.0f;
			transNormals->z = 0.0f;
		}

		while (w < vWInfo->numWeights)
		{
			modelVertWeight_t *weight;
			modelBone_t *weightedBone;
			modelMatrix_t *transMat;
			float p[3];

			weight = obj->meshData.vertWeights + (vWInfo->weightIndex + w);
			weightedBone = root->meshData.bones + weight->boneIndex;

			assert(weightedBone && weightedBone->index == weight->boneIndex && weight->index == (vWInfo->weightIndex + w));

			transMat = &weightedBone->eData.finalMat;

			//you know, it would really make more sense if animations were additive off the
			//initially formed static mesh. what the hell is the point of even having original
			//matrix/position in the bones within the mesh then? i guess it's convenience so
			//the model can be rendered without animations. oh well.
			Math_TransformPointByMatrix(transMat, weight->pos, p);
			Math_VecScale(p, weight->weightFactor);

			Math_VecAdd((float *)v, p, (float *)v);

			if (transNormals)
			{ //this is tricky. extrude a point off the base vert and transform that,
			  //then we know the direction between the extruded transformed point
			  //and normal transformed point is our transformed vertex normal.
				float x[3];

				float extr[3];
				Math_VecAdd((float *)weight->pos, (float *)firstNormals, extr);

				Math_TransformPointByMatrix(transMat, extr, x);
				Math_VecScale(x, weight->weightFactor);

				Math_VecSub(x, p, x);
				Math_VecAdd((float *)transNormals, x, (float *)transNormals);
			}

			w++;
		}

		if (transNormals)
		{
			Math_VecNorm((float *)transNormals);

			transNormals++;
			firstNormals++;
		}

		vWInfo++;
		v++;

		i++;
	}
}

//get bone by name
modelBone_t *Mod_GetBoneByName(int numBones, modelBone_t *boneRoot, const char *name)
{
	modelBone_t *b = boneRoot;
	int i = 0;

	while (i < numBones)
	{
		if (!stricmp(b->name, name))
		{
			return b;
		}

		i++;
		b++;
	}

	return NULL;
}

//fill in parent pointers for quick access while checking for matrix overrides
void Mod_FillBoneParentPtrs(modelMeshObject_t *root)
{
	modelBone_t *b = root->meshData.bones;
	int i = 0;

	while (i < root->meshData.numBones)
	{
		if (b->parentName[0])
		{
			b->eData.parent = Mod_GetBoneByName(root->meshData.numBones, root->meshData.bones, b->parentName);
		}
		else
		{
			b->eData.parent = NULL;
		}

		i++;
		b++;
	}
}

//most of the parsing code here is disgusting, as i ripped it from my command line tool.
//please excuse the mess. ;_;

//parse the md5 and shove it into a modelMeshObject_t
modelMeshObject_t *Mod_LoadMD5Parse(byte *fileBuffer, int bufferLen)
{
	modelMeshObject_t *firstObj = (modelMeshObject_t *)malloc(sizeof(modelMeshObject_t));
	modelMeshObject_t **nObj = &firstObj;
	modelBone_t **bone = &firstObj->meshData.bones;
	char *p = (char *)fileBuffer;
	char *val;
	int bones = 0;
	int i = 0;
	int objectNum = 0;
	int somethingWentWrong = 0;

	memset(firstObj, 0, sizeof(modelMeshObject_t));

	while (i < bufferLen)
	{
		while (p[i] && p[i] != '{')
		{
			i++;
		}

		if (p[i] == '{')
		{
			int openBraces;
			int x = i - 1;

			while (x > 0 && p[x] != '\n' && p[x] != '\r')
			{
				x--;
			}

			if (x >= 0)
			{
				char groupName[1024];
				int j = 0;

				while (p[x] && p[x] != '{')
				{
					groupName[j] = p[x];
					j++;
					x++;
				}
				groupName[j] = 0;

				//now, lets see what kind of object this sucker is.
				if (p[x] == '{')
				{
					if (strstr(groupName, "bone"))
					{ //it's a bone
						if (!firstObj->meshData.numBones)
						{
							modelBone_t *b;

							//allocate room for a new bone to shove data into.
							while (*bone)
							{
								bone = &((*bone)->eData.next);
							}
							*bone = (modelBone_t *)malloc(sizeof(modelBone_t));
							memset(*bone, 0, sizeof(modelBone_t));

							b = *bone;

							val = ParseMD5GroupForVal(&p[x], "name", 0);
							if (val)
							{
								strcpy(b->name, val);
							}
							else
							{
								strcpy(b->name, "unknownName");
							}

							val = ParseMD5GroupForVal(&p[x], "parent", 0);
							if (val)
							{
								strcpy(b->parentName, val);
							}
							else
							{
								b->parentName[0] = 0;
							}

							val = ParseMD5GroupForVal(&p[x], "bindpos", 0);
							if (val)
							{
								float *v = StringToVec(val);
								b->mat.o[0] = v[0];
								b->mat.o[1] = v[1];
								b->mat.o[2] = v[2];
							}
							else
							{
								b->mat.o[0] = g_identityMatrix.o[0];
								b->mat.o[1] = g_identityMatrix.o[1];
								b->mat.o[2] = g_identityMatrix.o[2];
								somethingWentWrong = 1;
							}

							val = ParseMD5GroupForVal(&p[x], "bindmat", 0);
							if (val)
							{
								sscanf(val, "%f %f %f %f %f %f %f %f %f", &b->mat.x1[0], &b->mat.x2[0], &b->mat.x3[0],
									&b->mat.x1[1], &b->mat.x2[1], &b->mat.x3[1], &b->mat.x1[2], &b->mat.x2[2], &b->mat.x3[2]);
							}
							else
							{
								b->mat.x1[0] = g_identityMatrix.x1[0];
								b->mat.x1[1] = g_identityMatrix.x1[1];
								b->mat.x1[2] = g_identityMatrix.x1[2];
								b->mat.x2[0] = g_identityMatrix.x2[0];
								b->mat.x2[1] = g_identityMatrix.x2[1];
								b->mat.x2[2] = g_identityMatrix.x2[2];
								b->mat.x3[0] = g_identityMatrix.x3[0];
								b->mat.x3[1] = g_identityMatrix.x3[1];
								b->mat.x3[2] = g_identityMatrix.x3[2];
								somethingWentWrong = 1;
							}

							b->index = bones;
							bones++;
						}
						else
						{
							Com_Printf("Found a bone out of order!\n");
						}
					}
					else if (strstr(groupName, "mesh"))
					{ //vert, tri, weight data
						modelMeshObject_t *obj;

						//ok, hack, presume this means we allocated all the bones.
						if (bones > 0 && !firstObj->meshData.numBones && firstObj->meshData.bones)
						{
							//note that i am only doing it this way because my tool didn't have to care about efficiently accessing
							//bone data, but since we are parsing into the same format we will be using in game, having the bones
							//aligned is a must. and of course, i am too lazy to rewrite things to do this the right way by just
							//getting the number of bones first off. ^^;
							modelBone_t *boneAllocation, *b, *bTo, *bN;

							firstObj->meshData.numBones = bones;

							boneAllocation = (modelBone_t *)malloc(sizeof(modelBone_t)*firstObj->meshData.numBones);

							//now lets copy them over
							b = firstObj->meshData.bones;
							bTo = boneAllocation;
							while (b)
							{
								memcpy(bTo, b, sizeof(modelBone_t));

								bTo++;
								b = b->eData.next;
							}

							//now free all the old ones
							b = firstObj->meshData.bones;
							while (b)
							{
								bN = b->eData.next;
								free(b);
								b = bN;
							}

							//now, set the pointer on the object to the correct one
							firstObj->meshData.bones = boneAllocation;
						}

						if (!(*nObj))
						{ //no current object ptr is null, make a new one
							*nObj = (modelMeshObject_t *)malloc(sizeof(modelMeshObject_t));
							memset(*nObj, 0, sizeof(modelMeshObject_t));
						}
						obj = *nObj;

						val = ParseMD5GroupForVal(&p[x], "numverts", 0);
						if (val)
						{ //we totally need to know how many verts there are beforehand
							obj->meshData.numVerts = atoi(val);

							val = ParseMD5GroupForVal(&p[x], "numtris", 0);
							if (val)
							{ //need to know tris too
								obj->meshData.numFaces = atoi(val);

								val = ParseMD5GroupForVal(&p[x], "numweights", 0);
								if (val)
								{ //and finally, weights
									modelTexCoord_t *uvCoord;
									modelVertWInfo_t *vWInfo;
									modelTriFace_t *face;
									modelVertWeight_t *weights;
									modelVert_t *verts;
									int v;
									char findStr[256];

									obj->meshData.numWeights = atoi(val);

									//now, let us allocate room for everything.
									obj->meshData.vertData = (byte *)malloc(sizeof(modelVert_t)*obj->meshData.numVerts);
									obj->meshData.vertWInfo = (modelVertWInfo_t *)malloc(sizeof(modelVertWInfo_t)*obj->meshData.numVerts);
									obj->meshData.uvCoord = (byte *)malloc(sizeof(modelTexCoord_t)*obj->meshData.numVerts);
									obj->meshData.faceData = (byte *)malloc(sizeof(modelTriFace_t)*obj->meshData.numFaces);
									obj->meshData.vertWeights = (modelVertWeight_t *)malloc(sizeof(modelVertWeight_t)*obj->meshData.numWeights);

									//extra
									obj->meshData.vertDataTransformed = (modelVert_t *)malloc(sizeof(modelVert_t)*obj->meshData.numVerts);
									obj->meshData.faceDataFormatted = (modelTriFaceGLFormat_t *)malloc(sizeof(modelTriFaceGLFormat_t)*obj->meshData.numFaces);
									obj->meshData.colorPointer = (modelRGBA_t *)malloc(sizeof(modelRGBA_t)*obj->meshData.numVerts);
									obj->meshData.colorPointerVec = (modelRGB_t *)malloc(sizeof(modelRGB_t)*obj->meshData.numVerts);
									obj->meshData.vertNormals = (float *)malloc(sizeof(modelVert_t)*obj->meshData.numVerts);
									obj->meshData.vertNormalsTransformed = (float *)malloc(sizeof(modelVert_t)*obj->meshData.numVerts);
									obj->meshData.vertNormalIndexes = (byte *)malloc(sizeof(byte)*obj->meshData.numVerts);
									obj->meshData.tangents = (modelTangent_t *)malloc(sizeof(modelTangent_t)*obj->meshData.numVerts);
									obj->meshData.lightVectors = (float *)malloc(sizeof(modelVert_t)*obj->meshData.numVerts);


									memset(obj->meshData.vertData, 0, sizeof(modelVert_t)*obj->meshData.numVerts);

									val = ParseMD5GroupForVal(&p[x], "shader", 0);
									if (val)
									{
										strcpy(obj->meshData.skinName, val);
									}
									else
									{
										strcpy(obj->meshData.skinName, "default");
									}
									val = ParseMD5GroupForVal(&p[x], "shader_normal", 0);
									if (val)
									{
										strcpy(obj->meshData.skinNameNormal, val);
									}
									else
									{
										strcpy(obj->meshData.skinNameNormal, "");
									}


									uvCoord = (modelTexCoord_t *)obj->meshData.uvCoord;
									vWInfo = (modelVertWInfo_t *)obj->meshData.vertWInfo;
									v = 0;

									while (v < obj->meshData.numVerts)
									{
										sprintf(findStr, "vert %i", v);
										val = ParseMD5GroupForVal(&p[x], findStr, 1); //fixme: this is a crappy slow way to look up subsequent values
										if (val)
										{
											sscanf(val, "%f %f %i %i", &uvCoord->u, &uvCoord->v, &vWInfo->weightIndex, &vWInfo->numWeights);
										}

										uvCoord++;
										vWInfo++;

										v++;
									}
									Com_Printf("Finished vertex processing for object %i.\n", objectNum);

									face = (modelTriFace_t *)obj->meshData.faceData;
									v = 0;

									while (v < obj->meshData.numFaces)
									{
										sprintf(findStr, "tri %i", v);
										val = ParseMD5GroupForVal(&p[x], findStr, 1); //fixme: this is a crappy slow way to look up subsequent values
										if (val)
										{
											int faceA, faceB, faceC;
											sscanf(val, "%i %i %i", &faceA, &faceB, &faceC);
											face->a = (WORD)faceA;
											face->b = (WORD)faceB;
											face->c = (WORD)faceC;
											face->flag = 0;
										}

										face++;

										v++;
									}
									Com_Printf("Finished tri processing for object %i.\n", objectNum);

									weights = obj->meshData.vertWeights;
									v = 0;

									while (v < obj->meshData.numWeights)
									{
										sprintf(findStr, "weight %i", v);
										val = ParseMD5GroupForVal(&p[x], findStr, 1); //fixme: this is a crappy slow way to look up subsequent values
										if (val)
										{
											sscanf(val, "%i %f %f %f %f", &weights->boneIndex, &weights->weightFactor, &weights->pos[0], &weights->pos[1], &weights->pos[2]);
											weights->index = v;
										}

										weights++;

										v++;
									}
									Com_Printf("Finished weight processing for object %i.\n", objectNum);

									Com_Printf("Creating standard static mesh from vertex weights for object %i...", objectNum);
									verts = (modelVert_t *)obj->meshData.vertData;
									vWInfo = (modelVertWInfo_t *)obj->meshData.vertWInfo;
									v = 0;
									while (v < obj->meshData.numVerts)
									{
										int w;

										verts->x = 0;
										verts->y = 0;
										verts->z = 0;

										w = 0;
										while (w < vWInfo->numWeights)
										{
											modelVertWeight_t *weight;
											modelBone_t *weightedBone;

											weight = obj->meshData.vertWeights + (vWInfo->weightIndex + w);
											weightedBone = firstObj->meshData.bones + weight->boneIndex;

											if (weightedBone &&
												weightedBone->index == weight->boneIndex && weight->index == (vWInfo->weightIndex + w))
											{
												float p[3];
												modelMatrix_t *transMat = &weightedBone->mat;

												Math_TransformPointByMatrix(transMat, weight->pos, p);
												Math_VecScale(p, weight->weightFactor);

												Math_VecAdd((float *)verts, p, (float *)verts);
											}
											else
											{
												Com_Printf("CRITICAL ERROR: Bone/weight index values do not match up!\n");
												somethingWentWrong = 1;
											}

											w++;
										}

										vWInfo++;
										verts++;

										v++;
									}
									Com_Printf("Finished!\n");
								}
								else
								{
									somethingWentWrong = 1;
								}
							}
							else
							{
								somethingWentWrong = 1;
							}
						}
						else
						{
							somethingWentWrong = 1;
						}

						nObj = &(*nObj)->next;
						objectNum++;
					}
				}
			}

			openBraces = 0;
			while (p[i] != '}' || openBraces > 0)
			{
				i++;
				if (p[i] == '{')
				{
					openBraces++;
				}
				else if (p[i] == '}')
				{
					openBraces--;
				}
			}
		}
		else
		{
			break;
		}

		i++;
	}

	if (somethingWentWrong)
	{
		Com_Printf("Errors occured while parsing the md5mesh.\nThe mesh may contain errors.\n");
	}

	//fill in parent pointers for bones
	Mod_FillBoneParentPtrs(firstObj);

	return firstObj;
}

//kill all the mallocs made for the md5
void Mod_FreeUpMD5(modelMeshObject_t *obj)
{
	modelMeshObject_t *n;

	while (obj)
	{
		if (obj->meshData.bones)
		{
			free(obj->meshData.bones);
		}
		if (obj->meshData.vertData)
		{
			free(obj->meshData.vertData);
		}
		if (obj->meshData.vertWInfo)
		{
			free(obj->meshData.vertWInfo);
		}
		if (obj->meshData.uvCoord)
		{
			free(obj->meshData.uvCoord);
		}
		if (obj->meshData.faceData)
		{
			free(obj->meshData.faceData);
		}
		if (obj->meshData.vertWeights)
		{
			free(obj->meshData.vertWeights);
		}
		if (obj->meshData.vertDataTransformed)
		{
			free(obj->meshData.vertDataTransformed);
		}
		if (obj->meshData.faceDataFormatted)
		{
			free(obj->meshData.faceDataFormatted);
		}
		if (obj->meshData.colorPointer)
		{
			free(obj->meshData.colorPointer);
		}
		if (obj->meshData.colorPointerVec)
		{
			free(obj->meshData.colorPointerVec);
		}
		if (obj->meshData.vertNormals)
		{
			free(obj->meshData.vertNormals);
		}
		if (obj->meshData.vertNormalsTransformed)
		{
			free(obj->meshData.vertNormalsTransformed);
		}
		if (obj->meshData.vertNormalIndexes)
		{
			free(obj->meshData.vertNormalIndexes);
		}
		if (obj->meshData.tangents)
		{
			free(obj->meshData.tangents);
		}
		if (obj->meshData.lightVectors)
		{
			free(obj->meshData.lightVectors);
		}

		n = obj->next;
		free(obj);
		obj = n;
	}
}

//free up md5anim allocations
void Mod_FreeUpMD5Anim(modelAnim_t *anm)
{
	modelAnim_t *n;
	while (anm)
	{
		if (anm->xyz)
		{
			free(anm->xyz);
		}
		if (anm->angles)
		{
			free(anm->angles);
		}

		n = anm->next;
		free(anm);
		anm = n;
	}
	return;
}

//interface with q2's model poop, to call my md5 model poop
int Mod_AllocateMD5Mesh(model_t *mod, byte *buf, int len)
{
	modelMeshObject_t *root = Mod_LoadMD5Parse(buf, len);
	modelMeshObject_t *obj;
	if (!root)
	{
		return 0;
	}

	//precalculate some stuff
	obj = root;
	while (obj)
	{
		Mod_CreateFormattedFaces(obj);
		Mod_CreateTransformedVerts(NULL, obj, root, NULL);
		Mod_CalculateVertNormals(obj);
		obj = obj->next;
	}

	mod->type = mod_md5;
	mod->md5 = root;
	return 1;
}

//see if one exists by said name
modelAnim_t *Mod_GetAnimObject(modelAnim_t *root, const char *name)
{
	modelAnim_t *a = root;
	while (a)
	{
		if (!stricmp(a->name, name))
		{
			return a;
		}
		a = a->next;
	}

	return NULL;
}

//allocate or find unused object from root
modelAnim_t *Mod_AllocNewAnimObj(modelAnim_t *root)
{
	modelAnim_t **a = &root;
	while (*a)
	{
		if (!(*a)->inuse)
		{ //not in use, so just use this.
			return *a;
		}

		a = &(*a)->next;
	}

	//if we got here, then we've reached the end of the chain, so malloc a new one.
	*a = (modelAnim_t *)malloc(sizeof(modelAnim_t));
	memset(*a, 0, sizeof(modelAnim_t));

	return *a;
}

//this.. sucks, kind of. but it would be a pain to precalculate the number of keys for
//a bone based on all the channels, so i'm just allocating a set amount, for now.
#define MAX_ANIM_KEYS	4096

//parse all the keys in a group to a float buffer.
int ParseMD5AnimKeys(char *parseGroup, float *outBuf, modelAnim_t *joint, int attribKeys, int rangeStart, int keySize)
{
	int i = 0;
	char keyVal[256];
	char *keyAdd = ParseMD5GroupForVal(parseGroup, "keys", -1);
	float lastKey;

	if (!keyAdd)
	{
		Com_Printf("Group parse error on joint %s.\n", joint->name);
		return 0;
	}

	keyAdd++;

	while (keyAdd[i] && keyAdd[i] != ' ' &&
		keyAdd[i] != '\r' && keyAdd[i] != '\n' &&
		keyAdd[i] != CHAR_TAB)
	{
		i++;
	}

	if (keyAdd[i])
	{ //kk, this should be the beginning of the keys themselves
		int j = i;
		i = 0;
		while (i < attribKeys)
		{
			int n = 0;
			while (keyAdd[j] &&
				(keyAdd[j] == ' ' ||
					keyAdd[j] == '\r' ||
					keyAdd[j] == '\n' ||
					keyAdd[j] == CHAR_TAB))
			{
				j++;
			}
			//this should be the start of the key
			while (keyAdd[j] &&
				(keyAdd[j] != ' ' &&
					keyAdd[j] != '\r' &&
					keyAdd[j] != '\n' &&
					keyAdd[j] != CHAR_TAB))
			{
				keyVal[n] = keyAdd[j];
				n++;
				j++;
			}
			keyVal[n] = 0;

			*outBuf = atof(keyVal);
			outBuf += keySize;
			i++;
		}
	}

	lastKey = *(outBuf - keySize);

	//fill in the rest with the last value... this is a little bad, and is some of the logic
	//that would have to be reworked if i did the attribute allocation the right way.
	while (i < (MAX_ANIM_KEYS - rangeStart))
	{
		*outBuf = lastKey;
		outBuf += keySize;

		i++;
	}

	return 1;
}

//parse the contents of the md5anim
modelAnim_t *Mod_LoadMD5AnimParse(byte *buf, int len)
{
	modelAnim_t *root = (modelAnim_t *)malloc(sizeof(modelAnim_t));
	char *p = (char *)buf;
	char *val;
	int i = 0;

	memset(root, 0, sizeof(modelAnim_t));

	while (i < len)
	{
		while (p[i] && p[i] != '{')
		{
			i++;
		}

		if (p[i] == '{')
		{
			int openBraces;
			int x = i - 1;

			while (x > 0 && p[x] != '\n' && p[x] != '\r')
			{
				x--;
			}

			if (x >= 0)
			{
				char groupName[1024];
				int j = 0;

				while (p[x] && p[x] != '{')
				{
					groupName[j] = p[x];
					j++;
					x++;
				}
				groupName[j] = 0;

				//now, lets see what kind of object this sucker is.
				if (p[x] == '{')
				{
					if (strstr(groupName, "channel"))
					{ //alright then...
						val = ParseMD5GroupForVal(&p[x], "joint", 0);
						if (val)
						{
							modelAnim_t *joint;
							int rangeStart;
							const int keySize = sizeof(float) * 3;

							joint = Mod_GetAnimObject(root, val);
							if (!joint)
							{ //not found, so make a new one
								joint = Mod_AllocNewAnimObj(root);
								assert(joint); //should never fail

								strcpy(joint->name, val);
								joint->inuse = 1;

								joint->xyz = (float *)malloc(keySize*MAX_ANIM_KEYS);
								memset(joint->xyz, 0, keySize*MAX_ANIM_KEYS);
								joint->angles = (float *)malloc(keySize*MAX_ANIM_KEYS);
								memset(joint->angles, 0, keySize*MAX_ANIM_KEYS);

								joint->startTime = 99999; //hack (obviously)

														  //just get the framerate once. if it changes between tracks for one bone, then.. blargh.
								val = ParseMD5GroupForVal(&p[x], "framerate", 0);
								if (val)
								{
									joint->frameRate = atof(val);
								}
								//Com_Printf("Created bone %s.\n", joint->name);
							}

							val = ParseMD5GroupForVal(&p[x], "starttime", 0);
							if (val)
							{
								float fVal = atof(val);
								if (fVal < joint->startTime)
								{ //before the current startTime, so use it instead.
									joint->startTime = fVal;
								}
							}
							val = ParseMD5GroupForVal(&p[x], "endtime", 0);
							if (val)
							{
								float fVal = atof(val);
								if (fVal > joint->endTime)
								{ //after the current endTime, so use it instead.
									joint->endTime = fVal;
								}
							}
							val = ParseMD5GroupForVal(&p[x], "range", 0);
							if (val)
							{
								char findNextPart[512];
								int l = 0;

								while (val[l] && val[l] != '\n' && val[l] != '\r' && val[l] != ' ')
								{
									l++;
								}

								if (val[l] == ' ')
								{ //should always land on a space, since this should be a two-part arg, but just in case, we check.
									val[l] = 0;
									rangeStart = atoi(val);
									sprintf(findNextPart, "range %s", val);
									val = ParseMD5GroupForVal(&p[x], findNextPart, 1);
									if (val)
									{
										joint->rangeEnd = atoi(val);

										val = ParseMD5GroupForVal(&p[x], "keys", 0);
										if (val)
										{
											int attribKeys = atoi(val);

											if (attribKeys > joint->numKeys)
											{ //joint's numKeys keeps the highest number of keys in any channel for the joint
												joint->numKeys = attribKeys;
											}

											if (joint->numKeys < MAX_ANIM_KEYS)
											{
												val = ParseMD5GroupForVal(&p[x], "attribute", 0);
												if (val)
												{ //ok, got the attribute, now we know where we should be parsing to.
													float *parseFloatTo = NULL;

													if (!stricmp(val, "x"))
													{
														parseFloatTo = (joint->xyz);
													}
													else if (!stricmp(val, "y"))
													{
														parseFloatTo = (joint->xyz + 1);
													}
													else if (!stricmp(val, "z"))
													{
														parseFloatTo = (joint->xyz + 2);
													}
													else if (!stricmp(val, "pitch"))
													{
														parseFloatTo = (joint->angles); //PITCH == 0
													}
													else if (!stricmp(val, "yaw"))
													{
														parseFloatTo = (joint->angles + 1); //YAW == 1
													}
													else if (!stricmp(val, "roll"))
													{
														parseFloatTo = (joint->angles + 2); //ROLL == 2
													}

													if (parseFloatTo)
													{ //ok, we got something to parse to, so lets finally parse the keys.
													  //skip to the start of the range for this guy
														parseFloatTo += (rangeStart * 3);

														if (ParseMD5AnimKeys(&p[x], parseFloatTo, joint, attribKeys, rangeStart, 3))
														{
															joint->completed = 1; //so we know this bone contains data we can really use
														}
													}
												}
											}
											else
											{ //fixme, maybe if i'm gonna keep using my lazy method, at least try re-allocating the keys?
											  //the reason it's difficult is because numKeys can vary from attribute to attribute on a
											  //single joint/bone, and assumptions are made about ranges, so reallocating even keeping
											  //the same data causes some extra logic issues.
												Com_Printf("WARNING: keys for bone %s exceed MAX_ANIM_KEYS!\n", joint->name);
											}
										}
									}
								}
							}
						}
					}
				}
			}

			openBraces = 0;
			while (p[i] != '}' || openBraces > 0)
			{
				i++;
				if (p[i] == '{')
				{
					openBraces++;
				}
				else if (p[i] == '}')
				{
					openBraces--;
				}
			}
		}
		else
		{
			break;
		}

		i++;
	}

	return root;
}

//animation instances are stored as models, purely for convenience.
int Mod_AllocateMD5Anim(model_t *mod, byte *buf, int len)
{
	modelAnim_t *root = Mod_LoadMD5AnimParse(buf, len);
	if (!root)
	{
		return 0;
	}

	mod->type = mod_md5anim;
	mod->md5anim = root;
	return 1;
}
