/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
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
#include "r_local.h"

int alphaSurfSort(const msurface_t** a, const msurface_t** b) {
	return	(((*a)->texInfo->albedo->texnum)) - (((*b)->texInfo->albedo->texnum));
}
int waterSurfSort(const msurface_t** a, const msurface_t** b) {
	return	(((*a)->texInfo->albedo->texnum)) - (((*b)->texInfo->albedo->texnum));
}

msurface_t* interactionTranSurf[MAX_MAP_FACES/4];
int			numInteractionTransSurfs;

void R_AddAlphaSurceces(msurface_t* s, uint* indeces, bool update) {
	int i;
	uint numIndices;
	float scroll = 0.0;
	bool scrolling = false;
	int nv = s->polys->numVerts;

	numIndices = *indeces;

	if (update) {

		if (s->texInfo->flags & SURF_FLOWING) {
			scroll = -64 * ((r_newrefdef.time / 500.0) - (int)(r_newrefdef.time / 500.0));

			if (scroll == 0.0)
				scroll = -64.0;

			scrolling = true;
			qglUniform1f(U_SCROLL, scroll);
		}
		else
			qglUniform1f(U_SCROLL, 0.0);

		if (scrolling)
			GL_SetBindlessTexture(U_TMU0, gi.waterDistort->handle);
		else
			GL_SetBindlessTexture(U_TMU0, s->texInfo->normalmap->handle);

		GL_SetBindlessTexture(U_TMU1, s->texInfo->albedo->handle);
		GL_SetBindlessTexture(U_TMU4, s->texInfo->emissive->handle);
	}

	for (i = 0; i < nv - 2; i++) {
		indexArray[numIndices++] = s->baseIndex;
		indexArray[numIndices++] = s->baseIndex + i + 1;
		indexArray[numIndices++] = s->baseIndex + i + 2;
	}
	*indeces = numIndices;
}


void R_DrawAlphaSurfaces() {
	msurface_t* s;
	bool	newTex;
	float		ambientScale = max(r_lightmapScale->value, 0.33);
	uint		oldTex = 0;
	uint		numIndices = 0;

	// setup program
	GL_BindProgram(glassProgram);

	GL_SetBindlessTexture(U_TMU2, gi.hdrBaseInterim->handle);
	GL_SetBindlessTexture(U_TMU3, gi.linearDepth->handle);

	qglUniform1f(U_REFR_DEFORM_MUL, 1.0);
	qglUniformMatrix4fv(U_MVP_MATRIX, 1, false, (const float*)r_newrefdef.modelViewProjectionMatrix);
	qglUniformMatrix4fv(U_MODELVIEW_MATRIX, 1, false, (const float*)r_newrefdef.modelViewMatrix);
	qglUniformMatrix4fv(U_PROJ_MATRIX, 1, false, (const float*)r_newrefdef.projectionMatrix);

	qglUniform1f(U_REFR_THICKNESS0, 150.0);
	qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);
	qglUniform1f(U_AMBIENT_LEVEL, ambientScale);

	float blurScale = 18.88 * ((float)vid.width / 1024.0);
	qglUniform1f(U_PARAM_FLOAT_0, blurScale);

	qsort(r_alphaSurfaces, numAlphaSurfaces, sizeof(msurface_t*), (int(*)(const void*, const void*))alphaSurfSort);

	for (int i = 0; i < numAlphaSurfaces; i++) {

		s = r_alphaSurfaces[i];

		if (s->texInfo->albedo->texnum != oldTex) {
			if (numIndices) {
				GL_DrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
				c_brushTris += numIndices / 3;

				R_ShowTrisBSP(false, numIndices, 1.0, 0.0, 1.0, glassProgram);
				numIndices = 0;
			}
			oldTex = s->texInfo->albedo->texnum;
			newTex = true;
		}
		else
			newTex = false;

		R_AddAlphaSurceces(s, &numIndices, newTex);

		if (numIndices >= MAX_INDICES) { //overflow
			GL_DrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
			c_brushTris += numIndices / 3;

			R_ShowTrisBSP(false, numIndices, 1.0, 0.0, 1.0, glassProgram);
			numIndices = 0;
		}
	}
	if (numIndices) {
		GL_DrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
		c_brushTris += numIndices / 3;

		R_ShowTrisBSP(false, numIndices, 1.0, 0.0, 1.0, glassProgram);
		numIndices = 0;
	}
	numAlphaSurfaces = 0;
}

