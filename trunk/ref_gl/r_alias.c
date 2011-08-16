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
// r_alias_md2.c: triangle model functions

//willow: radar bips!
#include "r_local.h"
#include "..\client\client.h"
#include "..\client\sound.h"

/*
=============================================================
ALIAS MODELS
=============================================================
*/


#define NUMVERTEXNORMALS	162

static float	r_avertexnormals[NUMVERTEXNORMALS][3] = {
#include "anorms.h"
};

vec4_t		s_lerped[MAX_VERTS];
float		shadelight[3];

// precalculated dot products for quantized angles
#define		SHADEDOT_QUANT 16
float		r_avertexnormal_dots[SHADEDOT_QUANT][256]= 
#include	"anormtab.h"
;
float		*shadedots = r_avertexnormal_dots[0];

float		ref_realtime =0;



void GL_LerpVerts(int nverts, dtrivertx_t *v, dtrivertx_t *ov, dtrivertx_t *verts, float *lerp, float move[3], float frontv[3], float backv[3], float shellscale)
{
	int i;

	//PMM -- added RF_SHELL_DOUBLE, RF_SHELL_HALF_DAM
	if (currententity->flags & (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_GOD)) {
		for (i=0 ; i < nverts; i++, v++, ov++, lerp+=4 ) {
			float *normal = r_avertexnormals[verts[i].lightnormalindex];
			lerp[0] = move[0] + ov->v[0]*backv[0] + v->v[0]*frontv[0] + normal[0] * shellscale;
			lerp[1] = move[1] + ov->v[1]*backv[1] + v->v[1]*frontv[1] + normal[1] * shellscale;
			lerp[2] = move[2] + ov->v[2]*backv[2] + v->v[2]*frontv[2] + normal[2] * shellscale;
		}
	} else {
		for (i = 0; i < nverts; i++, v++, ov++, lerp += 4) {
			lerp[0] = move[0] + ov->v[0]*backv[0] + v->v[0]*frontv[0];
			lerp[1] = move[1] + ov->v[1]*backv[1] + v->v[1]*frontv[1];
			lerp[2] = move[2] + ov->v[2]*backv[2] + v->v[2]*frontv[2];
		}
	}

}

float Md2VertArray[MAX_TRIANGLES*4][3];
float Md2TexArray[MAX_TRIANGLES*4][2];
float Md2ColorArray[MAX_TRIANGLES*4][4];

void GL_DrawAliasFrameLerpDistort (dmdl_t *paliashdr, float backlerp)
{
	daliasframe_t	*frame, *oldframe;
	dtrivertx_t	*v, *ov, *verts;
	int		*order;
	int		count;
	float	frontlerp, alpha;
	vec3_t	move, delta, vectors[3];
	vec3_t	frontv, backv;
	int		i;
	int		index_xyz;
	float	*lerp;
	int aliasArray = 0;


	if (currententity->flags & (RF_VIEWERMODEL))
			return;
	
	qglDisable(GL_BLEND);
	
	if (currententity->flags & RF_TRANSLUCENT)
		alpha = currententity->alpha;
	else
		alpha = 1.0;

	frame = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames
		+ currententity->frame * paliashdr->framesize);
	verts = v = frame->verts;

	oldframe = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames
		+ currententity->oldframe * paliashdr->framesize);
	ov = oldframe->verts;

	order = (int *)((byte *)paliashdr + paliashdr->ofs_glcmds);

	
	frontlerp = 1.0 - backlerp;
	
	c_alias_polys += paliashdr->num_tris;
		

	// move should be the delta back to the previous frame * backlerp
	VectorSubtract(currententity->oldorigin, currententity->origin, delta);
	AngleVectors(currententity->angles, vectors[0], vectors[1], vectors[2]);

	move[0] = DotProduct(delta, vectors[0]);	// forward
	move[1] = -DotProduct(delta, vectors[1]);	// left
	move[2] = DotProduct(delta, vectors[2]);	// up

	VectorAdd(move, oldframe->translate, move);

	for (i = 0; i < 3; i++) {
		move[i] = backlerp * move[i] + frontlerp * frame->translate[i];
	}

	for (i = 0; i < 3; i++) {
		frontv[i] = frontlerp * frame->scale[i];
		backv[i] = backlerp * oldframe->scale[i];
	}
	
	
	lerp = s_lerped[0];

	GL_LerpVerts( paliashdr->num_xyz, v, ov, verts, lerp, move, frontv, backv, 0);
			
		qglEnableClientState	(GL_VERTEX_ARRAY);
		qglVertexPointer		(3, GL_FLOAT, 0, Md2VertArray);
		
		while (1) {
			// get the vertex count and primitive type
			count = *order++;
						
			if (!count)
				break;		// done
			if (count < 0)
				count = -count;
		
			do {
				index_xyz = order[2];
				VA_SetElem2(Md2TexArray[aliasArray], ((float *)order)[0], ((float *)order)[1]); 
				VA_SetElem3(Md2VertArray[aliasArray], s_lerped[index_xyz][0], s_lerped[index_xyz][1], s_lerped[index_xyz][2]); 
				aliasArray++;
				
				order += 3;
			
			} while (--count);
		
			
		}


		if(gl_state.DrawRangeElements && r_DrawRangeElements->value)
			qglDrawRangeElementsEXT(GL_TRIANGLES, 0, aliasArray, currentmodel->numIndices, GL_UNSIGNED_INT, currentmodel->indexArray);		
		else
			qglDrawElements(GL_TRIANGLES, currentmodel->numIndices, GL_UNSIGNED_INT, currentmodel->indexArray);

	

		qglDisableClientState	(GL_VERTEX_ARRAY);
		
		
}



