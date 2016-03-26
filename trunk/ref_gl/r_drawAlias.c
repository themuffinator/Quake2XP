/*
Copyright (C) 1997-2001 Id Software, Inc. 2004-2013 Quake2xp Team, Berserker.

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
#include "../client/client.h"
#include "../client/sound.h"

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

void	GL_DrawAliasFrameLerp(dmdl_t *paliashdr, vec3_t color);
void	GL_DrawAliasFrameLerpShell(dmdl_t *paliashdr);

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

	paliashdr = (dmdl_t *)currentmodel->extraData;

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

			for (f = 0; f < 6; f++) {
				float dp = DotProduct(frustum[f].normal, bbox[p]);

				if ((dp - frustum[f].dist) < 0) {
					mask |= (1 << f);
				}
			}

			aggregatemask &= mask;
		}

		if (aggregatemask) {
			return qtrue;
		}

		return qfalse;
	}
}

void SetModelsLight ()
{
	int i;
	float mid;

	if (currententity->flags & (RF_FULLBRIGHT | RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE 
								| RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_GOD))
	{
		for (i = 0; i < 3; i++)
			shadelight[i] = 1.0;
	} 
	else
		R_LightPoint (currententity->origin, shadelight);

	if (currententity->flags & RF_MINLIGHT) {
		for (i = 0; i < 3; i++)
			if (shadelight[i] > 0.01)
				break;
		if (i == 3) {
			shadelight[0] = 0.01;
			shadelight[1] = 0.01;
			shadelight[2] = 0.01;
		}
	}

	// player lighting hack for communication back to server
	// big hack!
	if (currententity->flags & RF_WEAPONMODEL) {
		mid = max(max(shadelight[0], shadelight[1]), shadelight[2]);

		if (mid <= 0.1)
			mid = 0.15;

		mid *= 2.0;
		r_lightLevel->value = 150 * mid;
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

/*
=================
R_DrawAliasModel
=================
*/

void GL_DrawAliasFrameLerpWeapon(dmdl_t *paliashdr);

void R_DrawAliasModel (entity_t *e, qboolean weapon_model)
{
	dmdl_t		*paliashdr;
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
		if (r_leftHand->value == 2)
			return;
	}
	
	paliashdr = (dmdl_t *)currentmodel->extraData;

	if (currententity->flags & RF_DEPTHHACK) // hack the depth range to prevent view model from poking into walls
		GL_DepthRange(gldepthmin, gldepthmin + 0.3 * (gldepthmax - gldepthmin));
		
	SetModelsLight();
	
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

	R_SetupEntityMatrix(e);

	if ( currententity->flags & ( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_GOD)) 
		GL_DrawAliasFrameLerpShell(paliashdr);
	else 
		GL_DrawAliasFrameLerp(paliashdr, shadelight);

	if(weapon_model)	
		GL_DrawAliasFrameLerpWeapon(paliashdr);

	if (currententity->flags & RF_DEPTHHACK)
		GL_DepthRange(gldepthmin, gldepthmax);

}


void R_DrawAliasModelLightPass (qboolean weapon_model)
{
	dmdl_t	*paliashdr;
	vec3_t	bbox[8];
	vec3_t	mins, maxs;
	vec3_t	oldLight, oldView, tmp;
	int		i;

	if (!r_drawEntities->value)
		return;

	if (currententity->flags & RF_DISTORT)
		return;
	
	if (currententity->flags & RF_TRANSLUCENT)
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

	if(r_newrefdef.rdflags & RDF_NOWORLDMODEL){
		if(!currentShadowLight->isNoWorldModel)
			return;
		goto visible;
	}

	if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2]) {
		for (i = 0; i < 3; i++) {
			mins[i] = currententity->origin[i] - currentmodel->radius;
			maxs[i] = currententity->origin[i] + currentmodel->radius;
		}
	}
	else
	{
	VectorAdd(currententity->origin, currententity->model->maxs, maxs);
	VectorAdd(currententity->origin, currententity->model->mins, mins);
	}
	
	if (currentShadowLight->_cone) {

		if (R_CullConeLight(mins, maxs, currentShadowLight->frust))
			return;

	}
	else if (currentShadowLight->spherical) {

		if (!BoundsAndSphereIntersect(mins, maxs, currentShadowLight->origin, currentShadowLight->radius[0]))
			return;
	}
	else {

		if (!BoundsIntersect(mins, maxs, currentShadowLight->mins, currentShadowLight->maxs))
			return;
	}

	if (!InLightVISEntity())
		return;

visible:

	paliashdr = (dmdl_t *)currentmodel->extraData;
	
	c_alias_polys += paliashdr->num_tris;

	
	if (currententity->flags & RF_DEPTHHACK) // hack the depth range to prevent view model from poking into walls
		GL_DepthRange(gldepthmin, gldepthmin + 0.3 * (gldepthmax - gldepthmin));

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
	
	VectorCopy(currentShadowLight->origin, oldLight);
	VectorCopy(r_origin, oldView);

	VectorSubtract(currentShadowLight->origin, currententity->origin, tmp);
	Mat3_TransposeMultiplyVector(currententity->axis, tmp, currentShadowLight->origin);

	VectorSubtract(r_origin, currententity->origin, tmp);
	Mat3_TransposeMultiplyVector(currententity->axis, tmp, r_origin);

	R_SetupEntityMatrix(currententity);

	GL_StencilFunc(GL_EQUAL, 128, 255);
	GL_StencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	GL_StencilMask(0);
	GL_DepthFunc(GL_LEQUAL);

	GL_DrawAliasFrameLerpLight(paliashdr);

	VectorCopy(oldLight, currentShadowLight->origin);
	VectorCopy(oldView, r_origin);

	if (currententity->flags & RF_DEPTHHACK)
		GL_DepthRange(gldepthmin, gldepthmax);
}