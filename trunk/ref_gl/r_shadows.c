/*
Copyright (C) 2004-2013 Quake2xp Team, Berserker.

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

int			num_shadow_surfaces, shadowTimeStamp;
vec4_t		s_lerped[MAX_VERTS];
vec3_t		vcache[MAX_MAP_TEXINFO * MAX_POLY_VERT];
index_t		icache[MAX_MAP_TEXINFO * MAX_POLY_VERT];
msurface_t	*shadow_surfaces[MAX_MAP_FACES];
char		triangleFacingLight[MAX_INDICES / 3];

/*
=====================
Alias Shadow Volumes
=====================
*/

void R_MarkShadowTriangles (dmdl_t *paliashdr, dtriangle_t *tris, vec3_t lightOrg) {

	vec3_t	r_triangleNormals[MAX_INDICES / 3];
	vec3_t	temp, dir0, dir1;
	int		i;
	float	f;
	float	*v0, *v1, *v2;

	for (i = 0; i < paliashdr->num_tris; i++, tris++) {

		v0 = (float*)s_lerped[tris->index_xyz[0]];
		v1 = (float*)s_lerped[tris->index_xyz[1]];
		v2 = (float*)s_lerped[tris->index_xyz[2]];

		//Calculate shadow volume triangle normals
		VectorSubtract (v0, v1, dir0);
		VectorSubtract (v2, v1, dir1);

		CrossProduct (dir0, dir1, r_triangleNormals[i]);

		// Find front facing triangles
		VectorSubtract (lightOrg, v0, temp);
		f = DotProduct (temp, r_triangleNormals[i]);

		triangleFacingLight[i] = f > 0;
	}
}