void GL_DrawTexturedShell(dmdl_t *paliashdr, float backlerp)
{
	daliasframe_t	*frame, *oldframe;
	dtrivertx_t	*v, *ov, *verts;
	int		*order;
	int		count;
	float	frontlerp;
	vec3_t	move, delta, vectors[3];
	vec3_t	frontv, backv;
	int		i;
	int		index_xyz;
	float	*lerp;
	float	alpha;
	float	scroll;
	int		shellArray = 0;

	frame = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames
		+ currententity->frame * paliashdr->framesize);
	verts = v = frame->verts;

	oldframe = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames
		+ currententity->oldframe * paliashdr->framesize);
	ov = oldframe->verts;

	order = (int *)((byte *)paliashdr + paliashdr->ofs_glcmds);

	
	if (currententity->flags & RF_TRANSLUCENT)
		alpha = currententity->alpha;
	else
		alpha = 1.0;

	frontlerp = 1.0 - backlerp;
	
	c_alias_polys += paliashdr->num_tris;

	// move should be the delta back to the previous frame * backlerp
	VectorSubtract (currententity->oldorigin, currententity->origin, delta);
	AngleVectors (currententity->angles, vectors[0], vectors[1], vectors[2]);


	move[0] = DotProduct (delta, vectors[0]);	// forward
	move[1] = -DotProduct (delta, vectors[1]);	// left
	move[2] = DotProduct (delta, vectors[2]);	// up

	VectorAdd (move, oldframe->translate, move);

	for (i = 0; i < 3; i++) {
		move[i] = backlerp*move[i] + frontlerp*frame->translate[i];
	}

	for (i = 0; i < 3; i++) {
		frontv[i] = frontlerp*frame->scale[i];
		backv[i] = backlerp*oldframe->scale[i];
	}

	lerp = s_lerped[0];
	
	
	if (currententity->flags & RF_WEAPONMODEL)
		GL_LerpVerts(paliashdr->num_xyz, v, ov, verts, lerp, move, frontv, backv, /*WEAPON_SHELL_SCALE*/0.1f);
	else if (currententity->flags & RF_CAMERAMODEL2)
		GL_LerpVerts(paliashdr->num_xyz, v, ov, verts, lerp, move, frontv, backv, 0); //he-he-he shell scale dont scale shell, but move shell
	else
		GL_LerpVerts(paliashdr->num_xyz, v, ov, verts, lerp, move, frontv, backv, SHELL_SCALE);
	
	qglEnable(GL_BLEND);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE);
			
	if (currententity->flags & RF_SHELL_BLUE)
		GL_Bind(r_texshell[0]->texnum);
	if (currententity->flags & RF_SHELL_RED)
		GL_Bind(r_texshell[1]->texnum);
	if (currententity->flags & RF_SHELL_GREEN)
		GL_Bind(r_texshell[2]->texnum);
	if (currententity->flags & RF_SHELL_GOD)
		GL_Bind(r_texshell[3]->texnum);
	if (currententity->flags & RF_SHELL_HALF_DAM)
		GL_Bind(r_texshell[4]->texnum);
	if (currententity->flags & RF_SHELL_DOUBLE)
		GL_Bind(r_texshell[5]->texnum);

	qglShadeModel(GL_SMOOTH);

		qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
		qglTexCoordPointer (2, GL_FLOAT, 0, Md2TexArray);
		qglEnableClientState(GL_VERTEX_ARRAY);
		qglVertexPointer(3, GL_FLOAT, 0, Md2VertArray);

	while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (!count)
			break;		// done
		if (count < 0)
			count = -count;
	
		do
		{
			index_xyz = order[2];
			order += 3;
			scroll = r_newrefdef.time *0.65;
			
			VA_SetElem2(Md2TexArray[shellArray], (	(s_lerped[index_xyz][1] + s_lerped[index_xyz][0]) * 0.1)  - (scroll),
													(s_lerped[index_xyz][2] * 0.1) + (scroll));
			VA_SetElem3(Md2VertArray[shellArray],	s_lerped[index_xyz][0], s_lerped[index_xyz][1], s_lerped[index_xyz][2]); 
			shellArray++;
			
		} while (--count);
		
	}

		if(gl_state.DrawRangeElements && r_DrawRangeElements->value )
			qglDrawRangeElementsEXT(GL_TRIANGLES, 0, shellArray, currentmodel->numIndices, GL_UNSIGNED_INT, currentmodel->indexArray);		
		else
			qglDrawElements(GL_TRIANGLES, currentmodel->numIndices, GL_UNSIGNED_INT, currentmodel->indexArray);

			
	qglDisableClientState(GL_VERTEX_ARRAY);
	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
		
	qglDisable(GL_BLEND);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
}


