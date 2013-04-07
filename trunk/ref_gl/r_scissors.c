/*
Copyright (C) 2004-2011 Serge Borodulin aka Berserker (tm)
                         <http://berserker.quakegate.ru>
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
#include "r_local.h"

void SinCos( float radians, float *sine, float *cosine )
{
	_asm
	{
		fld	dword ptr [radians]
		fsincos

		mov edx, dword ptr [cosine]
		mov eax, dword ptr [sine]

		fstp dword ptr [edx]
		fstp dword ptr [eax]
	}
}

void transform_point(float out[4], const float m[16], const float in[4])
{
#define M(row,col)  m[col*4+row]
   out[0] = M(0, 0) * in[0] + M(0, 1) * in[1] + M(0, 2) * in[2] + M(0, 3) * in[3];
   out[1] = M(1, 0) * in[0] + M(1, 1) * in[1] + M(1, 2) * in[2] + M(1, 3) * in[3];
   out[2] = M(2, 0) * in[0] + M(2, 1) * in[1] + M(2, 2) * in[2] + M(2, 3) * in[3];
   out[3] = M(3, 0) * in[0] + M(3, 1) * in[1] + M(3, 2) * in[2] + M(3, 3) * in[3];
#undef M
}


qboolean qgluProject(float objx, float objy, float objz, const float model[16], const float proj[16], const int viewport[4], float *winx, float *winy)
{
   /* matrice de transformation */
   float in[4], out[4], temp;

   /* initilise la matrice et le vecteur a transformer */
   in[0] = objx;
   in[1] = objy;
   in[2] = objz;
   in[3] = 1.0;
   transform_point(out, model, in);
   transform_point(in, proj, out);

   /* d'ou le resultat normalise entre -1 et 1 */
   if (in[3] == 0.0)
      return false;

   temp = 1.0 / in[3];
   in[0] *= temp;
   in[1] *= temp;
   in[2] *= temp;

   /* en coordonnees ecran */
   *winx = viewport[0] + (1 + in[0]) * (float)viewport[2] * 0.5;
   *winy = viewport[1] + (1 + in[1]) * (float)viewport[3] * 0.5;
   return true;
}
/*
=============
R_ProjectSphere

Returns the rectangle the sphere will be in when it is drawn.
FIXME: This is crappy code we draw a "sprite" and project those points
it should be possible to analytically derive a eq.
=============
*/
void R_ProjectSphere (worldShadowLight_t *light, int *rect)
{
	int		i, j;
	float	a;
	vec3_t	v;
	vec3_t	mrad;
	float minx, maxx, miny, maxy;
	float px, py;///, pz;

	for (j=0 ; j<3 ; j++)
		mrad[j] = 2*vpn[j]*light->radius;

	minx = 9999999;
	miny = 9999999;
	maxx = -9999999;
	maxy = -9999999;

	for (i=0 ; i<4 ; i++)
	{
		float s, c;
		a = (float)i * M_PI * 0.5;
		SinCos(a, &s, &c);
		for (j=0 ; j<3 ; j++)
			v[j] = light->origin[j] + (vright[j]*c + vup[j]*s + vpn[j])*light->radius;

		qgluProject(v[0], v[1], v[2], r_world_matrix, r_project_matrix, (int *) r_viewport, &px, &py);

		if (px > maxx) maxx = px;
		if (px < minx) minx = px;
		if (py > maxy) maxy = py;
		if (py < miny) miny = py;

		for (j=0 ; j<3 ; j++)
			v[j] -= mrad[j];

		qgluProject(v[0], v[1], v[2], r_world_matrix, r_project_matrix, (int *) r_viewport, &px, &py);

		if (px > maxx) maxx = px;
		if (px < minx) minx = px;
		if (py > maxy) maxy = py;
		if (py < miny) miny = py;
	}

	rect[0] = (int)minx;
	rect[1] = (int)miny;
	rect[2] = (int)maxx;
	rect[3] = (int)maxy;	
}


void IntersectRayPlane(vec3_t v1, vec3_t v2, cplane_t *plane, vec3_t res)
{
	vec3_t	v;
	float	sect;

	VectorSubtract (v1, v2, v);
	sect = -(DotProduct (plane->normal, v1)-plane->dist) / DotProduct (plane->normal, v);
	VectorScale (v,sect,v);
	VectorAdd (v1, v, res);
}