void BuildShadowVolumeTriangles (dmdl_t * hdr, vec3_t light, float lightRadius) {
	dtriangle_t		*ot, *tris;
	neighbors_t		*neighbours;
	vec4_t			v0, v1, v2, v3, l0, l1, l2, l3;
	daliasframe_t	*frame;
	dtrivertx_t		*verts;
	int				i, j, numVerts = 0, id = 0;

	frame = (daliasframe_t *)((byte *)hdr + hdr->ofs_frames + currententity->frame * hdr->framesize);
	verts = frame->verts;
	ot = tris = (dtriangle_t *)((unsigned char *)hdr + hdr->ofs_tris);

	R_MarkShadowTriangles (hdr, tris, light);

	for (i = 0, tris = ot, neighbours = currentmodel->neighbours; i < hdr->num_tris; i++, tris++, neighbours++) {

		if (!triangleFacingLight[i])
			continue;

		if (neighbours->n[0] < 0 || !triangleFacingLight[neighbours->n[0]]) {

			for (j = 0; j < 3; j++) {
				v0[j] = s_lerped[tris->index_xyz[1]][j];
				v1[j] = s_lerped[tris->index_xyz[0]][j];
			}

			VectorSubtract (v1, light, l2);
			VectorSubtract (v0, light, l3);
			VectorMA (v1, lightRadius, l2, v2);
			VectorMA (v0, lightRadius, l3, v3);

			VA_SetElem3 (vcache[numVerts + 0], v0[0], v0[1], v0[2]);
			VA_SetElem3 (vcache[numVerts + 1], v1[0], v1[1], v1[2]);
			VA_SetElem3 (vcache[numVerts + 2], v2[0], v2[1], v2[2]);
			VA_SetElem3 (vcache[numVerts + 3], v3[0], v3[1], v3[2]);

			icache[id++] = numVerts + 0;
			icache[id++] = numVerts + 1;
			icache[id++] = numVerts + 3;
			icache[id++] = numVerts + 3;
			icache[id++] = numVerts + 1;
			icache[id++] = numVerts + 2;
			numVerts += 4;
		}

		if (neighbours->n[1] < 0 || !triangleFacingLight[neighbours->n[1]]) {

			for (j = 0; j < 3; j++) {
				v0[j] = s_lerped[tris->index_xyz[2]][j];
				v1[j] = s_lerped[tris->index_xyz[1]][j];
			}

			VectorSubtract (v1, light, l2);
			VectorSubtract (v0, light, l3);
			VectorMA (v1, lightRadius, l2, v2);
			VectorMA (v0, lightRadius, l3, v3);

			VA_SetElem3 (vcache[numVerts + 0], v0[0], v0[1], v0[2]);
			VA_SetElem3 (vcache[numVerts + 1], v1[0], v1[1], v1[2]);
			VA_SetElem3 (vcache[numVerts + 2], v2[0], v2[1], v2[2]);
			VA_SetElem3 (vcache[numVerts + 3], v3[0], v3[1], v3[2]);

			icache[id++] = numVerts + 0;
			icache[id++] = numVerts + 1;
			icache[id++] = numVerts + 3;
			icache[id++] = numVerts + 3;
			icache[id++] = numVerts + 1;
			icache[id++] = numVerts + 2;
			numVerts += 4;
		}

		if (neighbours->n[2] < 0 || !triangleFacingLight[neighbours->n[2]]) {

			for (j = 0; j < 3; j++) {
				v0[j] = s_lerped[tris->index_xyz[0]][j];
				v1[j] = s_lerped[tris->index_xyz[2]][j];
			}

			VectorSubtract (v1, light, l2);
			VectorSubtract (v0, light, l3);
			VectorMA (v1, lightRadius, l2, v2);
			VectorMA (v0, lightRadius, l3, v3);

			VA_SetElem3 (vcache[numVerts + 0], v0[0], v0[1], v0[2]);
			VA_SetElem3 (vcache[numVerts + 1], v1[0], v1[1], v1[2]);
			VA_SetElem3 (vcache[numVerts + 2], v2[0], v2[1], v2[2]);
			VA_SetElem3 (vcache[numVerts + 3], v3[0], v3[1], v3[2]);

			icache[id++] = numVerts + 0;
			icache[id++] = numVerts + 1;
			icache[id++] = numVerts + 3;
			icache[id++] = numVerts + 3;
			icache[id++] = numVerts + 1;
			icache[id++] = numVerts + 2;
			numVerts += 4;
		}
	}

	// build shadows caps
	for (i = 0, tris = ot; i < hdr->num_tris; i++, tris++) {
		if (!triangleFacingLight[i])
			continue;

		for (j = 0; j < 3; j++) {
			v0[j] = s_lerped[tris->index_xyz[0]][j];
			v1[j] = s_lerped[tris->index_xyz[1]][j];
			v2[j] = s_lerped[tris->index_xyz[2]][j];
		}

		VA_SetElem3 (vcache[numVerts + 0], v0[0], v0[1], v0[2]);
		VA_SetElem3 (vcache[numVerts + 1], v1[0], v1[1], v1[2]);
		VA_SetElem3 (vcache[numVerts + 2], v2[0], v2[1], v2[2]);

		icache[id++] = numVerts + 0;
		icache[id++] = numVerts + 1;
		icache[id++] = numVerts + 2;
		numVerts += 3;

		// rear cap (with flipped winding order)

		for (j = 0; j < 3; j++) {
			v0[j] = s_lerped[tris->index_xyz[0]][j];
			v1[j] = s_lerped[tris->index_xyz[1]][j];
			v2[j] = s_lerped[tris->index_xyz[2]][j];
		}

		VectorSubtract (v0, light, l0);
		VectorSubtract (v1, light, l1);
		VectorSubtract (v2, light, l2);
		VectorMA (v0, lightRadius, l0, v0);
		VectorMA (v1, lightRadius, l1, v1);
		VectorMA (v2, lightRadius, l2, v2);

		VA_SetElem3 (vcache[numVerts + 0], v0[0], v0[1], v0[2]);
		VA_SetElem3 (vcache[numVerts + 1], v1[0], v1[1], v1[2]);
		VA_SetElem3 (vcache[numVerts + 2], v2[0], v2[1], v2[2]);

		icache[id++] = numVerts + 2;
		icache[id++] = numVerts + 1;
		icache[id++] = numVerts + 0;
		numVerts += 3;
	}

	qglDrawElements (GL_TRIANGLES, id, GL_UNSIGNED_SHORT, icache);

	c_shadow_tris += id / 3;
	c_shadow_volumes++;
}