void R_AddWaterSurceces(msurface_t* s, uint* indeces, bool update) {
	int i;
	uint numIndices;
	float scroll = 0.0;
	bool scrolling = false;
	int nv = s->polys->numVerts;

	numIndices = *indeces;

	if (update) {
		if (s->texInfo->flags & (SURF_TRANS33 | SURF_TRANS66))
			qglUniform1i(U_WATER_TRANS, 1);
		else
			qglUniform1i(U_WATER_TRANS, 0);
		GL_SetBindlessTexture(U_TMU0, s->texInfo->albedo->handle);
	}

	for (i = 0; i < nv - 2; i++) {
		indexArray[numIndices++] = s->baseIndex;
		indexArray[numIndices++] = s->baseIndex + i + 1;
		indexArray[numIndices++] = s->baseIndex + i + 2;
	}
	*indeces = numIndices;
}

void R_DrawWaterSurfaces(bool bmodel) {
	msurface_t* s;
	float		ambientScale = max(r_lightmapScale->value, 0.33);
	bool	newTex;
	uint		oldTex = 0;
	uint		numIndices = 0;

	GL_BindProgram(waterProgram);

	GL_SetBindlessTexture(U_TMU1, gi.waterNormals[((int)(r_newrefdef.time * 15)) & (MAX_WATER_NORMALS - 1)]->handle);
	GL_SetBindlessTexture(U_TMU2, gi.hdrBaseInterim->handle);
	GL_SetBindlessTexture(U_TMU3, gi.linearDepth->handle);

	qglUniform1f(U_WATER_DEFORM_MUL, 1.0);
	qglUniform1f(U_AMBIENT_LEVEL, ambientScale);
	qglUniform1f(U_WATHER_THICKNESS, 150.0);
	qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);

	if (!bmodel)
		qglUniformMatrix4fv(U_MVP_MATRIX, 1, false, (const float*)r_newrefdef.modelViewProjectionMatrix);
	else
		qglUniformMatrix4fv(U_MVP_MATRIX, 1, false, (const float*)currententity->orMatrix);

	if (r_newrefdef.rdflags & RDF_UNDERWATER)
		qglUniform1i(U_WATER_MIRROR, 0);
	else
		qglUniform1i(U_WATER_MIRROR, 1);

	qglUniformMatrix4fv(U_MODELVIEW_MATRIX, 1, false, (const float*)r_newrefdef.modelViewMatrix);
	qglUniformMatrix4fv(U_PROJ_MATRIX, 1, false, (const float*)r_newrefdef.projectionMatrix);

	qsort(r_reflectiveSurfaces, numReflectiveSurfaces, sizeof(msurface_t*), (int(*)(const void*, const void*))waterSurfSort);

	for (int i = 0; i < numReflectiveSurfaces; i++) {
		s = r_reflectiveSurfaces[i];

		if (s->texInfo->albedo->texnum != oldTex) {
			if (numIndices) {
				GL_DrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
				c_brushTris += numIndices / 3;

				R_ShowTrisBSP(bmodel, numIndices, 0.0, 0.0, 1.0, waterProgram);

				numIndices = 0;
			}
			oldTex = s->texInfo->albedo->texnum;
			newTex = true;
		}
		else
			newTex = false;

		R_AddWaterSurceces(s, &numIndices, newTex);

		if (numIndices >= MAX_INDICES) { //overflow
			GL_DrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
			c_brushTris += numIndices / 3;
			
			R_ShowTrisBSP(bmodel, numIndices, 0.0, 0.0, 1.0, waterProgram);
			numIndices = 0;
		}
	}
	if (numIndices) {
		GL_DrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
		c_brushTris += numIndices / 3;

		R_ShowTrisBSP(bmodel, numIndices, 0.0, 0.0, 1.0, waterProgram);
		numIndices = 0;
	}
	numReflectiveSurfaces = 0;
}


void R_AddHeatHazeSurceces(msurface_t* s, uint* indeces) {
	int i;
	uint numIndices;
	int nv = s->polys->numVerts;

	numIndices = *indeces;

	for (i = 0; i < nv - 2; i++) {
		indexArray[numIndices++] = s->baseIndex;
		indexArray[numIndices++] = s->baseIndex + i + 1;
		indexArray[numIndices++] = s->baseIndex + i + 2;
	}
	*indeces = numIndices;
}