/*
** R_CullAliasModel
*/
qboolean R_CullAliasModel(vec3_t bbox[8], entity_t *e)
{
	int i;
	vec3_t		mins, maxs;
	dmdl_t		*paliashdr;
	vec3_t		vectors[3];
	vec3_t		thismins, oldmins, thismaxs, oldmaxs;
	daliasframe_t *pframe, *poldframe;


	vec3_t tmp;

	paliashdr = (dmdl_t *)currentmodel->extradata;

	if ((e->frame >= paliashdr->num_frames) || (e->frame < 0)) {
		Com_Printf("R_CullAliasModel %s: no such frame %d\n",
			currentmodel->name, e->frame);
		e->frame = 0;
	}
	if ((e->oldframe >= paliashdr->num_frames) || (e->oldframe < 0)) {
		Com_Printf("R_CullAliasModel %s: no such oldframe %d\n",
			currentmodel->name, e->oldframe);
		e->oldframe = 0;
	}

	pframe = (daliasframe_t *) ( ( byte * ) paliashdr +
		                              paliashdr->ofs_frames +
									  e->frame * paliashdr->framesize);

	poldframe = (daliasframe_t *) ( ( byte * ) paliashdr +
		                              paliashdr->ofs_frames +
									  e->oldframe * paliashdr->framesize);

	if (pframe == poldframe) {
		for (i = 0; i < 3; i++) {
			mins[i] = pframe->translate[i];
			maxs[i] = mins[i] + pframe->scale[i]*255;
		}
	} else {
		for ( i = 0; i < 3; i++ ) {
			thismins[i] = pframe->translate[i];
			thismaxs[i] = thismins[i] + pframe->scale[i]*255;

			oldmins[i]  = poldframe->translate[i];
			oldmaxs[i]  = oldmins[i] + poldframe->scale[i]*255;

			if ( thismins[i] < oldmins[i] )
				mins[i] = thismins[i];
			else
				mins[i] = oldmins[i];

			if ( thismaxs[i] > oldmaxs[i] )
				maxs[i] = thismaxs[i];
			else
				maxs[i] = oldmaxs[i];
		}
	}

	//=================

	// Compute and rotate bonding box
	AngleVectors(e->angles, vectors[0], vectors[1], vectors[2]);
	VectorSubtract(vec3_origin, vectors[1], vectors[1]); // AngleVectors returns "right" instead of "left"

	for (i = 0; i < 8; i++) {
		tmp[0] = ((i & 1) ? mins[0] : maxs[0]);
		tmp[1] = ((i & 2) ? mins[1] : maxs[1]);
		tmp[2] = ((i & 4) ? mins[2] : maxs[2]);

		bbox[i][0] = vectors[0][0] * tmp[0] + vectors[1][0] * tmp[1] + vectors[2][0] * tmp[2] + e->origin[0];
		bbox[i][1] = vectors[0][1] * tmp[0] + vectors[1][1] * tmp[1] + vectors[2][1] * tmp[2] + e->origin[1];
		bbox[i][2] = vectors[0][2] * tmp[0] + vectors[1][2] * tmp[1] + vectors[2][2] * tmp[2] + e->origin[2];
	}

	//=========================

	{
		int p, f, aggregatemask = ~0;

		for (p = 0; p < 8; p++) {
			int mask = 0;

			for (f = 0; f < 4; f++) {
				float dp = DotProduct(frustum[f].normal, bbox[p]);

				if ((dp - frustum[f].dist) < 0) {
					mask |= (1 << f);
				}
			}

			aggregatemask &= mask;
		}

		if (aggregatemask) {
			return true;
		}

		return false;
	}
}

