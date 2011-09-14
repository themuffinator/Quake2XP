/*
Copyright (C) 2004-2011 Quake2xp Team.

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

extern float shadowalpha;

typedef float vec4_t[4];
vec4_t s_lerped[MAX_VERTS];
float shadelight[3];

/*
===============
SHADOW VOLUMES
===============
*/
vec3_t		ShadowArray[MAX_SHADOW_VERTS];
unsigned	ShadowIndex[MAX_INDICES];

qboolean	triangleFacingLight	[MAX_INDICES / 3];


void GL_LerpVerts(int nverts, dtrivertx_t *v, dtrivertx_t *ov, dtrivertx_t *verts, float *lerp, float move[3], float frontv[3], float backv[3])
{
	int i;

		for (i = 0; i < nverts; i++, v++, ov++, lerp += 4) {
			lerp[0] = move[0] + ov->v[0]*backv[0] + v->v[0]*frontv[0];
			lerp[1] = move[1] + ov->v[1]*backv[1] + v->v[1]*frontv[1];
			lerp[2] = move[2] + ov->v[2]*backv[2] + v->v[2]*frontv[2];
		}
}

void R_MarkShadowTriangles(dmdl_t *paliashdr, dtriangle_t *tris, vec3_t lightOrg){
	
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
		VectorSubtract( v0, v1, dir0 );
		VectorSubtract( v2, v1, dir1 );
		
		CrossProduct( dir0, dir1, r_triangleNormals[i] );

		// Find front facing triangles
		VectorSubtract(lightOrg, v0, temp);
		f = DotProduct(temp, r_triangleNormals[i]);

		triangleFacingLight[i] = f > 0;
			
	}

}