void R_DrawHeatHazeSurfaces() {
	msurface_t* s;
	uint		numIndices = 0;

	// setup program
	GL_BindProgram(heatHazeProgram);

	GL_SetBindlessTexture(U_TMU0, gi.waterNormals[((int)(r_newrefdef.time * 15)) & (MAX_WATER_NORMALS - 1)]->handle);
	GL_SetBindlessTexture(U_TMU1, gi.hdrBaseInterim->handle);
	GL_SetBindlessTexture(U_TMU2, gi.linearDepth->handle);

	qglUniform1f(U_REFR_DEFORM_MUL, 1.0);
	qglUniformMatrix4fv(U_MVP_MATRIX, 1, false, (const float*)r_newrefdef.modelViewProjectionMatrix);
	qglUniformMatrix4fv(U_MODELVIEW_MATRIX, 1, false, (const float*)r_newrefdef.modelViewMatrix);
	qglUniformMatrix4fv(U_PROJ_MATRIX, 1, false, (const float*)r_newrefdef.projectionMatrix);

	qglUniform1f(U_REFR_THICKNESS0, 50.0);
	qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);

	for (int i = 0; i < numHeatHazeSurfaces; i++) {

		s = r_heatHazeSurfaces[i];

		R_AddHeatHazeSurceces(s, &numIndices);

		if (numIndices >= MAX_INDICES) { //overflow
			GL_DrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
			c_brushTris += numIndices / 3;
			numIndices = 0;
		}
	}
	if (numIndices) {
		GL_DrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
		c_brushTris += numIndices / 3;
		numIndices = 0;
	}
	numHeatHazeSurfaces = 0;
}


void R_DrawSurfacesRA(bool bmodel) {

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	GL_BindVAO(vao.bsp);
	
	R_CaptureColorBuffer();
	R_DrawHeatHazeSurfaces();

	R_CaptureColorBuffer();
	R_DrawAlphaSurfaces();

	R_CaptureColorBuffer();
	R_DrawWaterSurfaces(bmodel);
}

bool R_MarkLightSurfRA(msurface_t* surf, bool world, worldShadowLight_t* light) {
	cplane_t* plane;
	float		dist;
	glpoly_t* poly;

	if (!(surf->texInfo->flags & (SURF_TRANS66 | SURF_TRANS33)))
		return false;

	plane = surf->plane;
	poly = surf->polys;

	if (poly->lightTimeStampRA == r_lightTimeStampRA)
		return false;

	switch (plane->type)
	{
	case PLANE_X:
		dist = light->origin[0] - plane->dist;
		break;
	case PLANE_Y:
		dist = light->origin[1] - plane->dist;
		break;
	case PLANE_Z:
		dist = light->origin[2] - plane->dist;
		break;
	default:
		dist = DotProduct(light->origin, plane->normal) - plane->dist;
		break;
	}

	if (fabsf(dist) > light->maxRad)
		return false;

	if (world)
	{
		float	lbbox[6], pbbox[6];

		lbbox[0] = light->origin[0] - light->radius[0];
		lbbox[1] = light->origin[1] - light->radius[1];
		lbbox[2] = light->origin[2] - light->radius[2];
		lbbox[3] = light->origin[0] + light->radius[0];
		lbbox[4] = light->origin[1] + light->radius[1];
		lbbox[5] = light->origin[2] + light->radius[2];

		// surface bounding box
		pbbox[0] = surf->mins[0];
		pbbox[1] = surf->mins[1];
		pbbox[2] = surf->mins[2];
		pbbox[3] = surf->maxs[0];
		pbbox[4] = surf->maxs[1];
		pbbox[5] = surf->maxs[2];

		if (light->projector && R_CullConeLight(&pbbox[0], &pbbox[3], light->frust))
			return false;

		if (!BoundsIntersect(&lbbox[0], &lbbox[3], &pbbox[0], &pbbox[3]))
			return false;
	}

	poly->lightTimeStampRA = r_lightTimeStampRA;

	return true;
}