qboolean R_EntityInLightBounds () {

	int		i;
	vec3_t	mins, maxs;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return false;

	if (currententity->flags & (RF_SHELL_HALF_DAM | RF_SHELL_GREEN | RF_SHELL_RED |
		RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_GOD |
		RF_TRANSLUCENT | RF_BEAM | RF_WEAPONMODEL | RF_NOSHADOW | RF_DISTORT))
		return false;

	if (!r_playerShadow->value && (currententity->flags & RF_VIEWERMODEL))
		return false;

	if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2]) {
		for (i = 0; i < 3; i++) {
			mins[i] = currententity->origin[i] - currentmodel->radius;
			maxs[i] = currententity->origin[i] + currentmodel->radius;
		}
	}
	else {
		VectorAdd (currententity->origin, currententity->model->maxs, maxs);
		VectorAdd (currententity->origin, currententity->model->mins, mins);
	}

	if (!InLightVISEntity())
		return false;

	if (currentShadowLight->spherical) {

		if (!BoundsAndSphereIntersect (mins, maxs, currentShadowLight->origin, currentShadowLight->radius[0]))
			return false;
	}
	else {

		if (!BoundsIntersect (mins, maxs, currentShadowLight->mins, currentShadowLight->maxs))
			return false;
	}

	if (VectorCompare (currentShadowLight->origin, currententity->origin))
		return false;

	return true;
}


void GL_LerpShadowVerts (int nverts, dtrivertx_t *v, dtrivertx_t *ov, dtrivertx_t *verts, float *lerp, float move[3], float frontv[3], float backv[3]) {
	int i;

	if (nverts < 1)
		return;

	for (i = 0; i < nverts; i++, v++, ov++, lerp += 4) {
		lerp[0] = move[0] + ov->v[0] * backv[0] + v->v[0] * frontv[0];
		lerp[1] = move[1] + ov->v[1] * backv[1] + v->v[1] * frontv[1];
		lerp[2] = move[2] + ov->v[2] * backv[2] + v->v[2] * frontv[2];
	}
}

void R_DeformShadowVolume () {
	dmdl_t			*paliashdr;
	daliasframe_t	*frame, *oldframe;
	dtrivertx_t		*v, *ov, *verts;
	int				*order, i;
	float			frontlerp;
	vec3_t			move, delta, vectors[3], frontv, backv, light, temp;

	if (!R_EntityInLightBounds ())
		return;

	paliashdr = (dmdl_t *)currentmodel->extraData;

	frame = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames
		+ currententity->frame * paliashdr->framesize);

	verts = v = frame->verts;

	oldframe = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames +
		currententity->oldframe * paliashdr->framesize);

	ov = oldframe->verts;

	order = (int *)((byte *)paliashdr + paliashdr->ofs_glcmds);

	frontlerp = 1.0 - currententity->backlerp;

	// move should be the delta back to the previous frame * backlerp
	VectorSubtract (currententity->oldorigin, currententity->origin, delta);
	AngleVectors (currententity->angles, vectors[0], vectors[1],
		vectors[2]);

	move[0] = DotProduct (delta, vectors[0]);	// forward
	move[1] = -DotProduct (delta, vectors[1]);	// left
	move[2] = DotProduct (delta, vectors[2]);	// up

	VectorAdd (move, oldframe->translate, move);

	for (i = 0; i < 3; i++) {
		move[i] = currententity->backlerp * move[i] + frontlerp * frame->translate[i];
		frontv[i] = frontlerp * frame->scale[i];
		backv[i] = currententity->backlerp * oldframe->scale[i];
	}

	GL_LerpShadowVerts (paliashdr->num_xyz, v, ov, verts, s_lerped[0], move, frontv, backv);

	qglPushMatrix ();
	R_RotateForEntity (currententity);

	VectorSubtract (currentShadowLight->origin, currententity->origin, temp);
	Mat3_TransposeMultiplyVector (currententity->axis, temp, light);

	BuildShadowVolumeTriangles (paliashdr, light, currentShadowLight->len * 10);

	qglPopMatrix ();
}

void R_CastAliasShadowVolumes (void) {
	int			id, i;
	unsigned	defBits = 0;

	if (!r_shadows->value || !r_drawEntities->value)
		return;

	if (!currentShadowLight->isShadow || currentShadowLight->isAmbient)
		return;

	// setup program
	GL_BindProgram (nullProgram, defBits);
	id = nullProgram->id[defBits];

	GL_StencilMask (255);
	GL_StencilFuncSeparate (GL_FRONT_AND_BACK, GL_ALWAYS, 128, 255);
	GL_StencilOpSeparate (GL_BACK, GL_KEEP, GL_INCR_WRAP_EXT, GL_KEEP);
	GL_StencilOpSeparate (GL_FRONT, GL_KEEP, GL_DECR_WRAP_EXT, GL_KEEP);

	GL_Disable (GL_CULL_FACE);
	qglDisable (GL_TEXTURE_2D);
	GL_DepthFunc (GL_LESS);
	GL_Enable (GL_POLYGON_OFFSET_FILL);
	GL_PolygonOffset (0.1, 1);
	GL_ColorMask (0, 0, 0, 0);

	qglEnableVertexAttribArray (ATRB_POSITION);
	qglVertexAttribPointer (ATRB_POSITION, 3, GL_FLOAT, false, 0, vcache);

	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];
		currentmodel = currententity->model;

		if (!currentmodel)
			continue;

		if (currentmodel->type == mod_alias)
			R_DeformShadowVolume ();

	}
	qglDisableVertexAttribArray (ATRB_POSITION);
	GL_Disable (GL_POLYGON_OFFSET_FILL);
	GL_PolygonOffset (0, 0);
	qglEnable (GL_TEXTURE_2D);
	GL_Enable (GL_CULL_FACE);
	GL_ColorMask (1, 1, 1, 1);
	GL_BindNullProgram ();
}