void SetModelsLight (qboolean ppl)
{
	int i;
	
	if (currententity->flags & (RF_FULLBRIGHT | RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE 
								| RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_GOD))
	{
		for (i = 0; i < 3; i++)
			shadelight[i] = 1.0;
	} 
	else
		R_LightPoint (currententity->origin, shadelight, ppl);

	if (currententity->flags & RF_MINLIGHT) {
		for (i = 0; i < 3; i++)
			if (shadelight[i] > 0.1)
				break;
		if (i == 3) {
			shadelight[0] = 0.1;
			shadelight[1] = 0.1;
			shadelight[2] = 0.1;
		}
	}

	// player lighting hack for communication back to server
	// big hack!
	if (currententity->flags & RF_WEAPONMODEL) {
		// pick the greatest component, which should be the same
		// as the mono value returned by software
		if (shadelight[0] > shadelight[1]) {
			if (shadelight[0] > shadelight[2])
				r_lightLevel->value = 150 * shadelight[0];
			else
				r_lightLevel->value = 150 * shadelight[2];
		} else {
			if (shadelight[1] > shadelight[2])
				r_lightLevel->value = 150 * shadelight[1];
			else
				r_lightLevel->value = 150 * shadelight[2];
		}

	}


	// =================
	// PGM	ir goggles color override
	if ( r_newrefdef.rdflags & RDF_IRGOGGLES) {
		shadelight[0] = 1.0;
		shadelight[1] = 1.0;
		shadelight[2] = 1.0;
	}
	// PGM
	// =================

	shadedots = r_avertexnormal_dots[((int)(currententity->angles[1] * (SHADEDOT_QUANT / 360.0))) & (SHADEDOT_QUANT - 1)];
}


void GL_OldPerspective (GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
   GLdouble		xmin, xmax, ymin, ymax;

   ymax = zNear * tan ((fovy * M_PI) / 360.0);
   ymin = -ymax;

   xmin = ymin * aspect;
   xmax = ymax * aspect;

   xmin += -(2 * gl_state.camera_separation) / zNear;
   xmax += -(2 * gl_state.camera_separation) / zNear;

   qglFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
}

/*
==========================
GL_SetupLightMatrix
==========================
*/


void GL_DrawAliasFrameLerpAmbient (dmdl_t *paliashdr, vec3_t color);

/*
=================
R_DrawAliasModel
=================
*/


