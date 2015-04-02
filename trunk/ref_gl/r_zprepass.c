#include "r_local.h"

extern index_t		indexArray[MAX_BATCH_SURFS * 3];
extern msurface_t	*scene_surfaces[MAX_MAP_FACES];
static int			num_depth_surfaces;
static vec3_t		modelorg;			// relative to viewpoint

qboolean R_FillDepthBatch (msurface_t *surf, unsigned *vertices, unsigned *indeces) {
	unsigned	numVertices, numIndices;
	int			i, nv = surf->numEdges;
	float		*v;
	glpoly_t	*p;

	numVertices = *vertices;
	numIndices = *indeces;

	if (numVertices + nv > MAX_BATCH_SURFS)
		return false;

	c_brush_polys++;

	// create indexes
	if (numIndices == 0xffffffff)
		numIndices = 0;

	for (i = 0; i < nv - 2; i++) {
		indexArray[numIndices++] = numVertices;
		indexArray[numIndices++] = numVertices + i + 1;
		indexArray[numIndices++] = numVertices + i + 2;
	}

	p = surf->polys;
	v = p->verts[0];

	for (i = 0; i < nv; i++, v += VERTEXSIZE, numVertices++) {
		VectorCopy (v, wVertexArray[numVertices]);
	}

	*vertices = numVertices;
	*indeces = numIndices;

	return true;
}

static void GL_DrawDepthPoly () {
	msurface_t	*s;
	int			i;

	unsigned	oldTex = 0xffffffff;
	unsigned	oldFlag = 0xffffffff;
	unsigned	numIndices = 0xffffffff;
	unsigned	numVertices = 0;

	for (i = 0; i < num_depth_surfaces; i++) {
		s = scene_surfaces[i];

	repeat:
		if (!R_FillDepthBatch (s, &numVertices, &numIndices)) {
			if (numIndices != 0xFFFFFFFF) {
				qglDrawElements (GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, indexArray);
				numVertices = 0;
				numIndices = 0xFFFFFFFF;
			}
			goto repeat;
		}
	}

	// draw the rest
	if (numIndices != 0xFFFFFFFF)
		qglDrawElements (GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, indexArray);
}

static void R_RecursiveDepthWorldNode (mnode_t * node) {
	int c, side, sidebit;
	cplane_t *plane;
	msurface_t *surf, **mark;
	mleaf_t *pleaf;
	float dot;

	if (node->contents == CONTENTS_SOLID)
		return;					// solid

	if (node->visframe != r_visframecount)
		return;

	if (R_CullBox (node->minmaxs, node->minmaxs + 3))
		return;

	// if a leaf node, draw stuff
	if (node->contents != -1) {
		pleaf = (mleaf_t *)node;

		// check for door connected areas
		if (r_newrefdef.areabits) {
			if (!(r_newrefdef.areabits[pleaf->area >> 3] & (1 << (pleaf->area & 7))))
				return;			// not visible
		}

		mark = pleaf->firstmarksurface;
		c = pleaf->numMarkSurfaces;

		if (c) {
			do {
				if (SurfInFrustum (*mark))
					(*mark)->visframe = r_framecount;
				(*mark)->ent = NULL;
				mark++;
			} while (--c);
		}

		return;
	}
	// node is just a decision point, so go down the apropriate sides
	// find which side of the node we are on
	plane = node->plane;

	switch (plane->type) {
		case PLANE_X:
			dot = modelorg[0] - plane->dist;
			break;
		case PLANE_Y:
			dot = modelorg[1] - plane->dist;
			break;
		case PLANE_Z:
			dot = modelorg[2] - plane->dist;
			break;
		default:
			dot = DotProduct (modelorg, plane->normal) - plane->dist;
			break;
	}

	if (dot >= 0) {
		side = 0;
		sidebit = 0;
	}
	else {
		side = 1;
		sidebit = MSURF_PLANEBACK;
	}

	// recurse down the children, front side first
	R_RecursiveDepthWorldNode (node->children[side]);

	// draw stuff
	for (c = node->numsurfaces, surf = r_worldmodel->surfaces + node->firstsurface; c; c--, surf++) {

		if (surf->visframe != r_framecount)
			continue;

		if ((surf->flags & MSURF_PLANEBACK) != sidebit)
			continue;			// wrong side

		if (surf->texInfo->flags & SURF_SKY) {	// just adds to visible sky bounds
			R_AddSkySurface (surf);
		}
		else if (surf->texInfo->flags & SURF_NODRAW)
			continue;
		else if (surf->texInfo->flags & (SURF_TRANS33 | SURF_TRANS66))
			continue;
		else
			scene_surfaces[num_depth_surfaces++] = surf;
	}

	// recurse down the back side
	R_RecursiveDepthWorldNode (node->children[!side]);
}