void BuildShadowVolumeTriangles(dmdl_t * hdr, vec3_t light, float projectdistance)
{
	dtriangle_t *ot, *tris;
	neighbors_t *neighbors;
	int i, j, shadow_vert = 0, index = 0;
	vec3_t v0, v1, v2, v3;
	daliasframe_t *frame;
	dtrivertx_t *verts;
	
	frame = (daliasframe_t *) ((byte *) hdr + hdr->ofs_frames
							   + currententity->frame * hdr->framesize);
	verts = frame->verts;

	ot = tris = (dtriangle_t *) ((unsigned char *) hdr + hdr->ofs_tris);
	
	R_MarkShadowTriangles(hdr, tris, light);

	for (i = 0, tris = ot, neighbors = currentmodel->neighbors;
		 i < hdr->num_tris; i++, tris++, neighbors++) {
		if (!triangleFacingLight[i])
			continue;
	
		if (neighbors->n[0] < 0 || !triangleFacingLight[neighbors->n[0]]) {
			for (j = 0; j < 3; j++) {
				v0[j] = s_lerped[tris->index_xyz[1]][j];
				v1[j] = s_lerped[tris->index_xyz[0]][j];

				v2[j] = v1[j] + ((v1[j] - light[j]) * projectdistance);
				v3[j] = v0[j] + ((v0[j] - light[j]) * projectdistance);

			}

		VA_SetElem3(ShadowArray[shadow_vert+0], v0[0], v0[1], v0[2]);
		VA_SetElem3(ShadowArray[shadow_vert+1], v1[0], v1[1], v1[2]);
		VA_SetElem3(ShadowArray[shadow_vert+2], v2[0], v2[1], v2[2]);
		VA_SetElem3(ShadowArray[shadow_vert+3], v3[0], v3[1], v3[2]);
            

		ShadowIndex[index++] = shadow_vert+0;
		ShadowIndex[index++] = shadow_vert+1;
		ShadowIndex[index++] = shadow_vert+3;
		ShadowIndex[index++] = shadow_vert+3;
		ShadowIndex[index++] = shadow_vert+1;
		ShadowIndex[index++] = shadow_vert+2;
		shadow_vert +=4;
		}

		if (neighbors->n[1] < 0 || !triangleFacingLight[neighbors->n[1]]) {
			for (j = 0; j < 3; j++) {
				v0[j] = s_lerped[tris->index_xyz[2]][j];
				v1[j] = s_lerped[tris->index_xyz[1]][j];

				v2[j] = v1[j] + ((v1[j] - light[j]) * projectdistance);
				v3[j] = v0[j] + ((v0[j] - light[j]) * projectdistance);
			}

		VA_SetElem3(ShadowArray[shadow_vert+0], v0[0], v0[1], v0[2]);
		VA_SetElem3(ShadowArray[shadow_vert+1], v1[0], v1[1], v1[2]);
		VA_SetElem3(ShadowArray[shadow_vert+2], v2[0], v2[1], v2[2]);
		VA_SetElem3(ShadowArray[shadow_vert+3], v3[0], v3[1], v3[2]);
            

		ShadowIndex[index++] = shadow_vert+0;
		ShadowIndex[index++] = shadow_vert+1;
		ShadowIndex[index++] = shadow_vert+3;
		ShadowIndex[index++] = shadow_vert+3;
		ShadowIndex[index++] = shadow_vert+1;
		ShadowIndex[index++] = shadow_vert+2;
		shadow_vert +=4;
		}

		if (neighbors->n[2] < 0 || !triangleFacingLight[neighbors->n[2]]) {
			for (j = 0; j < 3; j++) {
				v0[j] = s_lerped[tris->index_xyz[0]][j];
				v1[j] = s_lerped[tris->index_xyz[2]][j];

				v2[j] = v1[j] + ((v1[j] - light[j]) * projectdistance);
				v3[j] = v0[j] + ((v0[j] - light[j]) * projectdistance);
			}

	
		VA_SetElem3(ShadowArray[shadow_vert+0], v0[0], v0[1], v0[2]);
		VA_SetElem3(ShadowArray[shadow_vert+1], v1[0], v1[1], v1[2]);
		VA_SetElem3(ShadowArray[shadow_vert+2], v2[0], v2[1], v2[2]);
		VA_SetElem3(ShadowArray[shadow_vert+3], v3[0], v3[1], v3[2]);
            

		ShadowIndex[index++] = shadow_vert+0;
		ShadowIndex[index++] = shadow_vert+1;
		ShadowIndex[index++] = shadow_vert+3;
		ShadowIndex[index++] = shadow_vert+3;
		ShadowIndex[index++] = shadow_vert+1;
		ShadowIndex[index++] = shadow_vert+2;
		shadow_vert +=4;
		}
	}

	
	 // build shadows caps from backfacing triangles

	for (i = 0, tris = ot; i < hdr->num_tris; i++, tris++) {
	
		if (!triangleFacingLight[i])
			continue;
		
			for (j = 0; j < 3; j++) {
				v0[j] = s_lerped[tris->index_xyz[0]][j];
				v1[j] = s_lerped[tris->index_xyz[1]][j];
				v2[j] = s_lerped[tris->index_xyz[2]][j];
			}

		VA_SetElem3(ShadowArray[shadow_vert+0], v0[0], v0[1], v0[2]);
		VA_SetElem3(ShadowArray[shadow_vert+1], v1[0], v1[1], v1[2]);
		VA_SetElem3(ShadowArray[shadow_vert+2], v2[0], v2[1], v2[2]);
			
		ShadowIndex[index++] = shadow_vert+0;
		ShadowIndex[index++] = shadow_vert+1;
		ShadowIndex[index++] = shadow_vert+2;
        shadow_vert +=3;
			
			// rear cap (with flipped winding order)

			for (j = 0; j < 3; j++) {
				v0[j] = s_lerped[tris->index_xyz[0]][j];
				v1[j] = s_lerped[tris->index_xyz[1]][j];
				v2[j] = s_lerped[tris->index_xyz[2]][j];


				v0[j] = v0[j] + ((v0[j] - light[j]) * projectdistance);
				v1[j] = v1[j] + ((v1[j] - light[j]) * projectdistance);
				v2[j] = v2[j] + ((v2[j] - light[j]) * projectdistance);

			}
			
		VA_SetElem3(ShadowArray[shadow_vert+0], v0[0], v0[1], v0[2]);
		VA_SetElem3(ShadowArray[shadow_vert+1], v1[0], v1[1], v1[2]);
		VA_SetElem3(ShadowArray[shadow_vert+2], v2[0], v2[1], v2[2]);
				 
		ShadowIndex[index++] = shadow_vert+2; 
		ShadowIndex[index++] = shadow_vert+1; 
		ShadowIndex[index++] = shadow_vert+0; 
		shadow_vert +=3;
	}
	
	if(gl_state.DrawRangeElements && r_DrawRangeElements->value)
		qglDrawRangeElementsEXT(GL_TRIANGLES, 0, shadow_vert, index, GL_UNSIGNED_INT, ShadowIndex);
			else
		qglDrawElements(GL_TRIANGLES, index, GL_UNSIGNED_INT, ShadowIndex);
			
	c_shadow_tris += index/3;
}