int  radarOldTime = 0;
void R_DrawAliasModel (entity_t *e, qboolean weapon_model)
{
	dmdl_t		*paliashdr;
	float		diffuseLight[3];
	vec3_t		bbox[8];
	image_t		*normalmap;
	
	if ( r_newrefdef.rdflags & RDF_IRGOGGLES) 
		goto next;

	if (e->flags & RF_DISTORT)
			return;
next:

	if (!(e->flags & RF_WEAPONMODEL)) {
		if (R_CullAliasModel(bbox, e))
			return;
	}

	if (e->flags & RF_WEAPONMODEL) {
		if (!weapon_model)
			return;
	}

	if (e->flags & RF_WEAPONMODEL) {
		if (r_leftHand->value == 2)
			return;
	}
	
	paliashdr = (dmdl_t *)currentmodel->extradata;

	if (currententity->flags & RF_DEPTHHACK) // hack the depth range to prevent view model from poking into walls
		qglDepthRange(gldepthmin, gldepthmin + 0.3 * (gldepthmax - gldepthmin));

	if ((currententity->flags & RF_WEAPONMODEL ) && ( r_leftHand->value == 1.0F)) {
		extern void GL_OldPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);
		qglMatrixMode(GL_PROJECTION);
		qglPushMatrix();
		qglLoadIdentity();
		qglScalef(-1, 1, 1);
		GL_OldPerspective(r_newrefdef.fov_y, (float) r_newrefdef.width / r_newrefdef.height, 4, 4096);
		qglMatrixMode(GL_MODELVIEW);
		qglCullFace(GL_BACK);
	}
	

    qglPushMatrix ();
	e->angles[PITCH] = -e->angles[PITCH];	// sigh.
	R_RotateForEntity (e);
	e->angles[PITCH] = -e->angles[PITCH];	// sigh.

	// select skin
	if (currententity->bump)
		normalmap = currententity->bump;	// custom player skin
	else {
		if (currententity->skinnum >= MAX_MD2SKINS) {
			normalmap = currentmodel->skins_normal[0];
			currententity->skinnum = 0;
		} else {
			normalmap	= currentmodel->skins_normal[currententity->skinnum];
			if (!normalmap) {
				normalmap = currentmodel->skins_normal[0];
				currententity->skinnum = 0;
			}
		}
	}
	
	if(!r_bumpAlias->value)
		SetModelsLight(false);
	else
		SetModelsLight(true);

	GL_Overbrights (false);

	
	qglShadeModel(GL_SMOOTH);

	if (currententity->flags & RF_TRANSLUCENT) {
		GLSTATE_ENABLE_BLEND
	}


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
		
	if(r_bumpAlias->value){
	VectorCopy(shadelight, diffuseLight);
	VectorScale(shadelight, r_ambientLevel->value, shadelight);
	}
	
	if ( currententity->flags & ( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_GOD)) 
	{
		GL_DrawTexturedShell(paliashdr,currententity->backlerp);
	} 
	else 
	GL_DrawAliasFrameLerpAmbient(paliashdr, shadelight);
		
		if(r_bumpAlias->value){
			VectorCopy(diffuseLight, shadelight);
	}

	GL_TexEnv(GL_REPLACE);

	qglPopMatrix();

	if ((currententity->flags & RF_WEAPONMODEL) && (r_leftHand->value == 1.0F)) {
		qglMatrixMode(GL_PROJECTION);
		qglPopMatrix();
		qglMatrixMode(GL_MODELVIEW);
		qglCullFace(GL_FRONT);
	}

	if (currententity->flags & RF_TRANSLUCENT) {
		GLSTATE_DISABLE_BLEND
	}

	if (currententity->flags & RF_DEPTHHACK)
		qglDepthRange(gldepthmin, gldepthmax);

	if (r_radar->value >1 && (!deathmatch->value)) {
		GL_Overbrights(false);
		
			RadarEnts[numRadarEnts].color[0]= 1.0;
			RadarEnts[numRadarEnts].color[1]= 1.0;
			RadarEnts[numRadarEnts].color[2]= 1.0;

		if (numRadarEnts>=MAX_RADAR_ENTS)
			return;
		
		if (currententity->flags & RF_WEAPONMODEL)
			return;
		if (currententity->flags & RF_NOSHADOW)
			return;

		if (currententity->flags & RF_GLOW) {
			RadarEnts[numRadarEnts].color[0]= 0.0;
			RadarEnts[numRadarEnts].color[1]= 1.0;
			RadarEnts[numRadarEnts].color[2]= 0.0;
		}	else
		if ( currententity->flags & RF_MONSTER) {
			RadarEnts[numRadarEnts].color[0]= 1.0;
			RadarEnts[numRadarEnts].color[1]= 0.0;
			RadarEnts[numRadarEnts].color[2]= 1.0;
		} 
		else {
			RadarEnts[numRadarEnts].color[0]= 0.0;
			RadarEnts[numRadarEnts].color[1]= 1.0;
			RadarEnts[numRadarEnts].color[2]= 1.0;
		}
		VectorCopy(currententity->origin,RadarEnts[numRadarEnts].org);
		
//		==========================================
//		add sound tracker @ALIENS RADAR STYLE@
//		==========================================
		
		if (r_radar->value > 2) {
			if(!VectorCompare (currententity->origin, currententity->oldorigin)) {
				if (r_newrefdef.time > radarOldTime) {
					S_StartLocalSound (fastsound_descriptor[id_radar_sound]);
					radarOldTime = r_newrefdef.time+1;
				}
			}
		}
		numRadarEnts++;
	}
	qglColor4f(1, 1, 1, 1);
	GL_Overbrights (false);
}




