/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/*
Copyright (C) 2004-2024 Quake2xp Team.

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
// shadows.c: shadow functions



#include "r_local.h"

//bsp
int			numShadowSurf, shadowTimeStamp;
msurface_t*	shadow_surfaces[MAX_VERTICES];

//differs from the test for mesh in light bound
bool R_EntityCastShadow() {

	int		i;
	vec3_t	mins, maxs;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return false;

	if (currententity->flags & (RF_SHELL_HALF_DAM | RF_SHELL_GREEN | RF_SHELL_RED |
		RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_GOD |
		RF_TRANSLUCENT | RF_BEAM | RF_WEAPONMODEL | RF_NOSHADOW | RF_DISTORT))
		return false;

	if (!r_playerShadow->integer && (currententity->flags & RF_VIEWERMODEL))
		return false;

	if ((currententity->flags & RF_VIEWERMODEL) && (currentShadowLight->filter == 33))
		return false;

	if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2]) {
		for (i = 0; i < 3; i++) {
			mins[i] = currententity->origin[i] - currentmodel->radius;
			maxs[i] = currententity->origin[i] + currentmodel->radius;
		}
	}
	else {
		VectorAdd(currententity->origin, currententity->model->maxs, maxs);
		VectorAdd(currententity->origin, currententity->model->mins, mins);
	}

	if (currentShadowLight->projector) {

		if (R_CullConeLight(mins, maxs, currentShadowLight->frust))
			return false;

	}
	else if (currentShadowLight->spherical) {

		if (!BoundsAndSphereIntersect(mins, maxs, currentShadowLight->origin, currentShadowLight->radius[0]))
			return false;
	}
	else {

		if (!BoundsIntersect(mins, maxs, currentShadowLight->mins, currentShadowLight->maxs))
			return false;
	}

	if (VectorCompare(currententity->origin, currentShadowLight->origin)) // skip shadows from shell lights
		return false;

	//	if (r_shadows->integer == 1) {
			//	// cull shadow volume out of view frustum
	if (Frustum_CullLocalBoundsProjection(currententity->model->mins, currententity->model->maxs, currententity->origin, currententity->axis, currentShadowLight->origin, 63))
		return false;
	//	}

	if (!InLightVISEntity())
		return false;

	return true;
}

bool R_MarkShadowSurf (msurface_t *surf) {
	cplane_t	*plane;
	glpoly_t	*poly;
	float		dist, lbbox[6], pbbox[6];

	if (surf->texInfo->flags & (SURF_NODRAW)) // rogue hack
		return false;
	
	if (surf->flags & MSURF_ALPHA)
		return false;

	// add sky surfaces to shadow marking
	if (surf->texInfo->flags & (SURF_SKY))
		goto hack;

	if ((surf->texInfo->flags & (SURF_TRANS33 | SURF_TRANS66 | SURF_WARP)) || (surf->flags & MSURF_DRAWTURB))
		return false;
hack:
	plane = surf->plane;
	poly = surf->polys;

	if (poly->shadowTimestamp == shadowTimeStamp)
		return false;

	switch (plane->type) {
		case PLANE_X:
			dist = currentShadowLight->origin[0] - plane->dist;
			break;
		case PLANE_Y:
			dist = currentShadowLight->origin[1] - plane->dist;
			break;
		case PLANE_Z:
			dist = currentShadowLight->origin[2] - plane->dist;
			break;
		default:
			dist = DotProduct (currentShadowLight->origin, plane->normal) - plane->dist;
			break;
	}

	//the normals are flipped when surf_planeback is 1
	if (((surf->flags & MSURF_PLANEBACK) && (dist > 0)) ||
		(!(surf->flags & MSURF_PLANEBACK) && (dist < 0)))
		return false;

	//the normals are flipped when surf_planeback is 1
	if (fabsf (dist) > currentShadowLight->maxRad)
		return false;

	lbbox[0] = currentShadowLight->origin[0] - currentShadowLight->radius[0];
	lbbox[1] = currentShadowLight->origin[1] - currentShadowLight->radius[1];
	lbbox[2] = currentShadowLight->origin[2] - currentShadowLight->radius[2];
	lbbox[3] = currentShadowLight->origin[0] + currentShadowLight->radius[0];
	lbbox[4] = currentShadowLight->origin[1] + currentShadowLight->radius[1];
	lbbox[5] = currentShadowLight->origin[2] + currentShadowLight->radius[2];

	// surface bounding box
	pbbox[0] = surf->mins[0];
	pbbox[1] = surf->mins[1];
	pbbox[2] = surf->mins[2];
	pbbox[3] = surf->maxs[0];
	pbbox[4] = surf->maxs[1];
	pbbox[5] = surf->maxs[2];

	if (currentShadowLight->projector && R_CullConeLight(&pbbox[0], &pbbox[3], currentShadowLight->frust))
		return false;

	if (!BoundsIntersect (&lbbox[0], &lbbox[3], &pbbox[0], &pbbox[3]))
		return false;

	poly->shadowTimestamp = shadowTimeStamp;

	return true;
}

void R_MarkBrushModelShadowSurfaces () {
	int			i;
	msurface_t	*psurf;
	model_t		*clmodel;

	clmodel = currententity->model;
	psurf = &clmodel->surfaces[clmodel->firstModelSurface];

	for (i = 0; i < clmodel->numModelSurfaces; i++, psurf++) {

		if (R_MarkShadowSurf (psurf))
			shadow_surfaces[numShadowSurf++] = psurf;
	}
}

void R_MarkShadowCasting (mnode_t *node) {
	cplane_t	*plane;
	int			c, cluster;
	float		dist;
	msurface_t	**surf;
	mleaf_t		*leaf;

	if (node->contents != -1) {

		//we are in a leaf
		leaf = (mleaf_t *)node;
		cluster = leaf->cluster;

		if (!(currentShadowLight->vis[cluster >> 3] & (1 << (cluster & 7))))
			return;

		surf = leaf->firstmarksurface;

		for (c = 0; c < leaf->numMarkSurfaces; c++, surf++) {

			if (R_MarkShadowSurf ((*surf))) {

				shadow_surfaces[numShadowSurf++] = (*surf);
			}
		}
		return;
	}

	plane = node->plane;
	dist = DotProduct (currentShadowLight->origin, plane->normal) - plane->dist;

	if (dist > currentShadowLight->maxRad) {
		R_MarkShadowCasting (node->children[0]);
		return;
	}

	if (dist < -currentShadowLight->maxRad) {
		R_MarkShadowCasting (node->children[1]);
		return;
	}

	R_MarkShadowCasting (node->children[0]);
	R_MarkShadowCasting (node->children[1]);
}


// ===========
// shadow maps
// ===========

int		SignbitsForPlane(cplane_t *out);
void	R_RecursiveDepthWorldNode(mnode_t *node, vec3_t viewOrg);
void	R_DrawDepthAliasModel();
void	R_DrawDepthMD3Model();
void	GL_DrawDepthBspTris();
void	R_DrawDepthBrushModel();
extern	mat4_t	r_flipMatrix;

extern	int	numDepthSurfaces;
extern	model_t *r_worldmodel;

void R_SetLightFrustum(vec3_t angles, float fov_x, float fov_y) {
	int i;
	vec3_t forward, right, up;

	AngleVectors(angles, forward, right, up);

	RotatePointAroundVector(frustum[0].normal, up, forward,		-(90.0	- fov_x * 0.5));
	RotatePointAroundVector(frustum[1].normal, up, forward,		90.0	- fov_x * 0.5);
	RotatePointAroundVector(frustum[2].normal, right, forward,	90.0	- fov_y * 0.5);
	RotatePointAroundVector(frustum[3].normal, right, forward,	-(90.0	- fov_y * 0.5));
	
	VectorCopy	(forward, frustum[4].normal);
	VectorNegate(forward, frustum[5].normal);

	for (i = 0; i < 6; i++) {
		VectorNormalize(frustum[i].normal);

		frustum[i].type = PLANE_ANYZ;
		frustum[i].dist = DotProduct(currentShadowLight->origin, frustum[i].normal);
		frustum[i].signbits = SignbitsForPlane(&frustum[i]);
	}

	frustum[4].dist = LIGHT_ZNEAR;
	frustum[5].dist -= currentShadowLight->maxRad;
}

void R_DrawShadowWorld(void) {

	int i;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	currentmodel = r_worldmodel;

	GL_BindVAO(vao.depthBsp);

	numDepthSurfaces = 0;
	R_RecursiveDepthWorldNode(r_worldmodel->nodes, currentShadowLight->origin);
	qglUniformMatrix4fv(U_MVP_MATRIX, 1, false, (const float *)r_newrefdef.shadowMVP);
	GL_DrawDepthBspTris();

	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];
		currentmodel = currententity->model;

		if (!currentmodel)
			continue;

		if (currentmodel->type == mod_brush)
			R_DrawDepthBrushModel();
	}

	GL_BindVAO(vao.stream3d);
	GL_BindVBO(vbo.stream3d);
	
	GL_CullFace(GL_FRONT);

	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];
		currentmodel = currententity->model;

		if (!currentmodel)
			continue;

		if (currententity->flags & RF_TRANSLUCENT)
			continue;
		if (currententity->flags & RF_WEAPONMODEL)
			continue;
		if (currententity->flags & (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_GOD))
			continue;
		if (currententity->flags & RF_DISTORT)
			continue;

		if (currentmodel->type == mod_alias)
			R_DrawDepthAliasModel();

		if (currentmodel->type == mod_alias_md3)
			R_DrawDepthMD3Model();
	}
	GL_CullFace(GL_BACK);
}

vec3_t r_cubeFaceDirs[6] = {
	{	0.0,	0.0,	90.0	},	// GL_TEXTURE_CUBE_MAP_POSITIVE_X 
	{	0.0,	180.0, -90.0	},	// GL_TEXTURE_CUBE_MAP_NEGATIVE_X 
	{	0.0,	90.0,   0.0		},	// GL_TEXTURE_CUBE_MAP_POSITIVE_Y
	{	0.0,	270.0,	180.0	},	// GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 
	{ -90.0,	180.0, -90.0	},	// GL_TEXTURE_CUBE_MAP_POSITIVE_Z
	{	90.0,   0.0,	90.0	},	// GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

void R_DrawShadowMaps() {
	vec3_t	angles;
	int		i, vps;
	float	clearDepthBit = 1.0;
	vec4_t	clearColorBit = { 0.0, 0.0, 0.0, 0.0 };
	float	zFar = currentShadowLight->maxRad;
	mat4_t	projMatrix, modelMatrix, m;
	mat3_t	axis;
	vec2_t	fov;

	if (!currentShadowLight->isShadow)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	if (!r_shadows->integer)
		return;

	fov[0] = 90.0;
	fov[1] = 90.0;

	GL_Disable(GL_BLEND);

	vps = 1024 >> currentShadowLight->lod;

	GL_Viewport(0, 0, vps, vps);
	GL_Scissor	(0, 0, vps, vps);
	GL_DepthBoundsTest(0.0, 1.0);
	GL_DepthMask(1);
	GL_Disable(GL_POLYGON_OFFSET_FILL);

	gl_state.shadowMapPass = true;

	qglBindFramebuffer(GL_FRAMEBUFFER, fb.shadowMap[currentShadowLight->lod]->id);
	qglClearBufferfv(GL_COLOR, 0, clearColorBit);

	GL_BindProgram(shadowOmniProgram);

	float scale = 1.f / (zFar - LIGHT_ZNEAR);

	qglUniform1f(U_PARAM_FLOAT_0, (zFar - LIGHT_ZNEAR) / 100.0);

	projMatrix[0][0] = 1.0 / tanf(DEG2RAD(fov[0] * 0.5f));
	projMatrix[0][1] = 0.0;
	projMatrix[0][2] = 0.0;
	projMatrix[0][3] = 0.0;

	projMatrix[1][0] = 0.0;
	projMatrix[1][1] = 1.0 / tanf(DEG2RAD(fov[1] * 0.5f));
	projMatrix[1][2] = 0.0;
	projMatrix[1][3] = 0.0;

	projMatrix[2][0] = 0.0;
	projMatrix[2][1] = 0.0;
	projMatrix[2][2] = /*-(zFar + LIGHT_ZNEAR) * scale;*/zFar / (LIGHT_ZNEAR - zFar);
	projMatrix[2][3] = -1.0;

	projMatrix[3][0] = 0.0;
	projMatrix[3][1] = 0.0;
	projMatrix[3][2] = /*-2.f * zFar * LIGHT_ZNEAR * scale;*/(LIGHT_ZNEAR * zFar) / (LIGHT_ZNEAR - zFar);
	projMatrix[3][3] = 0.0;

	for (i = 0; i < 6; i++) {
				
		VectorSet(angles, r_cubeFaceDirs[i][0], r_cubeFaceDirs[i][1], r_cubeFaceDirs[i][2]);

		AnglesToMat3(angles, axis);
		Mat4_SetupTransform(modelMatrix, axis, currentShadowLight->origin);
		Mat4_AffineInvert(modelMatrix, m);
		Mat4_Multiply(m, r_flipMatrix, modelMatrix);

		Mat4_Multiply(modelMatrix, projMatrix, r_newrefdef.shadowMVP);

		R_SetLightFrustum(angles, fov[0], fov[1]);
		qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, gi.shadowCube[currentShadowLight->lod]->texnum, 0);
		qglClearBufferfv(GL_DEPTH, 0, &clearDepthBit);
		R_DrawShadowWorld();		
	}

	qglBindFramebuffer(GL_FRAMEBUFFER, fb.hdrBase->id);

	GL_Scissor(currentShadowLight->scissor[0], currentShadowLight->scissor[1], currentShadowLight->scissor[2], currentShadowLight->scissor[3]);
	GL_DepthBoundsTest(currentShadowLight->depthBounds[0], currentShadowLight->depthBounds[1]);
	GL_Viewport(r_newrefdef.viewport[0], r_newrefdef.viewport[1], r_newrefdef.viewport[2], r_newrefdef.viewport[3]);

	R_SetFrustum(false);
	GL_DepthMask(0);
	GL_Enable(GL_BLEND);
	GL_Enable(GL_POLYGON_OFFSET_FILL);
	gl_state.shadowMapPass = false;
}