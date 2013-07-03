/*
Copyright (C) 1997-2001 Id Software, Inc., 2004-2013 Quake2xp Team.

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

float frustumPlanes[6][4];

// this is the slow, general version
int BoxOnPlaneSide22 (vec3_t emins, vec3_t emaxs, struct cplane_s *p)
{
	int		i;
	float	dist1, dist2;
	int		sides;
	vec3_t	corners[2];

	for (i=0 ; i<3 ; i++)
	{
		if (p->normal[i] < 0)
		{
			corners[0][i] = emins[i];
			corners[1][i] = emaxs[i];
		}
		else
		{
			corners[1][i] = emins[i];
			corners[0][i] = emaxs[i];
		}
	}
	dist1 = DotProduct (p->normal, corners[0]) - p->dist;
	dist2 = DotProduct (p->normal, corners[1]) - p->dist;
	sides = 0;
	if (dist1 >= 0)
		sides = 1;
	if (dist2 < 0)
		sides |= 2;

	return sides;
}


/*
 =================
 BoundsAndSphereIntersect
 =================
*/
qboolean BoundsAndSphereIntersect (const vec3_t mins, const vec3_t maxs, const vec3_t origin, float radius){

	if (mins[0] > origin[0] + radius || mins[1] > origin[1] + radius || mins[2] > origin[2] + radius)
		return false;
	if (maxs[0] < origin[0] - radius || maxs[1] < origin[1] - radius || maxs[2] < origin[2] - radius)
		return false;

	return true;
}


/*
=================
R_CullBox

Returns true if the box is completely outside the frustom
=================
*/
qboolean R_CullBox(vec3_t mins, vec3_t maxs)
{
	int i;

	if (r_noCull->value)
		return false;

	for (i = 0; i < 4; i++)
		if (BOX_ON_PLANE_SIDE(mins, maxs, &frustum[i]) == 2)
			return true;
	return false;
}

qboolean R_CullBox_ (vec3_t mins, vec3_t maxs, cplane_t *frust)
{
	int		i;

	for (i=0 ; i<4 ; i++)
		if ( BoxOnPlaneSide22(mins, maxs, &frust[i]) == 2)
			return true;
	return false;
}

/*
=================
R_CullOrigin

Returns true if the origin is completely outside the frustom
=================
*/
qboolean R_CullOrigin(vec3_t origin)
{
	int i;
	
	if (r_noCull->value)
		return false;

	for (i = 0; i < 4; i++)
		if (BOX_ON_PLANE_SIDE(origin, origin, &frustum[i]) == 2)
			return true;
	return false;
}


qboolean R_CullPoint(vec3_t org)
{
	int i;

	for (i = 0; i < 4; i++)
		if (DotProduct(org, frustum[i].normal) > frustum[i].dist)
			return true;

	return false;
}

qboolean R_CullSphere( const vec3_t centre, const float radius)
{
	int		i;
	cplane_t *p;

	if (r_noCull->value)
		return false;

	for (i=0,p=frustum ; i<4; i++,p++)
	{
	if ( DotProduct ( centre, p->normal ) - p->dist <= -radius )
			return true;
	}

	return false;
}

qboolean intersectsBoxPoint(vec3_t mins, vec3_t maxs, vec3_t p)
{
	if (p[0] > maxs[0]) return false;
	if (p[1] > maxs[1]) return false;
	if (p[2] > maxs[2]) return false;
 
	if (p[0] < mins[0]) return false;
	if (p[1] < mins[1]) return false;
	if (p[2] < mins[2]) return false;

	return true;
}



qboolean BoxOutsideFrustum(vec3_t mins, vec3_t maxs)
{
	int		i, j;
	float	dist1, dist2;
	vec3_t	corners[2];


	for (i=0 ; i<6 ; i++)
	{
		for (j=0 ; j<3 ; j++)
		{
			if (frustumPlanes[i][j] < 0)
			{
				corners[0][j] = mins[j];
				corners[1][j] = maxs[j];
			}
			else
			{
				corners[1][j] = mins[j];
				corners[0][j] = maxs[j];
			}
		}

		dist1 = DotProduct (frustumPlanes[i], corners[0]) + frustumPlanes[i][3];
		dist2 = DotProduct (frustumPlanes[i], corners[1]) + frustumPlanes[i][3];
		if (dist1 < 0 && dist2 < 0)
			return true;
	}

	return false;
}

float SphereInFrustum( vec3_t o, float radius )
{
   int p;
   float d = 0;

   for( p = 0; p < 6; p++ )
   {
      d = frustumPlanes[p][0] * o[0] + frustumPlanes[p][1] * o[1] + frustumPlanes[p][2] * o[2] + frustumPlanes[p][3];
      if( d <= -radius )
         return 0;
   }
   return d + radius;
}


int SignbitsForPlane(cplane_t * out)
{
	int bits, j;

	// for fast box on planeside test

	bits = 0;
	for (j = 0; j < 3; j++) {
		if (out->normal[j] < 0)
			bits |= 1 << j;
	}
	return bits;
}


void R_SetFrustum(void)
{
	int i;

	if (r_newrefdef.fov_x == 90) {
		// front side is visible

		VectorAdd(vpn, vright, frustum[0].normal);
		VectorSubtract(vpn, vright, frustum[1].normal);

		VectorAdd(vpn, vup, frustum[2].normal);
		VectorSubtract(vpn, vup, frustum[3].normal);
	} else {
		// Speedup Small Calculations - Eradicator
		RotatePointAroundVector(frustum[0].normal, vup, vpn,
								-(90 - r_newrefdef.fov_x * 0.5));
		RotatePointAroundVector(frustum[1].normal, vup, vpn,
								90 - r_newrefdef.fov_x * 0.5);
		RotatePointAroundVector(frustum[2].normal, vright, vpn,
								90 - r_newrefdef.fov_y * 0.5);
		RotatePointAroundVector(frustum[3].normal, vright, vpn,
								-(90 - r_newrefdef.fov_y * 0.5));
	}

	for (i = 0; i < 4; i++) {
		frustum[i].type = PLANE_ANYZ;
		frustum[i].dist = DotProduct(r_origin, frustum[i].normal);
		frustum[i].signbits = SignbitsForPlane(&frustum[i]);
	}
}