void R_MarkLightCastingRA(mnode_t* node, bool precalc, worldShadowLight_t* light)
{
	cplane_t* plane;
	float				dist;
	msurface_t** surf;
	mleaf_t* leaf;
	int					c, cluster;

	if (light->isNoWorldModel)
		return;

	if (node->contents != -1)
	{
		//we are in a leaf
		leaf = (mleaf_t*)node;
		cluster = leaf->cluster;

		if (!(light->vis[cluster >> 3] & (1 << (cluster & 7))))
			return;

		surf = leaf->firstmarksurface;

		for (c = 0; c < leaf->numMarkSurfaces; c++, surf++) {

			if (R_MarkLightSurfRA((*surf), true, light)) {
				if (!precalc)
					interactionRA[numInteractionSurfsRA++] = (*surf);
				else
					light->interactionRA[light->numInteractionSurfsRA++] = (*surf);
			}
		}
		return;
	}

	plane = node->plane;
	dist = DotProduct(light->origin, plane->normal) - plane->dist;

	if (dist > light->maxRad)
	{
		R_MarkLightCastingRA(node->children[0], precalc, light);
		return;
	}
	if (dist < -light->maxRad)
	{
		R_MarkLightCastingRA(node->children[1], precalc, light);
		return;
	}

	R_MarkLightCastingRA(node->children[0], precalc, light);
	R_MarkLightCastingRA(node->children[1], precalc, light);
}

void R_AddLightAlphaSurceces(msurface_t* s, uint* indeces, bool update) {
	int i;
	uint numIndices;
	float scroll = 0.0;
	bool scrolling = false;
	int nv = s->polys->numVerts;
	float alpha, scale[2];

	numIndices = *indeces;

	if (s->texInfo->flags & SURF_FLOWING) {
		scroll = -64 * ((r_newrefdef.time / 500.0) - (int)(r_newrefdef.time / 500.0));

		if (scroll == 0.0)
			scroll = -64.0;

		scrolling = true;

		qglUniform1f(U_SCROLL, scroll);
	}
	else
		qglUniform1f(U_SCROLL, 0.0);
	
	scale[0] = r_parallaxScale->value / s->texInfo->albedo->width;
	scale[1] = r_parallaxScale->value / s->texInfo->albedo->height;

	qglUniform4f(U_PARALLAX_PARAMS, scale[0], scale[1], s->texInfo->albedo->upload_width, s->texInfo->albedo->upload_height);

	if (s->texInfo->flags & SURF_TRANS33)
		alpha = 0.33f;
	else
		alpha = 0.66f;
	qglUniform1f(U_PARAM_FLOAT_3, alpha);

	if (update) {
		GL_SetBindlessTexture(U_TMU0, s->texInfo->albedo->handle);
		GL_SetBindlessTexture(U_TMU1, s->texInfo->normalmap->handle);
		GL_SetBindlessTexture(U_TMU2, gi.lightCubeMaps[currentShadowLight->filter]->handle);
		GL_SetBindlessTexture(U_TMU3, gi.causticTexture[((int)(r_newrefdef.time * 15)) & (MAX_CAUSTICS - 1)]->handle);
	}

	for (i = 0; i < nv - 2; i++) {
		indexArray[numIndices++] = s->baseIndex;
		indexArray[numIndices++] = s->baseIndex + i + 1;
		indexArray[numIndices++] = s->baseIndex + i + 2;
	}
	*indeces = numIndices;
}