/*
======================================
BSP SHADOW VOLUMES
EASY VERSION FROM TENEBRAE AND BERS@Q2
======================================
*/

qboolean R_MarkShadowSurf (msurface_t *surf) {
	cplane_t	*plane;
	glpoly_t	*poly;
	float		dist, lbbox[6], pbbox[6];

	if (surf->texInfo->flags & (SURF_SKY))
		goto hack;

	// add sky surfaces to shadow marking
	if ((surf->texInfo->flags & (SURF_TRANS33 | SURF_TRANS66 | SURF_WARP | SURF_NODRAW)) || (surf->flags & MSURF_DRAWTURB))
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
	if (abs (dist) > currentShadowLight->len)
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

	if (!BoundsIntersect (&lbbox[0], &lbbox[3], &pbbox[0], &pbbox[3]))
		return false;

	if (currentShadowLight->_cone && R_CullBox_ (&pbbox[0], &pbbox[3], currentShadowLight->frust))
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
			shadow_surfaces[num_shadow_surfaces++] = psurf;
	}
}

void R_DrawBrushModelVolumes () {
	int			i, j, vb = 0, ib = 0, surfBase = 0;
	float		scale, sca;
	msurface_t	*surf;
	model_t		*clmodel;
	glpoly_t	*poly;
	vec3_t		v1, temp, oldLightOrigin;
	qboolean	shadow;

	clmodel = currententity->model;
	surf = &clmodel->surfaces[clmodel->firstModelSurface];

	if (!R_EntityInLightBounds ())
		return;

	qglPushMatrix ();
	R_RotateForEntity (currententity);

	VectorCopy (currentShadowLight->origin, oldLightOrigin);
	VectorSubtract (currentShadowLight->origin, currententity->origin, temp);
	Mat3_TransposeMultiplyVector (currententity->axis, temp, currentShadowLight->origin);

	shadowTimeStamp++;
	num_shadow_surfaces = 0;
	R_MarkBrushModelShadowSurfaces ();

	scale = currentShadowLight->len * 10;

	// generate vertex buffer
	for (i = 0; i < num_shadow_surfaces; i++) {
		surf = shadow_surfaces[i];
		poly = surf->polys;

		if (surf->polys->shadowTimestamp != shadowTimeStamp)
			continue;

		for (j = 0; j < surf->numEdges; j++) {

			VectorCopy (poly->verts[j], vcache[vb * 2 + 0]);
			VectorSubtract (poly->verts[j], currentShadowLight->origin, v1);

			sca = scale / VectorLength (v1);
			vcache[vb * 2 + 1][0] = v1[0] * sca + poly->verts[j][0];
			vcache[vb * 2 + 1][1] = v1[1] * sca + poly->verts[j][1];
			vcache[vb * 2 + 1][2] = v1[2] * sca + poly->verts[j][2];
			vb++;
		}
	}

	// generate index buffer
	for (i = 0; i < num_shadow_surfaces; i++) {
		surf = shadow_surfaces[i];
		poly = surf->polys;

		if (surf->polys->shadowTimestamp != shadowTimeStamp)
			continue;

		for (j = 0; j < surf->numEdges; j++) {
			shadow = false;

			if (poly->neighbours[j] != NULL) {

				if (poly->neighbours[j]->shadowTimestamp != poly->shadowTimestamp)
					shadow = true;
			}
			else
				shadow = true;

			if (shadow) {
				int jj = (j + 1) % poly->numVerts;

				icache[ib++] = j * 2 + 0 + surfBase;
				icache[ib++] = j * 2 + 1 + surfBase;
				icache[ib++] = jj * 2 + 1 + surfBase;

				icache[ib++] = j * 2 + 0 + surfBase;
				icache[ib++] = jj * 2 + 1 + surfBase;
				icache[ib++] = jj * 2 + 0 + surfBase;
			}
		}

		//Draw near light cap
		for (j = 0; j < surf->numEdges - 2; j++) {
			icache[ib++] = 0 + surfBase;
			icache[ib++] = (j + 1) * 2 + 0 + surfBase;
			icache[ib++] = (j + 2) * 2 + 0 + surfBase;
		}

		//Draw extruded cap
		for (j = 0; j < surf->numEdges - 2; j++) {
			icache[ib++] = 1 + surfBase;
			icache[ib++] = (j + 2) * 2 + 1 + surfBase;
			icache[ib++] = (j + 1) * 2 + 1 + surfBase;
		}
		surfBase += surf->numEdges * 2;
	}

	if (ib) {
		qglVertexAttribPointer (ATRB_POSITION, 3, GL_FLOAT, false, 0, vcache);
		qglDrawElements	(GL_TRIANGLES, ib, GL_UNSIGNED_SHORT, icache);
	}

	c_shadow_volumes++;
	c_shadow_tris += ib / 3;

	VectorCopy (oldLightOrigin, currentShadowLight->origin);
	qglPopMatrix ();
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

				shadow_surfaces[num_shadow_surfaces++] = (*surf);
			}
		}
		return;
	}

	plane = node->plane;
	dist = DotProduct (currentShadowLight->origin, plane->normal) - plane->dist;

	if (dist > currentShadowLight->len) {
		R_MarkShadowCasting (node->children[0]);
		return;
	}

	if (dist < -currentShadowLight->len) {
		R_MarkShadowCasting (node->children[1]);
		return;
	}

	R_MarkShadowCasting (node->children[0]);
	R_MarkShadowCasting (node->children[1]);
}

