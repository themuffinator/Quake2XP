#include "r_local.h"

extern msurface_t	*scene_surfaces[MAX_MAP_FACES];
static int			num_depth_surfaces;
static vec3_t		modelorg;			// relative to viewpoint

qboolean R_FillDepthBatch (msurface_t *surf, unsigned *vertices, unsigned *indeces) {
	unsigned	numVertices, numIndices;
	int			i, nv = surf->polys->numVerts;

	numVertices = *vertices;
	numIndices = *indeces;

	if (numVertices + nv > MAX_BATCH_SURFS)
		return qfalse;

	// create indexes
	if (numIndices == 0xffffffff)
		numIndices = 0;

	for (i = 0; i < nv - 2; i++)
	{
		indexArray[numIndices++] = surf->baseIndex;
		indexArray[numIndices++] = surf->baseIndex + i + 1;
		indexArray[numIndices++] = surf->baseIndex + i + 2;
	}

	c_brush_polys += (nv - 2);
	
	*vertices = numVertices;
	*indeces = numIndices;

	return qtrue;
}

static void GL_DrawDepthPoly () {
	msurface_t	*s;
	int			i;
	unsigned	numIndices = 0xffffffff;
	unsigned	numVertices = 0;

	for (i = 0; i < num_depth_surfaces; i++) {
		s = scene_surfaces[i];

		if (!R_FillDepthBatch (s, &numVertices, &numIndices)) {
			if (numIndices != 0xFFFFFFFF) {
				qglDrawElements (GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
				numVertices = 0;
				numIndices = 0xFFFFFFFF;
			}
		}
	}

	// draw the rest
	if (numIndices != 0xFFFFFFFF)
		qglDrawElements (GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indexArray);
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

	for (i = 0; i < currentmodel->numModelSurfaces; i++, psurf++) {
		// find which side of the node we are on
		pplane = psurf->plane;
		if (pplane->type < 3)
			dot = modelorg[pplane->type] - pplane->dist;
		else
			dot = DotProduct (modelorg, pplane->normal) - pplane->dist;

		// draw the polygon
		if (((psurf->flags & MSURF_PLANEBACK) && (dot < -BACKFACE_EPSILON))
			|| (!(psurf->flags & MSURF_PLANEBACK) && (dot > BACKFACE_EPSILON))) {

			if (psurf->visframe == r_framecount)	// reckless fix
				continue;

			if (psurf->texInfo->flags & (SURF_TRANS33 | SURF_TRANS66)) {
				continue;
			}
			scene_surfaces[num_depth_surfaces++] = psurf;
		}
	}
}
void R_DrawDepthBrushModel (void) {
	vec3_t		mins, maxs;
	int			i;
	qboolean	rotated;
	mat4_t		mvp;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	if (!r_drawEntities->value)
		return;

	if (currentmodel->numModelSurfaces == 0)
		return;

	if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2]) {
		rotated = qtrue;
		for (i = 0; i < 3; i++) {
			mins[i] = currententity->origin[i] - currentmodel->radius;
			maxs[i] = currententity->origin[i] + currentmodel->radius;
		}
	}
	else {
		rotated = qfalse;
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

	R_SetupEntityMatrix (currententity);

	Mat4_TransposeMultiply(currententity->matrix, r_newrefdef.modelViewProjectionMatrix, mvp);
	qglUniformMatrix4fv(null_mvp, 1, qfalse, (const float *)mvp);

	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo.vbo_BSP);
	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, BUFFER_OFFSET(vbo.xyz_offset));

	num_depth_surfaces = 0;
	R_AddBModelDepthPolys ();
	GL_DrawDepthPoly();

	qglDisableVertexAttribArray (ATT_POSITION);
	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

void R_CalcAliasFrameLerp (dmdl_t *paliashdr, float shellScale);
extern vec3_t	tempVertexArray[MAX_VERTICES * 4];

void GL_DrawAliasFrameLerpDepth(dmdl_t *paliashdr) {
	vec3_t		vertexArray[3 * MAX_TRIANGLES];
	int			index_xyz;
	int			i, j, jj = 0;
	dtriangle_t	*tris;

	if (currententity->flags & (RF_VIEWERMODEL))
		return;


	R_CalcAliasFrameLerp(paliashdr, 0);			/// Просто сюда переместили вычисления Lerp...

	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, vertexArray);
	Mat4_TransposeMultiply(currententity->matrix, r_newrefdef.modelViewProjectionMatrix, currententity->orMatrix);
	qglUniformMatrix4fv(null_mvp, 1, qfalse, (const float *)currententity->orMatrix);

	c_alias_polys += paliashdr->num_tris;
	tris = (dtriangle_t *)((byte *)paliashdr + paliashdr->ofs_tris);
	jj = 0;

	for (i = 0; i < paliashdr->num_tris; i++) {
		for (j = 0; j < 3; j++, jj++) {
			index_xyz = tris[i].index_xyz[j];
			VectorCopy(tempVertexArray[index_xyz], vertexArray[jj]);
		}
	}

	qglDrawArrays(GL_TRIANGLES, 0, jj);

	qglDisableVertexAttribArray(ATT_POSITION);
}

void R_DrawDepthAliasModel(void){

	dmdl_t		*paliashdr;
	vec3_t		bbox[8];
	
	if (!r_drawEntities->value)
		return;
	
	if (R_CullAliasModel(bbox, currententity))
		return;

	paliashdr = (dmdl_t *)currentmodel->extraData;

	if ((currententity->frame >= paliashdr->num_frames)
		|| (currententity->frame < 0)) {
		Com_Printf("R_DrawAliasModel %s: no such frame %d\n",
			currentmodel->name, currententity->frame);
		currententity->frame = 0;
		currententity->oldframe = 0;
	}

	if ((currententity->oldframe >= paliashdr->num_frames)
		|| (currententity->oldframe < 0)) {
		Com_Printf("R_DrawAliasModel %s: no such oldframe %d\n",
			currentmodel->name, currententity->oldframe);
		currententity->frame = 0;
		currententity->oldframe = 0;
	}

	R_SetupEntityMatrix(currententity);

	GL_DrawAliasFrameLerpDepth(paliashdr);
}

void R_DrawDepthScene (void) {

	int i;

	if (!r_drawWorld->value)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	currentmodel = r_worldmodel;

	VectorCopy (r_newrefdef.vieworg, modelorg);

	R_ClearSkyBox ();

	GL_BindProgram (nullProgram, 0);

	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo.vbo_BSP);
	qglEnableVertexAttribArray (ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, BUFFER_OFFSET(vbo.xyz_offset));
	qglUniformMatrix4fv(null_mvp, 1, qfalse, (const float *)r_newrefdef.modelViewProjectionMatrix);
//	qglPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //debug tool

	num_depth_surfaces = 0;
	R_RecursiveDepthWorldNode (r_worldmodel->nodes);
	GL_DrawDepthPoly ();
	
	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	qglDisableVertexAttribArray (ATT_POSITION);

	R_DrawSkyBox (qfalse);

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

		if (currentmodel->type == mod_brush)
			R_DrawDepthBrushModel ();

		if (currentmodel->type == mod_alias)
			R_DrawDepthAliasModel ();
	}
//	qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	GL_BindNullProgram ();
}
