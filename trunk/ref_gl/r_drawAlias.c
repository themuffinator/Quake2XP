/*
Copyright (C) 1997-2001 Id Software, Inc. 2004-20011 Quake2xp Team, Berserker.

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
// r_drawAlias.c: draw alias models
// regular, shells, bumpmapped and refracted

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
#define	SHADEDOT_QUANT 16
float	r_avertexnormal_dots[SHADEDOT_QUANT][256]= 
#include "anormtab.h"
;
float shadelight[3];
float	*shadedots = r_avertexnormal_dots[0];
float	ref_realtime =0;

void	GL_DrawAliasFrameLerpAmbient (dmdl_t *paliashdr, vec3_t color);
void	GL_DrawAliasFrameLerpAmbientShell(dmdl_t *paliashdr);
void	GL_DrawAliasFrameLerpAmbientDistort(dmdl_t *paliashdr, vec4_t color);

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


	if(!r_bumpAlias->value)
		SetModelsLight(false);
	else
		SetModelsLight(true);

	GL_Overbrights (false);


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
		GL_DrawAliasFrameLerpAmbientShell(paliashdr);
	else 
		GL_DrawAliasFrameLerpAmbient(paliashdr, shadelight);
		
	if(r_bumpAlias->value)
		VectorCopy(diffuseLight, shadelight);

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


void R_DrawAliasDistortModel (entity_t *e)
{
	dmdl_t		*paliashdr;
	vec3_t		bbox[8];

	if ( R_CullAliasModel( bbox, e ) )
		return;
	
	
		paliashdr = (dmdl_t *)currentmodel->extradata;

		GL_Overbrights (false);
		SetModelsLight(false);

		//
		// draw all the triangles
		//
		if (currententity->flags & RF_DEPTHHACK) // hack the depth range to prevent view model from poking into walls
			qglDepthRange (gldepthmin, gldepthmin + 0.3*(gldepthmax-gldepthmin));


		qglPushMatrix ();
		e->angles[PITCH] = -e->angles[PITCH];	// sigh.
		R_RotateForEntity (e);
		e->angles[PITCH] = -e->angles[PITCH];	// sigh.

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

		GL_DrawAliasFrameLerpAmbientDistort(paliashdr, shadelight);
		
		qglPopMatrix();

		if (currententity->flags & RF_DEPTHHACK)
			qglDepthRange (gldepthmin, gldepthmax);

		GL_Overbrights (false);

	

}