model_t *loadmodel;
int numPreCachedLights;

void R_DrawBspModelVolumes (qboolean precalc, worldShadowLight_t *light) {
	int			i, j, vb = 0, ib = 0, surfBase = 0;
	float		scale, sca;
	msurface_t	*surf;
	glpoly_t	*poly;
	vec3_t		v1;
	qboolean	shadow;

	if (precalc)
		currentShadowLight = light;
	else
		light = NULL;

	shadowTimeStamp++;
	num_shadow_surfaces = 0;
	R_MarkShadowCasting (r_worldmodel->nodes);

	scale = currentShadowLight->len * 10;

	// generate vertex buffer
	for (i = 0; i < num_shadow_surfaces; i++) {
		surf = shadow_surfaces[i];
		poly = surf->polys;

		if (surf->polys->shadowTimestamp != shadowTimeStamp)
			continue;

		for (j = 0; j < surf->numEdges; j++) {

			VectorCopy (poly->verts[j], vcache[vb * 2 + 0]);
			VectorSubtract (poly->verts[j], currentShadowLight->origin, v1);

			sca = scale / VectorLength (v1);

			vcache[vb * 2 + 1][0] = v1[0] * sca + poly->verts[j][0];
			vcache[vb * 2 + 1][1] = v1[1] * sca + poly->verts[j][1];
			vcache[vb * 2 + 1][2] = v1[2] * sca + poly->verts[j][2];
			vb++;
		}
	}

	// generate index buffer
	for (i = 0; i < num_shadow_surfaces; i++) {
		surf = shadow_surfaces[i];
		poly = surf->polys;

		if (surf->polys->shadowTimestamp != shadowTimeStamp)
			continue;

		for (j = 0; j < surf->numEdges; j++) {
			shadow = false;

			if (poly->neighbours[j] != NULL) {

				if (poly->neighbours[j]->shadowTimestamp != poly->shadowTimestamp)
					shadow = true;
			}
			else
				shadow = true;

			if (shadow) {
				int jj = (j + 1) % poly->numVerts;

				icache[ib++] = j * 2 + 0 + surfBase;
				icache[ib++] = j * 2 + 1 + surfBase;
				icache[ib++] = jj * 2 + 1 + surfBase;

				icache[ib++] = j * 2 + 0 + surfBase;
				icache[ib++] = jj * 2 + 1 + surfBase;
				icache[ib++] = jj * 2 + 0 + surfBase;
			}
		}

		//Draw near light cap
		for (j = 0; j < surf->numEdges - 2; j++) {
			icache[ib++] = 0 + surfBase;
			icache[ib++] = (j + 1) * 2 + 0 + surfBase;
			icache[ib++] = (j + 2) * 2 + 0 + surfBase;
		}

		//Draw extruded cap
		for (j = 0; j < surf->numEdges - 2; j++) {
			icache[ib++] = 1 + surfBase;
			icache[ib++] = (j + 2) * 2 + 1 + surfBase;
			icache[ib++] = (j + 1) * 2 + 1 + surfBase;
		}
		surfBase += surf->numEdges * 2;
	}

	if (precalc) {

		if (currentShadowLight->vboId)
			qglDeleteBuffers (1, &currentShadowLight->vboId);

		qglGenBuffers (1, &currentShadowLight->vboId);
		qglBindBuffer (GL_ARRAY_BUFFER_ARB, currentShadowLight->vboId);
		qglBufferData (GL_ARRAY_BUFFER_ARB, surfBase*sizeof(vec3_t), vcache, GL_STATIC_DRAW_ARB);
		qglBindBuffer (GL_ARRAY_BUFFER_ARB, 0);

		if (currentShadowLight->iboId)
			qglDeleteBuffers (1, &currentShadowLight->iboId);

		qglGenBuffers (1, &currentShadowLight->iboId);
		qglBindBuffer (GL_ELEMENT_ARRAY_BUFFER, currentShadowLight->iboId);
		qglBufferData (GL_ELEMENT_ARRAY_BUFFER, ib*sizeof(GL_UNSIGNED_SHORT), icache, GL_STATIC_DRAW_ARB);
		currentShadowLight->iboNumIndices = ib;
		qglBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
		numPreCachedLights++;
	}
	else {
		if (ib) {
			qglVertexAttribPointer (ATRB_POSITION, 3, GL_FLOAT, false, 0, vcache);
			qglDrawElements	(GL_TRIANGLES, ib, GL_UNSIGNED_SHORT, icache);
		}
	}
	c_shadow_volumes++;
	c_shadow_tris += ib / 3;
}