static void R_AddBModelDepthPolys (void) {
	int i;
	cplane_t *pplane;
	float dot;
	msurface_t *psurf;

	psurf = &currentmodel->surfaces[currentmodel->firstModelSurface];

	//
	// draw texture
	//

	for (i = 0; i < currentmodel->numModelSurfaces; i++, psurf++) {
		// find which side of the node we are on
		pplane = psurf->plane;
		dot = DotProduct (modelorg, pplane->normal) - pplane->dist;

		// draw the polygon
		if (((psurf->flags & MSURF_PLANEBACK) && (dot < -BACKFACE_EPSILON))
			|| (!(psurf->flags & MSURF_PLANEBACK) && (dot > BACKFACE_EPSILON))) {

			if (psurf->visframe == r_framecount)	// reckless fix
				continue;

			if (psurf->texInfo->flags & (SURF_TRANS33 | SURF_TRANS66)) {
				continue;
			}

			//		if (!(psurf->texInfo->flags & SURF_WARP))
			scene_surfaces[num_depth_surfaces++] = psurf;
		}
	}
}
void R_DrawDepthBrushModel (void) {
	vec3_t		mins, maxs;
	int			i;
	qboolean	rotated;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	if (currentmodel->numModelSurfaces == 0)
		return;

	gl_state.currenttextures[0] = gl_state.currenttextures[1] = -1;

	if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2]) {
		rotated = true;
		for (i = 0; i < 3; i++) {
			mins[i] = currententity->origin[i] - currentmodel->radius;
			maxs[i] = currententity->origin[i] + currentmodel->radius;
		}
	}
	else {
		rotated = false;
		VectorAdd (currententity->origin, currentmodel->mins, mins);
		VectorAdd (currententity->origin, currentmodel->maxs, maxs);
	}

	if (R_CullBox (mins, maxs))
		return;

	VectorSubtract (r_newrefdef.vieworg, currententity->origin, modelorg);

	if (rotated) {
		vec3_t temp;
		vec3_t forward, right, up;

		VectorCopy (modelorg, temp);
		AngleVectors (currententity->angles, forward, right, up);
		modelorg[0] = DotProduct (temp, forward);
		modelorg[1] = -DotProduct (temp, right);
		modelorg[2] = DotProduct (temp, up);
	}

	qglPushMatrix ();
	R_RotateForEntity (currententity);

	qglEnableVertexAttribArray (ATRB_POSITION);
	qglVertexAttribPointer (ATRB_POSITION, 3, GL_FLOAT, false, 0, wVertexArray);

	num_depth_surfaces = 0;
	R_AddBModelDepthPolys ();
	GL_DrawDepthPoly ();

	qglDisableVertexAttribArray (ATRB_POSITION);
	qglPopMatrix ();
}

void R_CalcAliasFrameLerp (dmdl_t *paliashdr, float shellScale);
static vec3_t	tempVertexArray[MAX_VERTICES * 4];