// openGL 2.0 Unified Separate Stencil
void GL_RenderVolumesGL20(dmdl_t * paliashdr, vec3_t lightdir, int projdist)
{
		
	if(VectorCompare(lightdir, vec3_origin))
		return;

		qglDisable(GL_CULL_FACE);

		qglStencilOpSeparate(GL_BACK, GL_KEEP,  GL_INCR_WRAP_EXT, GL_KEEP);
		qglStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP_EXT, GL_KEEP);
		
		BuildShadowVolumeTriangles(paliashdr, lightdir, projdist);
		
		qglEnable(GL_CULL_FACE);
	
}


void GL_RenderVolumes(dmdl_t * paliashdr, vec3_t lightdir, int projdist){

	GL_RenderVolumesGL20(paliashdr, lightdir, projdist);
	c_shadow_volumes++;
}


void GL_DrawAliasShadowVolume(dmdl_t * paliashdr, int posenumm)
{
	vec3_t		light, temp;
	int			i, worldlight = 0, dlight = 0;
	dlight_t	*dl;
	flare_t		*lightSurf;
	float		dist;
	mat3_t		entityAxis;
	trace_t		r_trace;

	if (r_newrefdef.vieworg[2] < (currententity->origin[2] - 10))
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
	
	VectorClear(light);
	
//============================================================================
	dl = r_newrefdef.dlights;

	if (r_shadows->value >3) {

		for (i = 0; i < r_newrefdef.num_dlights; i++, dl++) {
			
			if ((dl->origin[0] == currententity->origin[0]) &&
		    (dl->origin[1] == currententity->origin[1]) &&
		    (dl->origin[2] == currententity->origin[2]))
		        continue;

			VectorSubtract(currententity->origin, dl->origin, temp);
			dist = VectorLength(temp);

			if (dist > dl->intensity)
				continue;		// big distance!

			// light surf behind the wall 
			if (r_newrefdef.areabits){
			r_trace = CM_BoxTrace(currententity->origin, dl->origin, vec3_origin, vec3_origin, r_worldmodel->firstnode, MASK_OPAQUE);
			if(r_trace.fraction != 1.0)
			continue;
			}

			AnglesToMat3(currententity->angles, entityAxis);
			VectorSubtract(dl->origin, currententity->origin, temp);
			Mat3_TransposeMultiplyVector(entityAxis, temp, light);	
												
			GL_RenderVolumes(paliashdr, light, 15);
			worldlight++;
			dlight++;
		}
	}
//============================================================================

	if (r_shadows->value >2) {

		for (i = 0; i < r_numflares; i++) {
			int sidebit;
			float viewplane;
						
			lightSurf = &r_flares[i];
			
			if(dlight)
				continue;
			
			// PVS coolling 
			if (r_newrefdef.areabits){
				if (!(r_newrefdef.areabits[lightSurf->area >> 3] & (1 << (lightSurf->area & 7)))){
					continue;
				}
			}

			VectorSubtract(lightSurf->origin, currententity->origin, temp);

			dist = VectorNormalize(temp);

			if (dist > lightSurf->size * r_shadowWorldLightScale->value)
				continue;		// big distance!
			
			if(lightSurf->ignore)
				continue;

			viewplane = DotProduct(currententity->origin, lightSurf->surf->plane->normal) - lightSurf->surf->plane->dist;
			
			if (viewplane >= 0)
				sidebit = 0;
			else
				sidebit = SURF_PLANEBACK;

			if ((lightSurf->surf->flags & SURF_PLANEBACK) != sidebit)
					continue;		// wrong light poly side!
			
		
			// light surf behind the wall 
			if (r_newrefdef.areabits){
			r_trace = CM_BoxTrace(currententity->origin, lightSurf->origin, vec3_origin, vec3_origin, r_worldmodel->firstnode, MASK_OPAQUE);
			if(r_trace.fraction != 1.0)
			continue;
			}
			AnglesToMat3(currententity->angles, entityAxis);
			VectorSubtract(lightSurf->origin, currententity->origin, temp);
			Mat3_TransposeMultiplyVector(entityAxis, temp, light);	
			light[2] += currententity->model->maxs[2] + 56;
			worldlight++;
			
		}
	}
//============================================================================
	if (!worldlight) {

		VectorSet(light, currententity->origin[0], currententity->origin[1], currententity->origin[2]);
		light[2] += 200;
		AnglesToMat3(currententity->angles, entityAxis);
		VectorSubtract(light, currententity->origin, temp);
		Mat3_TransposeMultiplyVector(entityAxis, temp, light);	
	
		GL_RenderVolumes(paliashdr, light, 3);

	}
	if (worldlight && !dlight)
		GL_RenderVolumes(paliashdr, light, 5);

	
}