void R_UpdateLightRAuniforms(bool bModel){
	mat4_t		entAttenMatrix,
				entSpotMatrix;

	qglUniform1i(U_AMBIENT_LIGHT, (int)currentShadowLight->isAmbient);
	qglUniform3fv(U_LIGHT_POS, 1, currentShadowLight->origin);
	qglUniform4f(U_COLOR, currentShadowLight->color[0] * r_hdrLightScale->value, currentShadowLight->color[1] * r_hdrLightScale->value, currentShadowLight->color[2] * r_hdrLightScale->value, 1.0);
	qglUniform1i(U_USE_FOG, (int)currentShadowLight->isFog);
	qglUniform1f(U_FOG_DENSITY, currentShadowLight->fogDensity);
	qglUniform1f(U_CAUSTICS_SCALE, 2.5);

	if (bModel)
		qglUniform3fv(U_VIEW_POS, 1, BmodelViewOrg);
	else
		qglUniform3fv(U_VIEW_POS, 1, r_origin);

	if (!bModel) {
		qglUniformMatrix4fv(U_MVP_MATRIX, 1, false, (const float*)r_newrefdef.modelViewProjectionMatrix);
		qglUniformMatrix4fv(U_ATTEN_MATRIX, 1, false, (const float*)currentShadowLight->attenMatrix);
		qglUniformMatrix4fv(U_SPOT_MATRIX, 1, false, (const float*)currentShadowLight->spotMatrix);
	}
	else {
		qglUniformMatrix4fv(U_MVP_MATRIX, 1, false, (const float*)currententity->orMatrix);

		Mat4_TransposeMultiply(currententity->matrix, currentShadowLight->attenMatrix, entAttenMatrix);
		qglUniformMatrix4fv(U_ATTEN_MATRIX, 1, false, (const float*)entAttenMatrix);

		Mat4_TransposeMultiply(currententity->matrix, currentShadowLight->spotMatrix, entSpotMatrix);
		qglUniformMatrix4fv(U_SPOT_MATRIX, 1, false, (const float*)entSpotMatrix);
	}

	qglUniform3f(U_SPOT_PARAMS, currentShadowLight->hotSpot, 1.f / (1.f - currentShadowLight->hotSpot), currentShadowLight->coneExp);

	if (currentShadowLight->projector)
		qglUniform1i(U_SPOT_LIGHT, 1);
	else
		qglUniform1i(U_SPOT_LIGHT, 0);

	R_CalcCubeMapMatrix(bModel);
	qglUniformMatrix4fv(U_CUBE_MATRIX, 1, false, (const float*)currentShadowLight->cubeMapMatrix);

}
void R_DrawLightAlphaSurfaces() {
	msurface_t* s;
	bool	newTex;
	uint		oldTex = 0;
	uint		oldFlag = 0;
	uint		numIndices = 0;
	
	R_UpdateLightRAuniforms(false);

	for (int i = 0; i < currentShadowLight->numInteractionSurfsRA; i++) {

		s = currentShadowLight->interactionRA[i];

		if (s->texInfo->flags & SURF_WARP)
			continue;

		if ((s->visframe != r_framecount) || (s->ent))
			continue;

		if (s->texInfo->albedo->texnum != oldTex || s->flags != oldFlag) {
			if (numIndices) {
				GL_DrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
				c_lightBrushTris += numIndices / 3;
				numIndices = 0;
			}
			oldTex	= s->texInfo->albedo->texnum;
			oldFlag = s->flags;
			newTex	= true;
		}
		else
			newTex = false;

		R_AddLightAlphaSurceces(s, &numIndices, newTex);

		if (numIndices >= MAX_INDICES) { //overflow
			GL_DrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
			c_lightBrushTris += numIndices / 3;
			numIndices = 0;
		}
	}
	if (numIndices) {
		GL_DrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
		c_lightBrushTris += numIndices / 3;
		numIndices = 0;
	}
}


void R_DrawLightAlphaSurfacesDynamic(bool bmodel, bool caustics) {
	msurface_t* s;
	glpoly_t*	poly;
	bool	newTex;
	uint		oldTex = 0;
	uint		oldCaust = 0;
	uint		oldFlag = 0;
	uint		numIndices = 0;
	
	R_UpdateLightRAuniforms(bmodel);

	qsort(interactionRA, numInteractionSurfsRA, sizeof(msurface_t*), (int(*)(const void*, const void*))alphaSurfSort);

	for (int i = 0; i < numInteractionSurfsRA; i++) {

		s = interactionRA[i];
		poly = s->polys;

		if (s->texInfo->flags & SURF_WARP)
			continue;

		if ((poly->lightTimeStampRA != r_lightTimeStampRA) || (s->visframe != r_framecount))
			continue;

		if (s->texInfo->albedo->texnum != oldTex || s->flags != oldFlag || caustics != oldCaust) {
			if (numIndices) {
				GL_DrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
				c_brushTris += numIndices / 3;
				numIndices = 0;
			}
			oldTex = s->texInfo->albedo->texnum;
			oldFlag = s->flags;
			oldCaust = caustics;
			newTex = true;
		}
		else
			newTex = false;

		R_AddLightAlphaSurceces(s, &numIndices, newTex);

		if (numIndices >= MAX_INDICES) { //overflow
			GL_DrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
			c_lightBrushTris += numIndices / 3;
			numIndices = 0;
		}
	}
	if (numIndices) {
		GL_DrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
		c_lightBrushTris += numIndices / 3;
		numIndices = 0;
	}
}

void R_DrawLightRA(void){

	if (currentShadowLight->isStatic)
		R_DrawLightAlphaSurfaces();
	else {
		r_lightTimeStampRA++;
		numInteractionSurfsRA = 0;
		R_MarkLightCastingRA(r_worldmodel->nodes, false, currentShadowLight);
		if (numInteractionSurfsRA > 0)
			R_DrawLightAlphaSurfacesDynamic(false, false);
	}

}