void GL_DrawAliasFrameLerpDepth (dmdl_t *paliashdr) {
	int				index_xyz;
	int				i, j, jj = 0;
	dtriangle_t		*tris;
	float			backlerp, frontlerp;
	daliasframe_t	*frame;
	dtrivertx_t		*verts;
	vec3_t			vertexArray[3 * MAX_TRIANGLES];

	if (currententity->flags & (RF_VIEWERMODEL))
		return;

	R_CalcAliasFrameLerp (paliashdr, 0);

	c_alias_polys += paliashdr->num_tris;

	tris = (dtriangle_t *)((byte *)paliashdr + paliashdr->ofs_tris);
	jj = 0;

	frame = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->frame * paliashdr->framesize);
	verts = frame->verts;
	backlerp = currententity->backlerp;
	frontlerp = 1 - backlerp;

	qglEnableVertexAttribArray (ATRB_POSITION);
	qglVertexAttribPointer (ATRB_POSITION, 3, GL_FLOAT, false, 0, vertexArray);

	for (i = 0; i < paliashdr->num_tris; i++) {
		for (j = 0; j < 3; j++, jj++) {
			index_xyz = tris[i].index_xyz[j];
			VectorCopy (tempVertexArray[index_xyz], vertexArray[jj]);

		}
	}
	qglDrawArrays (GL_TRIANGLES, 0, jj);
	qglDisableVertexAttribArray (ATRB_POSITION);
}

void R_DrawDepthAliasModel (void) {
	dmdl_t		*paliashdr;
	vec3_t		bbox[8];

	if (R_CullAliasModel (bbox, currententity))
		return;

	paliashdr = (dmdl_t *)currentmodel->extraData;

	if ((currententity->frame >= paliashdr->num_frames)
		|| (currententity->frame < 0)) {
		Com_Printf ("R_DrawAliasModel %s: no such frame %d\n",
			currentmodel->name, currententity->frame);
		currententity->frame = 0;
		currententity->oldframe = 0;
	}

	if ((currententity->oldframe >= paliashdr->num_frames)
		|| (currententity->oldframe < 0)) {
		Com_Printf ("R_DrawAliasModel %s: no such oldframe %d\n",
			currentmodel->name, currententity->oldframe);
		currententity->frame = 0;
		currententity->oldframe = 0;
	}

	qglPushMatrix ();
	R_RotateForEntity (currententity);

	GL_DrawAliasFrameLerpDepth (paliashdr);

	qglPopMatrix ();
}

void R_DrawDepthScene (void) {
	entity_t ent;
	int i;

	if (!r_drawWorld->value)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	currentmodel = r_worldmodel;

	VectorCopy (r_newrefdef.vieworg, modelorg);

	R_ClearSkyBox ();

	//	qglBindFramebuffer(GL_FRAMEBUFFER, gl_state.fboId);

	GL_BindProgram (nullProgram, 0);
	qglEnableVertexAttribArray (ATRB_POSITION);
	qglVertexAttribPointer (ATRB_POSITION, 3, GL_FLOAT, false, 0, wVertexArray);

	//	qglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	num_depth_surfaces = 0;
	R_RecursiveDepthWorldNode (r_worldmodel->nodes);
	GL_DrawDepthPoly ();

	qglDisableVertexAttribArray (ATRB_POSITION);

	R_DrawSkyBox (false);

	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];
		currentmodel = currententity->model;

		if (!currentmodel)
			continue;

		if (currententity->flags & RF_TRANSLUCENT)
			continue;

		if (currententity->flags & RF_WEAPONMODEL)
			continue;

		if (currentmodel->type == mod_brush)
			R_DrawDepthBrushModel ();

		if (currentmodel->type == mod_alias)
			R_DrawDepthAliasModel ();
	}

	//	qglBindFramebuffer(GL_FRAMEBUFFER, 0);
	//	qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	GL_BindNullProgram ();
}