/*==============
Vis's CullSphere
==============*/

qboolean R_CullSphere( const vec3_t centre, const float radius, const int clipflags )
{
	int		i;
	cplane_t *p;

	if (r_noCull->value)
		return false;

	for (i=0,p=frustum ; i<4; i++,p++)
	{
		if ( !(clipflags & (1<<i)) ) {
			continue;
		}

		if ( DotProduct ( centre, p->normal ) - p->dist <= -radius )
			return true;
	}

	return false;
}

int CL_PMpointcontents(vec3_t point);

void R_DrawShadowVolume(entity_t * e)
{
	dmdl_t			*paliashdr;
	daliasframe_t	*frame, *oldframe;
	dtrivertx_t		*v, *ov, *verts;
	int				*order, i;
	float			frontlerp, rad;
	vec3_t			move, delta, vectors[3], frontv, backv, tmp, water;
	trace_t			tr;
		
	VectorAdd(e->origin, currententity->model->maxs, water); 
	if(CL_PMpointcontents(water) & MASK_WATER)
		return;
	
	VectorSubtract(currententity->model->maxs, currententity->model->mins, tmp);
	VectorScale (tmp, 1.666, tmp); // Hail Satan!
	rad = VectorLength (tmp);
	
	if( R_CullSphere( e->origin, rad, 15 ) )
		return;
		

	if (r_newrefdef.areabits){
		tr = CM_BoxTrace(r_origin, e->origin, vec3_origin, vec3_origin, r_worldmodel->firstnode, MASK_OPAQUE);
		if(tr.fraction != 1.0)
			return;
	}

	paliashdr = (dmdl_t *) currentmodel->extradata;

	frame = (daliasframe_t *) ((byte *) paliashdr   + paliashdr->ofs_frames
						        + currententity->frame *
							  paliashdr->framesize);
	verts = v = frame->verts;

	oldframe =
		(daliasframe_t *) ((byte *) paliashdr + paliashdr->ofs_frames +
						   currententity->oldframe * paliashdr->framesize);
	ov = oldframe->verts;

	order = (int *) ((byte *) paliashdr + paliashdr->ofs_glcmds);

	frontlerp = 1.0 - currententity->backlerp;

	// move should be the delta back to the previous frame * backlerp
	VectorSubtract(currententity->oldorigin, currententity->origin, delta);
	AngleVectors(currententity->angles, vectors[0], vectors[1],
				 vectors[2]);

	move[0] = DotProduct(delta, vectors[0]);	// forward
	move[1] = -DotProduct(delta, vectors[1]);	// left
	move[2] = DotProduct(delta, vectors[2]);	// up

	VectorAdd(move, oldframe->translate, move);

	for (i = 0; i < 3; i++) {
		move[i] =
			currententity->backlerp * move[i] +
			frontlerp * frame->translate[i];
		frontv[i] = frontlerp * frame->scale[i];
		backv[i] = currententity->backlerp * oldframe->scale[i];
	}

	GL_LerpVerts(paliashdr->num_xyz, v, ov, verts, s_lerped[0], move,
				 frontv, backv);


	if (r_shadows->value > 1 ) {
		currententity->angles[PITCH] = -currententity->angles[PITCH];
		qglPushMatrix();
		qglDisable(GL_TEXTURE_2D);
		qglTranslatef(e->origin[0], e->origin[1], e->origin[2]);
		qglRotatef(e->angles[1], 0, 0, 1);
		
		GL_DrawAliasShadowVolume(paliashdr, currententity->frame);
		
		qglEnable(GL_TEXTURE_2D);
		qglPopMatrix();
		currententity->angles[PITCH] = -currententity->angles[PITCH];

	}


}

extern cvar_t *r_shadowAlpha;

/*=====================
Shadow World Triangles
=====================*/

vec3_t sVertexArray[MAX_BATCH_SURFS];
static vec3_t modelorg;			// relative to viewpoint
void R_DrawArrays (void);