void R_DrawAliasModelLightPass (qboolean weapon_model)
{
	dmdl_t		*paliashdr;
	vec3_t		bbox[8];
		
	
	if (currententity->flags & RF_DISTORT)
			return;

	if (!(currententity->flags & RF_WEAPONMODEL)) {
		if (R_CullAliasModel(bbox, currententity))
			return;
	}

	if (currententity->flags & RF_WEAPONMODEL) {
		if (!weapon_model)
			return;
	}

	if (currententity->flags & RF_WEAPONMODEL) {
		if (r_leftHand->value == 2)
			return;
	}

	paliashdr = (dmdl_t *)currentmodel->extradata;
	
	c_alias_polys += paliashdr->num_tris;

	
	if (currententity->flags & RF_DEPTHHACK) // hack the depth range to prevent view model from poking into walls
		qglDepthRange(gldepthmin, gldepthmin + 0.3 * (gldepthmax - gldepthmin));

	if ((currententity->flags & RF_WEAPONMODEL ) && ( r_leftHand->value == 1.0F)) {
		extern void GL_OldPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);
		qglMatrixMode(GL_PROJECTION);
		qglPushMatrix();
		qglLoadIdentity();
		qglScalef(-1, 1, 1);
		GL_OldPerspective(r_newrefdef.fov_y, (float) r_newrefdef.width / r_newrefdef.height, 4, 4096);
		qglMatrixMode(GL_MODELVIEW);
		qglCullFace(GL_BACK);
	}
	
	currententity->angles[PITCH] = -currententity->angles[PITCH];	// sinc with ambient pass.

	
	qglShadeModel(GL_SMOOTH);

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

	

	qglPushMatrix ();
	R_RotateForEntity (currententity);
	
	GL_DrawAliasFrameLerpArbBump(paliashdr);
		
	GL_TexEnv(GL_REPLACE);
	qglPopMatrix();

	currententity->angles[PITCH] = -currententity->angles[PITCH];	// restore player weapon angles.


	if ((currententity->flags & RF_WEAPONMODEL) && (r_leftHand->value == 1.0F)) {
		qglMatrixMode(GL_PROJECTION);
		qglPopMatrix();
		qglMatrixMode(GL_MODELVIEW);
		qglCullFace(GL_FRONT);
	}

	if (currententity->flags & RF_DEPTHHACK)
		qglDepthRange(gldepthmin, gldepthmax);
		
	

	qglColor4f(1, 1, 1, 1);
	GL_Overbrights (false);
}
extern image_t *r_distort2;