void R_CastBspShadowVolumes (void) {
	int			id, i;
	unsigned	defBits = 0;

	if (!r_shadows->value)
		return;

	if (!currentShadowLight->isShadow || currentShadowLight->isAmbient)
		return;

	// setup program
	GL_BindProgram (nullProgram, defBits);
	id = nullProgram->id[defBits];

	GL_StencilMask (255);
	GL_StencilFuncSeparate (GL_FRONT_AND_BACK, GL_ALWAYS, 128, 255);
	GL_StencilOpSeparate (GL_BACK, GL_KEEP, GL_INCR_WRAP_EXT, GL_KEEP);
	GL_StencilOpSeparate (GL_FRONT, GL_KEEP, GL_DECR_WRAP_EXT, GL_KEEP);

	GL_Disable (GL_CULL_FACE);
	qglDisable (GL_TEXTURE_2D);
	GL_DepthFunc (GL_LESS);
	GL_Enable (GL_POLYGON_OFFSET_FILL);
	GL_PolygonOffset (0.1, 1);
	GL_ColorMask (0, 0, 0, 0);
	qglEnableVertexAttribArray (ATRB_POSITION);

	if (currentShadowLight->vboId && currentShadowLight->iboId && currentShadowLight->isStatic) { // draw vbo shadow

		qglBindBuffer (GL_ARRAY_BUFFER_ARB, currentShadowLight->vboId);
		qglBindBuffer (GL_ELEMENT_ARRAY_BUFFER, currentShadowLight->iboId);

		qglVertexAttribPointer (ATRB_POSITION, 3, GL_FLOAT, false, 0, 0);
		qglDrawElements	(GL_TRIANGLES, currentShadowLight->iboNumIndices, GL_UNSIGNED_SHORT, NULL);

		qglBindBuffer (GL_ARRAY_BUFFER_ARB, 0);
		qglBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	if (!currentShadowLight->isStatic)
		R_DrawBspModelVolumes (false, NULL); //dlights have't vbo data

	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];
		currentmodel = currententity->model;

		if (!currentmodel)
			continue;

		if (currentmodel->type == mod_brush)
			R_DrawBrushModelVolumes ();
	}

	qglDisableVertexAttribArray (ATRB_POSITION);
	GL_Disable (GL_POLYGON_OFFSET_FILL);
	qglEnable (GL_TEXTURE_2D);
	GL_Enable (GL_CULL_FACE);
	GL_ColorMask (1, 1, 1, 1);
	GL_BindNullProgram ();
}