float   clip[16];

void Mat_Mul_1x4_4x4(vec4_t a, vec4_t result)
{
	int	index_j,	// column index
		index_k;	// row index
	float sum;		// temp used to hold sum of products

	// loop thru columns of b
	for (index_j=0; index_j<4; index_j++)
    {
	    // multiply ith row of a by jth column of b and store the sum
		// of products in the position i,j of result
	    sum = 0;
	    for (index_k=0; index_k<4; index_k++)
		    sum += a[index_k]*clip[index_k*4+index_j];
	    // store result
	    result[index_j] = sum;
    }
}


void addPoint(vec3_t v1, int *rect)
{
	vec4_t	point, res;
	float	px, py, tmp;

	point[0] = v1[0];
	point[1] = v1[1];
	point[2] = v1[2];
	point[3] = 1;
	Mat_Mul_1x4_4x4(point, res);

	tmp = 1.0/res[3];
	px = (res[0]*tmp+1.0) * 0.5;
	py = (res[1]*tmp+1.0) * 0.5;

	px = px * r_viewport[2] + r_viewport[0];
	py = py * r_viewport[3] + r_viewport[1];

	if (px > rect[2]) rect[2] = (int)px;
	if (px < rect[0]) rect[0] = (int)px;
	if (py > rect[3]) rect[3] = (int)py;
	if (py < rect[1]) rect[1] = (int)py;
}


void addEdge(vec3_t v1, vec3_t v2, int *rect)
{
	vec3_t		intersect, v1_, v2_;
	cplane_t	plane;
	qboolean	side1, side2;

	VectorCopy(v1,v1_);
	VectorCopy(v2,v2_);
	VectorCopy(vpn, plane.normal);
	plane.dist = DotProduct(r_origin, vpn) + 5.0;

	//Check edge to frustrum near plane
	side1 = ((DotProduct(plane.normal, v1_) - plane.dist) >= 0);
	side2 = ((DotProduct(plane.normal, v2_) - plane.dist) >= 0);

	if (!side1 && !side2)
		return; //edge behind near plane

	if (!side1 || !side2)
		IntersectRayPlane(v1_,v2_,&plane,intersect);

	if (!side1)
		VectorCopy(intersect,v1_);
	else if (!side2)
		VectorCopy(intersect,v2_);

	addPoint(v1_, rect);
	addPoint(v2_, rect);
}


/*
=============
boxScreenSpaceRect

Returns the screen space rectangle taken by the box.
(Clips the box to the near plane to have correct results even if the box intersects the near plane)
=============
*/

void boxScreenSpaceRect(worldShadowLight_t *light, int *rect)
{
	vec3_t	aaa,iaa,aia,iia,aai,iai,aii,iii;

	VectorSet(aaa, light->maxs[0], light->maxs[1], light->maxs[2]);
	VectorSet(iaa, light->mins[0], light->maxs[1], light->maxs[2]);
	VectorSet(aia, light->maxs[0], light->mins[1], light->maxs[2]);
	VectorSet(iia, light->mins[0], light->mins[1], light->maxs[2]);
	VectorSet(aai, light->maxs[0], light->maxs[1], light->mins[2]);
	VectorSet(iai, light->mins[0], light->maxs[1], light->mins[2]);
	VectorSet(aii, light->maxs[0], light->mins[1], light->mins[2]);
	VectorSet(iii, light->mins[0], light->mins[1], light->mins[2]);

	rect[0] = 9999999;
	rect[1] = 9999999;
	rect[2] = -9999999;
	rect[3] = -9999999;

	addEdge(aaa, iaa, rect);
	addEdge(aaa, aia, rect);
	addEdge(iia, iaa, rect);
	addEdge(iia, aia, rect);
	addEdge(aai, iai, rect);
	addEdge(aai, aii, rect);
	addEdge(iii, iai, rect);
	addEdge(iii, aii, rect);
	addEdge(iai, iaa, rect);
	addEdge(aai, aaa, rect);
	addEdge(iii, iia, rect);
	addEdge(aii, aia, rect);
}