void GL_DrawShadowTriangles(msurface_t * surf)
{
    glpoly_t *p= surf->polys;
    float *v;
    int i, nv;
       
    nv = surf->polys->numverts;

            v = p->verts[0];
			c_shadow_tris += numIndeces / 3;

			for (i=0; i < nv-2; i++) {
			indexArray[numIndeces++] = numVertices;
			indexArray[numIndeces++] = numVertices+i+1;
			indexArray[numIndeces++] = numVertices+i+2;
			}

			for (i = 0; i < nv; i++, v += VERTEXSIZE){

                VectorCopy(v, wVertexArray[i]);
				numVertices++;
			}
       
	R_DrawArrays();
    	
}


void R_DrawBModelShadow(void)
{
	int i;
	cplane_t *pplane;
	float dot;
	msurface_t *psurf;
	
	psurf = &currentmodel->surfaces[currentmodel->firstmodelsurface];

	for (i = 0; i < currentmodel->nummodelsurfaces; i++, psurf++) {
		// find which side of the node we are on
		pplane = psurf->plane;

		dot = DotProduct(modelorg, pplane->normal) - pplane->dist;
		
		// draw the polygon
		if (((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON))
			|| (!(psurf->flags & SURF_PLANEBACK)
				&& (dot > BACKFACE_EPSILON))) {
			
				GL_DrawShadowTriangles(psurf);
			
		}
	}
	
}


void R_DrawBrushModelShadow(entity_t * e)
{
	vec3_t mins, maxs;
	int i;
	qboolean rotated;
	
	if (currentmodel->nummodelsurfaces == 0)
		return;

	currententity = e;
	
	if (e->angles[0] || e->angles[1] || e->angles[2]) {
		rotated = true;
		for (i = 0; i < 3; i++) {
			mins[i] = e->origin[i] - currentmodel->radius;
			maxs[i] = e->origin[i] + currentmodel->radius;
		}
	} else {
		rotated = false;
		VectorAdd(e->origin, currentmodel->mins, mins);
		VectorAdd(e->origin, currentmodel->maxs, maxs);
	}

	if (R_CullBox(mins, maxs))
		return;


	VectorSubtract(r_newrefdef.vieworg, e->origin, modelorg);

	if (rotated) {
		vec3_t temp;
		vec3_t forward, right, up;

		VectorCopy(modelorg, temp);
		AngleVectors(e->angles, forward, right, up);
		modelorg[0] = DotProduct(temp, forward);
		modelorg[1] = -DotProduct(temp, right);
		modelorg[2] = DotProduct(temp, up);
	}

	qglPushMatrix();
	e->angles[0] = -e->angles[0];	// stupid quake bug
	e->angles[2] = -e->angles[2];	// stupid quake bug
	R_RotateForEntity(e);
	e->angles[0] = -e->angles[0];	// stupid quake bug
	e->angles[2] = -e->angles[2];	// stupid quake bug



	R_DrawBModelShadow();
	

	qglPopMatrix();
	
}