void R_MarkLightBrushModelSurfacesRA(void) {
	int			i;
	msurface_t* psurf;
	model_t* clmodel;

	clmodel = currententity->model;
	psurf = &clmodel->surfaces[clmodel->firstModelSurface];

	for (i = 0; i < clmodel->numModelSurfaces; i++, psurf++) {

		if (R_MarkLightSurfRA(psurf, false, currentShadowLight))
			interactionRA[numInteractionSurfsRA++] = psurf;
	}
}

void R_DrawLightBrushModelRA(void) {
	vec3_t		mins, maxs, org;
	int			i;
	bool	rotated;
	vec3_t		tmp, oldLight;
	bool	caustics;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	if (currentmodel->numModelSurfaces == 0)
		return;

	if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2]) {
		rotated = true;
		for (i = 0; i < 3; i++) {
			mins[i] = currententity->origin[i] - currentmodel->radius;
			maxs[i] = currententity->origin[i] + currentmodel->radius;
		}
	}
	else {
		rotated = false;
		VectorAdd(currententity->origin, currentmodel->mins, mins);
		VectorAdd(currententity->origin, currentmodel->maxs, maxs);
	}

	if (currentShadowLight->spherical) {
		if (!BoundsAndSphereIntersect(mins, maxs, currentShadowLight->origin, currentShadowLight->radius[0]))
			return;
	}
	else {
		if (!BoundsIntersect(mins, maxs, currentShadowLight->mins, currentShadowLight->maxs))
			return;
	}

	R_SetupEntityMatrix(currententity);

	//Put camera into model space view angle for bmodels parallax
	VectorSubtract(r_origin, currententity->origin, tmp);
	Mat3_TransposeMultiplyVector(currententity->axis, tmp, BmodelViewOrg);

	VectorCopy(currentShadowLight->origin, oldLight);
	VectorSubtract(currentShadowLight->origin, currententity->origin, tmp);
	Mat3_TransposeMultiplyVector(currententity->axis, tmp, currentShadowLight->origin);

	caustics = false;
	currententity->minmax[0] = mins[0];
	currententity->minmax[1] = mins[1];
	currententity->minmax[2] = mins[2];
	currententity->minmax[3] = maxs[0];
	currententity->minmax[4] = maxs[1];
	currententity->minmax[5] = maxs[2];

	VectorSet(org, currententity->minmax[0], currententity->minmax[1], currententity->minmax[5]);
	if (CL_PMpointcontents2(org, currentmodel) & MASK_WATER)
		caustics = true;
	else
	{
		VectorSet(org, currententity->minmax[3], currententity->minmax[1], currententity->minmax[5]);
		if (CL_PMpointcontents2(org, currentmodel) & MASK_WATER)
			caustics = true;
		else
		{
			VectorSet(org, currententity->minmax[0], currententity->minmax[4], currententity->minmax[5]);
			if (CL_PMpointcontents2(org, currentmodel) & MASK_WATER)
				caustics = true;
			else
			{
				VectorSet(org, currententity->minmax[3], currententity->minmax[4], currententity->minmax[5]);
				if (CL_PMpointcontents2(org, currentmodel) & MASK_WATER)
					caustics = true;
			}
		}
	}

	r_lightTimeStampRA++;
	numInteractionSurfsRA = 0;

	R_MarkLightBrushModelSurfacesRA();

	if (numInteractionSurfsRA > 0)
		R_DrawLightAlphaSurfacesDynamic(true, caustics);

	VectorCopy(oldLight, currentShadowLight->origin);
}

void R_DrawLightWorldRA(void){

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
	
	if (!r_transSurfShading->integer)
		return;

	GL_Enable(GL_BLEND);
	GL_BlendFunc(GL_ONE, GL_ONE);
	
	GL_BindProgram(lightGlassProgram);
	
	GL_BindVAO(vao.bsp);

	R_PrepareShadowLightFrame(false);

	if (shadowLight_frame) {

		for (currentShadowLight = shadowLight_frame; currentShadowLight; currentShadowLight = currentShadowLight->next) {

			if (r_skipStaticLights->integer && currentShadowLight->isStatic)
				continue;

			R_DrawLightRA();

			for (int i = 0; i < r_newrefdef.num_entities; i++) {
				currententity = &r_newrefdef.entities[i];
				currentmodel = currententity->model;

				if (!currentmodel)
					continue;
				
				if (currentmodel->type == mod_brush)
					R_DrawLightBrushModelRA();
			}
		}
	}

//	GL_BindNullVAO();
	GL_Disable(GL_BLEND);
}