void R_DrawAliasDistortModel (entity_t *e)
{
	dmdl_t		*paliashdr;
	vec3_t		bbox[8];
	int			id;
	unsigned	defBits = 0;

		if ( R_CullAliasModel( bbox, e ) )
			return;
	
	
		paliashdr = (dmdl_t *)currentmodel->extradata;

		GL_Overbrights (false);
	
		//
		// draw all the triangles
		//
		if (currententity->flags & RF_DEPTHHACK) // hack the depth range to prevent view model from poking into walls
			qglDepthRange (gldepthmin, gldepthmin + 0.3*(gldepthmax-gldepthmin));


		qglPushMatrix ();
		e->angles[PITCH] = -e->angles[PITCH];	// sigh.
		R_RotateForEntity (e);
		e->angles[PITCH] = -e->angles[PITCH];	// sigh.

		// setup program
		GL_BindProgram(refractProgram, defBits);
		id = refractProgram->id[defBits];

		GL_SelectTexture			(GL_TEXTURE0_ARB);
		qglEnableClientState		(GL_TEXTURE_COORD_ARRAY);
		qglEnableClientState		(GL_COLOR_ARRAY);
		qglTexCoordPointer			(2, GL_FLOAT, 0, Md2TexArray);
		GL_Bind						(r_predator->texnum);
		qglUniform1i				(qglGetUniformLocation(id, "u_deformMap"), 0);
				
		GL_SelectTexture			(GL_TEXTURE1_ARB);
		GL_BindRect					(ScreenMap->texnum);
		qglEnable					(GL_TEXTURE_RECTANGLE_ARB);
		qglUniform1i				(qglGetUniformLocation(id, "g_colorBufferMap"), 1);
	
		GL_SelectTexture			(GL_TEXTURE2_ARB);
		GL_BindRect					(depthMap->texnum);
		qglEnable					(GL_TEXTURE_RECTANGLE_ARB);
		qglUniform1i				(qglGetUniformLocation(id, "g_depthBufferMap"), 2);
	
	
		qglUniform1f				(qglGetUniformLocation(id, "u_deformMul"),	2.0);
		qglUniform1f				(qglGetUniformLocation(id, "u_alpha"),	0.0);
		qglUniform1f				(qglGetUniformLocation(id, "u_thickness"),	1.000);
		qglUniform2f				(qglGetUniformLocation(id, "u_viewport"),	vid.width, vid.height);
		qglUniform2f				(qglGetUniformLocation(id, "u_depthParms"), r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);
	

		if ( (currententity->frame >= paliashdr->num_frames)
			|| (currententity->frame < 0) ) {
			Con_Printf (PRINT_ALL, "R_DrawAliasDistortModel %s: no such frame %d\n",
				currentmodel->name, currententity->frame);
			currententity->frame = 0;
			currententity->oldframe = 0;
		}

		if ( (currententity->oldframe >= paliashdr->num_frames)
			|| (currententity->oldframe < 0)) {
			Con_Printf (PRINT_ALL, "R_DrawAliasDistortModel %s: no such oldframe %d\n",
				currentmodel->name, currententity->oldframe);
			currententity->frame = 0;
			currententity->oldframe = 0;
		}

			GL_DrawAliasFrameLerpDistort (paliashdr, currententity->backlerp);

		GL_SelectTexture		(GL_TEXTURE2_ARB);
		qglDisableClientState	(GL_TEXTURE_COORD_ARRAY);
		qglDisable				(GL_TEXTURE_RECTANGLE_ARB);
	
		GL_SelectTexture		(GL_TEXTURE1_ARB);
		qglDisableClientState	(GL_TEXTURE_COORD_ARRAY);
		qglDisable				(GL_TEXTURE_RECTANGLE_ARB);
	
		GL_SelectTexture		(GL_TEXTURE0_ARB);
		qglDisableClientState	(GL_TEXTURE_COORD_ARRAY);
	
		GL_BindNullProgram();

		GL_TexEnv(GL_REPLACE);

		qglPopMatrix();

		if (currententity->flags & RF_DEPTHHACK)
			qglDepthRange (gldepthmin, gldepthmax);

		GL_Overbrights (false);

	

}