void R_RecursiveShadowWorldNode(mnode_t * node)
{
	int c, side, sidebit;
	cplane_t *plane;
	msurface_t *surf, **mark;
	mleaf_t *pleaf;
	float dot, viewplane;

	if (node->contents == CONTENTS_SOLID)
		return;					// solid

	if (node->visframe != r_visframecount)
		return;

	if (R_CullBox(node->minmaxs, node->minmaxs + 3))
		return;
	
	
	// if a leaf node, draw stuff
	if (node->contents != -1) {
		pleaf = (mleaf_t *) node;

		// check for door connected areas
		if (r_newrefdef.areabits) {
			if (!(r_newrefdef.areabits[pleaf->area >> 3] & (1 << (pleaf->area & 7))))
				return;			// not visible
		}

		mark = pleaf->firstmarksurface;
		c = pleaf->nummarksurfaces;

		if (c) {
			do {
			if (SurfInFrustum(*mark))
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
		dot = DotProduct(modelorg, plane->normal) - plane->dist;
		break;
	}

	if (dot >= 0) 
		side = 0;
	else 
		side = 1;


	// recurse down the children, front side first
	R_RecursiveShadowWorldNode(node->children[side]);

	// draw stuff
	for (c = node->numsurfaces, surf = r_worldmodel->surfaces + node->firstsurface; c; c--, surf++) {
		
		if (surf->visframe != r_framecount)
			continue;
		
		// SURF_PLANEBACK culling fix
		viewplane = DotProduct(r_origin, surf->plane->normal) - surf->plane->dist;
		if (viewplane >= 0)
			sidebit = 0;
		else
			sidebit = SURF_PLANEBACK;

		if ((surf->flags & SURF_PLANEBACK) != sidebit)
			continue;			// wrong side
		
		if (surf->texinfo->flags & (SURF_TRANS33 | SURF_TRANS66 | SURF_NODRAW | SURF_SKY))
			continue;
		
		GL_DrawShadowTriangles(surf);
		
	}

	// recurse down the back side
	R_RecursiveShadowWorldNode(node->children[!side]);
}



void R_DrawShadowWorld(void)
{
	int i;
	float shadowalpha;

	if (!r_drawWorld->value)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
	
	if(r_shadows->value < 2 )
		return;

	if(r_newrefdef.rdflags & RDF_IRGOGGLES)
		return;

	qglEnableVertexAttribArray(ATRB_POSITION);
	qglVertexAttribPointer(ATRB_POSITION, 3, GL_FLOAT, false, 0, wVertexArray);
	
	qglEnable(GL_STENCIL_TEST);
    qglStencilFunc(GL_NOTEQUAL, 128, 255);
	qglClear(GL_STENCIL_BUFFER_BIT);
    qglStencilMask(0);
	qglDepthMask(0);
	
	qglDisable(GL_TEXTURE_2D);
	GL_SelectTexture(GL_TEXTURE0_ARB);
    qglEnable(GL_BLEND);
    qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	shadowalpha = 1.0 - r_ambientLevel->value;
	
	qglColor4f(0, 0, 0, shadowalpha);    

	R_RecursiveShadowWorldNode(r_worldmodel->nodes);
	

	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];
		currentmodel = currententity->model;
		
		if (!currentmodel)
			continue;
		
		if (currentmodel->type != mod_brush)
			continue;

		R_DrawBrushModelShadow(currententity);
	}

	qglDisableVertexAttribArray(ATRB_POSITION);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	qglColor4f(1, 1, 1, 1);    
	qglDisable(GL_BLEND);
	qglEnable(GL_TEXTURE_2D); 
	qglDisable(GL_STENCIL_TEST);
	qglDepthMask(1);
    		
}
/*
==================
Simple blob shadow
==================
*/
void MakeNormalVectors(vec3_t forward, vec3_t right, vec3_t up);
trace_t CL_PMTraceWorld(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int mask);
#define MAX_BLOB_SHADOW_VERT 4096*4

void R_BlobShadow(void){

	vec3_t			end, mins = {-15, -15, 0}, maxs = {15, 15, 2};
	trace_t			trace;
	float			alpha, dist;
	int				sVert=0, index=0, i;
	unsigned		sIndex[MAX_INDICES];
	vec3_t			axis[3];
	vec3_t			bbox[8], temp;
	vec4_t			bsColor[MAX_BLOB_SHADOW_VERT];
	vec3_t			bsVert[MAX_BLOB_SHADOW_VERT];
	vec2_t			bsTextCoord[MAX_BLOB_SHADOW_VERT];

	if(r_shadows->value != 1)
		return;
		
	GL_Overbrights			(false);
	qglEnableClientState	(GL_TEXTURE_COORD_ARRAY);
	qglTexCoordPointer		(2, GL_FLOAT, 0, bsTextCoord);
	qglEnableClientState	(GL_COLOR_ARRAY);
	qglColorPointer			(4, GL_FLOAT, 0, bsColor);
	qglEnableClientState	(GL_VERTEX_ARRAY);
	qglVertexPointer		(3, GL_FLOAT, 0, bsVert);
	
	qglEnable				(GL_POLYGON_OFFSET_FILL);
    qglPolygonOffset		(-2, -1);
	qglEnable				(GL_BLEND);
	qglBlendFunc			(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	qglDepthMask(0);
	GL_MBind				(GL_TEXTURE0_ARB, r_particletexture[PT_DEFAULT]->texnum);

	for (i = 0; i < r_newrefdef.num_entities; i++)	
	{
		
		currententity = &r_newrefdef.entities[i];
		currentmodel = currententity->model;
		
		if (!currentmodel)
			continue;
		
		if (currentmodel->type != mod_alias)
			continue;

		if (currententity->
		flags & (RF_SHELL_HALF_DAM | RF_SHELL_GREEN | RF_SHELL_RED |
				 RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_GOD |
				 RF_TRANSLUCENT | RF_BEAM | RF_WEAPONMODEL | RF_NOSHADOW | RF_DISTORT))
				 continue;
		
		if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
			continue;
	
		if (R_CullSphere(currententity->origin, 35, 15))
			continue;
		
		VectorCopy(currententity->origin, end);
		end[2] -= 128;

		trace = CL_PMTraceWorld (currententity->origin, vec3_origin, vec3_origin, end, MASK_SOLID);
				
		if (trace.fraction > 0 && trace.fraction < 1){
				
		if(sVert){
		if(gl_state.DrawRangeElements && r_DrawRangeElements->value)
			qglDrawRangeElementsEXT(GL_TRIANGLES, 0, sVert, index, GL_UNSIGNED_INT, sIndex);
		else
			qglDrawElements(GL_TRIANGLES, index, GL_UNSIGNED_INT, sIndex);

		index = 0;
		sVert = 0;
		}
			alpha = 0.7 - trace.fraction; 
						
			// Find axes
			VectorCopy(trace.plane.normal, axis[0]);
			MakeNormalVectors(axis[0], axis[1], axis[2]);
			
			//find model radiuse
			R_CullAliasModel(bbox, currententity);
			VectorSubtract(bbox[0], bbox[2], temp);
			dist = VectorNormalize(temp);

			// Scale the axes by radius
			VectorScale(axis[1], dist, axis[1]);
			VectorScale(axis[2], dist, axis[2]);

			// Build by surface aligned quad
			VA_SetElem2(bsTextCoord[sVert+0],0, 1);
			VA_SetElem3(bsVert[sVert+0],	trace.endpos[0] + axis[1][0] + axis[2][0],
											trace.endpos[1] + axis[1][1] + axis[2][1],
											trace.endpos[2] + axis[1][2] + axis[2][2]);
			VA_SetElem4(bsColor[sVert+0],	0.0, 0.0, 0.0, alpha);

			VA_SetElem2(bsTextCoord[sVert+1],0, 0);
			VA_SetElem3(bsVert[sVert+1],	trace.endpos[0] - axis[1][0] + axis[2][0],
											trace.endpos[1] - axis[1][1] + axis[2][1],
											trace.endpos[2] - axis[1][2] + axis[2][2]);
			VA_SetElem4(bsColor[sVert+1],	0.0, 0.0, 0.0, alpha);

			VA_SetElem2(bsTextCoord[sVert+2],1, 0);
			VA_SetElem3(bsVert[sVert+2],	trace.endpos[0] - axis[1][0] - axis[2][0],
											trace.endpos[1] - axis[1][1] - axis[2][1],
											trace.endpos[2] - axis[1][2] - axis[2][2]);
			VA_SetElem4(bsColor[sVert+2],	0.0, 0.0, 0.0, alpha);

			VA_SetElem2(bsTextCoord[sVert+3],1, 1);
			VA_SetElem3(bsVert[sVert+3],	trace.endpos[0] + axis[1][0] - axis[2][0],
											trace.endpos[1] + axis[1][1] - axis[2][1],
											trace.endpos[2] + axis[1][2] - axis[2][2]);
			VA_SetElem4(bsColor[sVert+3],	0.0, 0.0, 0.0, alpha);
			
			sIndex[index++] = sVert+0;
			sIndex[index++] = sVert+1;
			sIndex[index++] = sVert+3;
			sIndex[index++] = sVert+3;
			sIndex[index++] = sVert+1;
			sIndex[index++] = sVert+2;

			sVert+=4;
	}

	if(sVert)
	{
		if(gl_state.DrawRangeElements && r_DrawRangeElements->value)
			qglDrawRangeElementsEXT(GL_TRIANGLES, 0, sVert, index, GL_UNSIGNED_INT, sIndex);
		else
			qglDrawElements(GL_TRIANGLES, index, GL_UNSIGNED_INT, sIndex);
	}
	
	}
	qglDisable				(GL_POLYGON_OFFSET_FILL);
	qglDisable				(GL_BLEND);
	qglDepthMask			(1);
	qglDisableClientState	(GL_TEXTURE_COORD_ARRAY);
	qglDisableClientState	(GL_COLOR_ARRAY);
	qglDisableClientState	(GL_VERTEX_ARRAY);
	
}
