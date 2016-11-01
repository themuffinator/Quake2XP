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
// r_calcAlias.c: calc triangles for alias models


#include "r_local.h"

vec3_t	tempVertexArray[MAX_VERTICES * 4];

extern float	*shadedots;

void R_CalcAliasFrameLerp (dmdl_t *paliashdr, float shellScale) {
	daliasframe_t	*frame, *oldframe;
	dtrivertx_t	*v, *ov, *verts;
	float	frontlerp;
	vec3_t	move, vectors[3];
	vec3_t	frontv, backv;
	int		i;
	float	*lerp;
	float	backlerp;

	backlerp = currententity->backlerp;

	frame = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->frame * paliashdr->framesize);
	verts = v = frame->verts;
	oldframe = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->oldframe * paliashdr->framesize);
	ov = oldframe->verts;

	frontlerp = 1.0 - backlerp;

	// move should be the delta back to the previous frame * backlerp
	VectorSubtract (currententity->oldorigin, currententity->origin, move);

	if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2]) {
		vec3_t	temp;
		VectorCopy (move, temp);
		AngleVectors (currententity->angles, vectors[0], vectors[1], vectors[2]);
		move[0] = DotProduct (temp, vectors[0]);
		move[1] = -DotProduct (temp, vectors[1]);
		move[2] = DotProduct (temp, vectors[2]);
	}

	VectorAdd (move, oldframe->translate, move);

	for (i = 0; i < 3; i++) {
		move[i] = backlerp*move[i] + frontlerp*frame->translate[i];
		frontv[i] = frontlerp*frame->scale[i];
		backv[i] = backlerp*oldframe->scale[i];
	}

	lerp = tempVertexArray[0];

	if (currententity->flags & (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_GOD)) {
		for (i = 0; i < paliashdr->num_xyz; i++, v++, ov++, lerp += 3) {
			float *normal = q_byteDirs[verts[i].lightnormalindex];
			lerp[0] = move[0] + ov->v[0] * backv[0] + v->v[0] * frontv[0] + normal[0] * shellScale;
			lerp[1] = move[1] + ov->v[1] * backv[1] + v->v[1] * frontv[1] + normal[1] * shellScale;
			lerp[2] = move[2] + ov->v[2] * backv[2] + v->v[2] * frontv[2] + normal[2] * shellScale;
		}
	}
	else {
		for (i = 0; i < paliashdr->num_xyz; i++, v++, ov++, lerp += 3) {

			lerp[0] = move[0] + ov->v[0] * backv[0] + v->v[0] * frontv[0];
			lerp[1] = move[1] + ov->v[1] * backv[1] + v->v[1] * frontv[1];
			lerp[2] = move[2] + ov->v[2] * backv[2] + v->v[2] * frontv[2];
		}
	}
}

int CL_PMpointcontents (vec3_t point);

void GL_DrawAliasFrameLerp (dmdl_t *paliashdr, vec3_t lightColor) {
	vec3_t			vertexArray[3 * MAX_TRIANGLES];
	vec4_t			colorArray[4 * MAX_TRIANGLES];
	int				index_xyz;
	int				i, j, jj = 0;
	dtriangle_t		*tris;
	image_t			*skin, *skinNormalmap, *glowskin;
	float			alphaShift, alpha, l;
	vec3_t			normalArray[3 * MAX_TRIANGLES];
	float			backlerp, frontlerp;
	int				index2, oldindex2;
	daliasframe_t	*frame, *oldframe;
	dtrivertx_t		*verts, *oldverts;

	alphaShift = sin (ref_realtime * currentmodel->glowCfg[2]);
	alphaShift = (alphaShift + 1) * 0.5f;
	alphaShift = clamp (alphaShift, currentmodel->glowCfg[0], currentmodel->glowCfg[1]);

	if (currententity->flags & RF_TRANSLUCENT)
		alpha = currententity->alpha;
	else
		alpha = 1.0;

	if (currententity->flags & (RF_VIEWERMODEL))
		return;

	if (r_skipStaticLights->value) {

		if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
			VectorSet(lightColor, 0.75, 0.75, 0.75);
	}
	else {
		if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
			VectorSet(lightColor, 0.33, 0.33, 0.33);
	}

	if(r_newrefdef.rdflags & RDF_IRGOGGLES)
		VectorSet (lightColor, 1, 1, 1);

	// select skin
	if (currententity->skin)
		skin = currententity->skin;	// custom player skin
	else {
		if (currententity->skinnum >= MAX_MD2SKINS) {
			skin = currentmodel->skins[0];
			currententity->skinnum = 0;
		}
		else {
			skin = currentmodel->skins[currententity->skinnum];
			if (!skin) {
				skin = currentmodel->skins[0];
				currententity->skinnum = 0;
			}
		}
	}

	if (!skin)
		skin = r_notexture;

	// select skin
	if (currententity->bump)
		skinNormalmap = currententity->bump;	// custom player skin
	else {
		if (currententity->skinnum >= MAX_MD2SKINS) {
			skinNormalmap = currentmodel->skins_normal[0];
			currententity->skinnum = 0;
		}
		else {
			skinNormalmap = currentmodel->skins_normal[currententity->skinnum];
			if (!skin) {
				skinNormalmap = currentmodel->skins_normal[0];
				currententity->skinnum = 0;
			}
		}
	}
	if (!skinNormalmap)
		skinNormalmap = r_defBump;

	glowskin = currentmodel->glowtexture[currententity->skinnum];

	if (!glowskin)
		glowskin = r_notexture;

	if (!skin)
		skin = r_notexture;

	R_CalcAliasFrameLerp (paliashdr, 0);

	qglEnableVertexAttribArray (ATT_POSITION);
	qglVertexAttribPointer (ATT_POSITION, 3, GL_FLOAT, qfalse, 0, vertexArray);

	qglEnableVertexAttribArray (ATT_NORMAL);
	qglVertexAttribPointer (ATT_NORMAL, 3, GL_FLOAT, qfalse, 0, normalArray);

	qglEnableVertexAttribArray (ATT_COLOR);
	qglVertexAttribPointer (ATT_COLOR, 4, GL_FLOAT, qfalse, 0, colorArray);

	qglBindBufferARB (GL_ARRAY_BUFFER_ARB, currentmodel->vboId);
	qglEnableVertexAttribArray (ATT_TEX0);
	qglVertexAttribPointer (ATT_TEX0, 2, GL_FLOAT, qfalse, 0, 0);


	c_alias_polys += paliashdr->num_tris;
	tris = (dtriangle_t *)((byte *)paliashdr + paliashdr->ofs_tris);
	jj = 0;

	oldframe = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->oldframe * paliashdr->framesize);
	oldverts = oldframe->verts;
	frame = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->frame * paliashdr->framesize);
	verts = frame->verts;
	backlerp = currententity->backlerp;
	frontlerp = 1 - backlerp;

	for (i = 0; i < paliashdr->num_tris; i++) {
		for (j = 0; j < 3; j++, jj++) {
			index_xyz = tris[i].index_xyz[j];
			VectorCopy (tempVertexArray[index_xyz], vertexArray[jj]);

			l = shadedots[verts[index_xyz].lightnormalindex];
			VA_SetElem4 (colorArray[jj], l * lightColor[0], l * lightColor[1], l * lightColor[2], alpha);

			if (currentmodel->envMap) {
				index2 = verts[index_xyz].lightnormalindex;
				oldindex2 = oldverts[index_xyz].lightnormalindex;
				normalArray[jj][0] = q_byteDirs[oldindex2][0] * backlerp + q_byteDirs[index2][0] * frontlerp;
				normalArray[jj][1] = q_byteDirs[oldindex2][1] * backlerp + q_byteDirs[index2][1] * frontlerp;
				normalArray[jj][2] = q_byteDirs[oldindex2][2] * backlerp + q_byteDirs[index2][2] * frontlerp;
			}
		}
	}

	// setup program
	GL_BindProgram (aliasAmbientProgram, 0);

	if (currentmodel->envMap)
		qglUniform1i (ambientAlias_isEnvMaping, 1);
	else
		qglUniform1i (ambientAlias_isEnvMaping, 0);

	qglUniform1i (ambientAlias_isShell, 0);

	qglUniform1f (ambientAlias_colorModulate, r_textureColorScale->value);
	qglUniform1f (ambientAlias_addShift, alphaShift);

	GL_MBind (GL_TEXTURE0_ARB, skin->texnum);
	GL_MBind (GL_TEXTURE1_ARB, glowskin->texnum);
	GL_MBind (GL_TEXTURE2_ARB, r_envTex->texnum);
	GL_MBind (GL_TEXTURE3_ARB, skinNormalmap->texnum);

	qglUniform1f(ambientAlias_envScale, currentmodel->envScale);

	if (r_ssao->value && !(currententity->flags & RF_WEAPONMODEL) && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL) && !(r_newrefdef.rdflags & RDF_IRGOGGLES)) {
		GL_MBindRect (GL_TEXTURE4_ARB, fboColor[fboColorIndex]->texnum);
		qglUniform1i(ambientAlias_ssao, 1);
	}
	else
		qglUniform1i(ambientAlias_ssao, 0);

	qglUniform3fv(ambientAlias_viewOrg, 1, r_origin);
	qglUniformMatrix4fv(ambientAlias_mvp, 1, qfalse, (const float *)currententity->orMatrix);

	qglDrawArrays (GL_TRIANGLES, 0, jj);

	qglDisableVertexAttribArray (ATT_POSITION);
	qglDisableVertexAttribArray (ATT_NORMAL);
	qglDisableVertexAttribArray (ATT_COLOR);
	qglDisableVertexAttribArray (ATT_TEX0);
	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	GL_BindNullProgram ();
}

void GL_DrawAliasFrameLerpShell (dmdl_t *paliashdr) {
	vec3_t		vertexArray[3 * MAX_TRIANGLES];
	int			index_xyz, i, j, jj = 0;
	dtriangle_t	*tris;
	unsigned	defBits = 0;
	float		scroll = 0.0;
	vec3_t		normalArray[3 * MAX_TRIANGLES];
	float		backlerp, frontlerp;
	int			index2, oldindex2;
	daliasframe_t	*frame, *oldframe;
	dtrivertx_t		*verts, *oldverts;

	if (currententity->flags & (RF_VIEWERMODEL))
		return;

	scroll = r_newrefdef.time *0.45;

	if (currententity->flags & RF_WEAPONMODEL)
		R_CalcAliasFrameLerp (paliashdr, 0.1);
	else if (currententity->flags & RF_CAMERAMODEL2)
		R_CalcAliasFrameLerp (paliashdr, 0.0);
	else
		R_CalcAliasFrameLerp (paliashdr, 0.5);

	c_alias_polys += paliashdr->num_tris;

	jj = 0;
	tris = (dtriangle_t *)((byte *)paliashdr + paliashdr->ofs_tris);
	oldframe = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->oldframe * paliashdr->framesize);
	oldverts = oldframe->verts;
	frame = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->frame * paliashdr->framesize);
	verts = frame->verts;
	backlerp = currententity->backlerp;
	frontlerp = 1 - backlerp;

	for (i = 0; i < paliashdr->num_tris; i++) {
		for (j = 0; j < 3; j++, jj++) {
			index_xyz = tris[i].index_xyz[j];
			VectorCopy (tempVertexArray[index_xyz], vertexArray[jj]);

			index2 = verts[index_xyz].lightnormalindex;
			oldindex2 = oldverts[index_xyz].lightnormalindex;

			normalArray[jj][0] = q_byteDirs[oldindex2][0] * backlerp + q_byteDirs[index2][0] * frontlerp;
			normalArray[jj][1] = q_byteDirs[oldindex2][1] * backlerp + q_byteDirs[index2][1] * frontlerp;
			normalArray[jj][2] = q_byteDirs[oldindex2][2] * backlerp + q_byteDirs[index2][2] * frontlerp;

		}
	}

	// setup program
	GL_BindProgram (aliasAmbientProgram, defBits);

	qglUniform1i (ambientAlias_isShell, 1);
	qglUniform1i (ambientAlias_isEnvMaping, 0);
	qglUniform1f (ambientAlias_colorModulate, r_textureColorScale->value);
	qglUniform1f (ambientAlias_scroll, scroll);
	qglUniform3fv(ambientAlias_viewOrg, 1, r_origin);

	qglUniformMatrix4fv(ambientAlias_mvp, 1, qfalse, (const float *)currententity->orMatrix);

	if (currententity->flags & RF_SHELL_BLUE)
		GL_MBind (GL_TEXTURE0_ARB, r_texshell[0]->texnum);
	if (currententity->flags & RF_SHELL_RED)
		GL_MBind (GL_TEXTURE0_ARB, r_texshell[1]->texnum);
	if (currententity->flags & RF_SHELL_GREEN)
		GL_MBind (GL_TEXTURE0_ARB, r_texshell[2]->texnum);
	if (currententity->flags & RF_SHELL_GOD)
		GL_MBind (GL_TEXTURE0_ARB, r_texshell[3]->texnum);
	if (currententity->flags & RF_SHELL_HALF_DAM)
		GL_MBind (GL_TEXTURE0_ARB, r_texshell[4]->texnum);
	if (currententity->flags & RF_SHELL_DOUBLE)
		GL_MBind (GL_TEXTURE0_ARB, r_texshell[5]->texnum);

	qglEnableVertexAttribArray (ATT_POSITION);
	qglVertexAttribPointer (ATT_POSITION, 3, GL_FLOAT, qfalse, 0, vertexArray);

	qglEnableVertexAttribArray (ATT_NORMAL);
	qglVertexAttribPointer (ATT_NORMAL, 3, GL_FLOAT, qfalse, 0, normalArray);

	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, currentmodel->vboId);
	qglEnableVertexAttribArray (ATT_TEX0);
	qglVertexAttribPointer (ATT_TEX0, 2, GL_FLOAT, qfalse, 0, 0);

	qglDrawArrays (GL_TRIANGLES, 0, jj);

	GL_Disable (GL_BLEND);

	qglDisableVertexAttribArray (ATT_POSITION);
	qglDisableVertexAttribArray (ATT_NORMAL);
	qglDisableVertexAttribArray (ATT_TEX0);
	qglBindBufferARB (GL_ARRAY_BUFFER_ARB, 0);
	GL_BindNullProgram ();
}

void R_UpdateLightAliasUniforms()
{
	mat4_t	entAttenMatrix, entSpotMatrix;

	qglUniform1f(lightAlias_colorScale, r_textureColorScale->value);
	qglUniform1i(lightAlias_ambient, (int)currentShadowLight->isAmbient);
	qglUniform4f(lightAlias_lightColor, currentShadowLight->color[0], currentShadowLight->color[1], currentShadowLight->color[2], 1.0);
	qglUniform1i(lightAlias_fog, (int)currentShadowLight->isFog);
	if (currententity->flags & RF_WEAPONMODEL)
		qglUniform1f(lightAlias_fogDensity, currentShadowLight->fogDensity * 8.0);
	else
		qglUniform1f(lightAlias_fogDensity, currentShadowLight->fogDensity);
	qglUniform1f(lightAlias_causticsIntens, r_causticIntens->value);
	qglUniform3fv(lightAlias_viewOrigin, 1, r_origin);
	qglUniform3fv(lightAlias_lightOrigin, 1, currentShadowLight->origin);

	Mat4_TransposeMultiply(currententity->matrix, currentShadowLight->attenMatrix, entAttenMatrix);
	qglUniformMatrix4fv(lightAlias_attenMatrix, 1, qfalse, (const float *)entAttenMatrix);


	Mat4_TransposeMultiply(currententity->matrix, currentShadowLight->spotMatrix, entSpotMatrix);
	qglUniformMatrix4fv(lightAlias_spotMatrix, 1, qfalse, (const float *)entSpotMatrix);
	qglUniform3f(lightAlias_spotParams, currentShadowLight->hotSpot, 1.f / (1.f - currentShadowLight->hotSpot), currentShadowLight->coneExp);

	if (currentShadowLight->isCone)
		qglUniform1i(lightAlias_spotLight, 1);
	else
		qglUniform1i(lightWorld_spotLight, 0);
	
	R_CalcCubeMapMatrix(qtrue);
	qglUniformMatrix4fv(lightAlias_cubeMatrix, 1, qfalse, (const float *)currentShadowLight->cubeMapMatrix);

	qglUniformMatrix4fv(lightAlias_mvp, 1, qfalse, (const float *)currententity->orMatrix);
	qglUniformMatrix4fv(lightAlias_mv, 1, qfalse, (const float *)r_newrefdef.modelViewMatrix);

}

void GL_DrawAliasFrameLerpLight (dmdl_t *paliashdr) {
	int				i, j, jj = 0;
	int				index_xyz;
	byte			*binormals, *oldbinormals;
	byte			*tangents, *oldtangents;
	dtriangle_t		*tris;
	daliasframe_t	*frame, *oldframe;
	dtrivertx_t		*verts, *oldverts;
	float			backlerp, frontlerp;
	unsigned		offs, offs2;
	vec3_t			normalArray[3 * MAX_TRIANGLES],
					tangentArray[3 * MAX_TRIANGLES],
					binormalArray[3 * MAX_TRIANGLES],
					vertexArray[3 * MAX_TRIANGLES],
					maxs;
	image_t			*skin, *skinNormalmap, *rgh;
	int				index2, oldindex2;
	qboolean		inWater;

	if (currententity->flags & (RF_VIEWERMODEL))
		return;

	if (currentmodel->noSelfShadow && r_shadows->value)
		GL_Disable(GL_STENCIL_TEST);

	backlerp = currententity->backlerp;
	frontlerp = 1 - backlerp;

	offs = paliashdr->num_xyz;

	oldframe = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->oldframe * paliashdr->framesize);
	oldverts = oldframe->verts;
	offs2 = offs*currententity->oldframe;
	oldbinormals = currentmodel->binormals + offs2;
	oldtangents = currentmodel->tangents + offs2;

	frame = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->frame * paliashdr->framesize);
	verts = frame->verts;
	offs2 = offs*currententity->frame;
	binormals = currentmodel->binormals + offs2;
	tangents = currentmodel->tangents + offs2;
	tris = (dtriangle_t *)((byte *)paliashdr + paliashdr->ofs_tris);

	// select skin
	if (currententity->skin)
		skin = currententity->skin;	// custom player skin
	else {
		if (currententity->skinnum >= MAX_MD2SKINS) {
			skin = currentmodel->skins[0];
			currententity->skinnum = 0;
		}
		else {
			skin = currentmodel->skins[currententity->skinnum];
			if (!skin) {
				skin = currentmodel->skins[0];
				currententity->skinnum = 0;
			}
		}
	}
	if (!skin)
		skin = r_notexture;

//	// select skin
	if (currententity->bump)
		skinNormalmap = currententity->bump;	// custom player skin
	else {
		if (currententity->skinnum >= MAX_MD2SKINS) {
			skinNormalmap = currentmodel->skins_normal[0];
			currententity->skinnum = 0;
		}
		else {
			skinNormalmap = currentmodel->skins_normal[currententity->skinnum];
			if (!skin) {
				skinNormalmap = currentmodel->skins_normal[0];
				currententity->skinnum = 0;
			}
		}
	}
	if (!skinNormalmap)
		skinNormalmap = r_defBump;
	
	rgh = currentmodel->skins_roughness[currententity->skinnum];
	if (!rgh)
		rgh = r_notexture;

	R_CalcAliasFrameLerp(paliashdr, 0);			/// Просто сюда переместили вычисления Lerp...

	for (i = 0; i < paliashdr->num_tris; i++) {
		for (j = 0; j < 3; j++, jj++) {
			index_xyz = tris[i].index_xyz[j];
			index2 = verts[index_xyz].lightnormalindex;
			oldindex2 = oldverts[index_xyz].lightnormalindex;

			normalArray[jj][0] = q_byteDirs[oldindex2][0] * backlerp + q_byteDirs[index2][0] * frontlerp;
			normalArray[jj][1] = q_byteDirs[oldindex2][1] * backlerp + q_byteDirs[index2][1] * frontlerp;
			normalArray[jj][2] = q_byteDirs[oldindex2][2] * backlerp + q_byteDirs[index2][2] * frontlerp;

			tangentArray[jj][0] = q_byteDirs[oldtangents[index_xyz]][0] * backlerp + q_byteDirs[tangents[index_xyz]][0] * frontlerp;
			tangentArray[jj][1] = q_byteDirs[oldtangents[index_xyz]][1] * backlerp + q_byteDirs[tangents[index_xyz]][1] * frontlerp;
			tangentArray[jj][2] = q_byteDirs[oldtangents[index_xyz]][2] * backlerp + q_byteDirs[tangents[index_xyz]][2] * frontlerp;

			binormalArray[jj][0] = q_byteDirs[oldbinormals[index_xyz]][0] * backlerp + q_byteDirs[binormals[index_xyz]][0] * frontlerp;
			binormalArray[jj][1] = q_byteDirs[oldbinormals[index_xyz]][1] * backlerp + q_byteDirs[binormals[index_xyz]][1] * frontlerp;
			binormalArray[jj][2] = q_byteDirs[oldbinormals[index_xyz]][2] * backlerp + q_byteDirs[binormals[index_xyz]][2] * frontlerp;

			VectorCopy(tempVertexArray[index_xyz], vertexArray[jj]);

		}
	}

	// setup program
	GL_BindProgram (aliasBumpProgram, 0);

	VectorAdd (currententity->origin, currententity->model->maxs, maxs);
	if (CL_PMpointcontents (maxs) & MASK_WATER)
		inWater = qtrue;
	else
		inWater = qfalse;

	R_UpdateLightAliasUniforms();

	if (inWater && currentShadowLight->castCaustics && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
		qglUniform1i(lightAlias_isCaustics, 1);
	else
		qglUniform1i(lightAlias_isCaustics, 0);

	qglUniform1f (lightAlias_specularScale, r_specularScale->value);

	GL_MBind (GL_TEXTURE0_ARB, skinNormalmap->texnum);
	GL_MBind (GL_TEXTURE1_ARB, skin->texnum);
	GL_MBind (GL_TEXTURE2_ARB, r_caustic[((int)(r_newrefdef.time * 15)) & (MAX_CAUSTICS - 1)]->texnum);
	GL_MBindCube (GL_TEXTURE3_ARB, r_lightCubeMap[currentShadowLight->filter]->texnum);

	if (rgh == r_notexture)
		qglUniform1i(lightAlias_isRgh, 0);
	else {
		qglUniform1i(lightAlias_isRgh, 1);
		GL_MBind(GL_TEXTURE4_ARB, rgh->texnum);
	}
	
	GL_MBind(GL_TEXTURE5_ARB, skinBump->texnum);

	qglEnableVertexAttribArray (ATT_POSITION);
	qglVertexAttribPointer (ATT_POSITION, 3, GL_FLOAT, qfalse, 0, vertexArray);

	qglEnableVertexAttribArray (ATT_TANGENT);
	qglVertexAttribPointer (ATT_TANGENT, 3, GL_FLOAT, qfalse, 0, tangentArray);

	qglEnableVertexAttribArray (ATT_BINORMAL);
	qglVertexAttribPointer (ATT_BINORMAL, 3, GL_FLOAT, qfalse, 0, binormalArray);

	qglEnableVertexAttribArray (ATT_NORMAL);
	qglVertexAttribPointer (ATT_NORMAL, 3, GL_FLOAT, qfalse, 0, normalArray);

	qglBindBufferARB (GL_ARRAY_BUFFER_ARB, currentmodel->vboId);
	qglEnableVertexAttribArray (ATT_TEX0);
	qglVertexAttribPointer (ATT_TEX0, 2, GL_FLOAT, qfalse, 0, 0);

	qglDrawArrays (GL_TRIANGLES, 0, jj);

	qglDisableVertexAttribArray (ATT_POSITION);
	qglDisableVertexAttribArray (ATT_TANGENT);
	qglDisableVertexAttribArray (ATT_BINORMAL);
	qglDisableVertexAttribArray (ATT_NORMAL);
	qglDisableVertexAttribArray (ATT_TEX0);
	qglBindBufferARB (GL_ARRAY_BUFFER_ARB, 0);
	GL_BindNullProgram ();

}

//calc the light vectors into the color pointer
void GL_CalculateMD5LightVectors(modelMeshObject_t *obj)
{
	int i = 0;
	float *lightDir = obj->meshData.lightVectors;
	modelVert_t *clr = (modelVert_t *)obj->meshData.colorPointerVec;
	while (i < obj->meshData.numVerts)
	{
		//scale and bias into a color vector
		clr->x = lightDir[0] * 0.5f + 0.5f;
		clr->y = lightDir[1] * 0.5f + 0.5f;
		clr->z = lightDir[2] * 0.5f + 0.5f;

		lightDir += 3;
		clr++;
		i++;
	}
}

extern float	shadelight[3];
//calculate vert lighting
void GL_CalculateMD5ColorPointer(modelMeshObject_t *obj, float arbitraryMultiplier)
{
	int v = 0;
	modelRGBA_t *clr = obj->meshData.colorPointer;
	BYTE *vNorm = obj->meshData.vertNormalIndexes;

	while (v < obj->meshData.numVerts)
	{
		float l = shadedots[*vNorm];

		clr->rgba[0] = (l*shadelight[0])*arbitraryMultiplier;
		clr->rgba[1] = (l*shadelight[1])*arbitraryMultiplier;
		clr->rgba[2] = (l*shadelight[2])*arbitraryMultiplier;
		clr->rgba[3] = 1.0f;

		v++;
		clr++;
		vNorm++;
	}
}

extern void Math_TransformPointByMatrix(modelMatrix_t *matrix, float *in, float *out);
extern void Math_MatrixInverse(modelMatrix_t *in, modelMatrix_t *out);

void GL_GetWorldToLocalMatrix(entity_t *e, modelMatrix_t *invModelMat)
{
	modelMatrix_t modelMat;

	//now i translate the gl 4x4 matrix to one of my matrices
	modelMat.x1[0] = e->orMatrix[0][0];
	modelMat.x1[1] = e->orMatrix[1][0];
	modelMat.x1[2] = e->orMatrix[2][0];

	modelMat.x2[0] = e->orMatrix[0][1];
	modelMat.x2[1] = e->orMatrix[1][1];
	modelMat.x2[2] = e->orMatrix[2][1];

	modelMat.x3[0] = e->orMatrix[0][2];
	modelMat.x3[1] = e->orMatrix[1][2];
	modelMat.x3[2] = e->orMatrix[2][2];

	modelMat.o[0] = e->orMatrix[0][3];
	modelMat.o[1] = e->orMatrix[1][3];
	modelMat.o[2] = e->orMatrix[2][3];

	//invert the matrix to transform the world coordinates with it
	Math_MatrixInverse(&modelMat, invModelMat);
}

extern void Mod_CreateTransformedVerts(entity_t *e, modelMeshObject_t *obj, modelMeshObject_t *root, model_t *anm);
extern void Mod_TangentUpdates(entity_t *e, modelMeshObject_t *obj, float *lightPos);
extern void SetModelsLight();

void R_DrawMD5Model(entity_t *e)
{
	modelMeshObject_t *root;
	modelMeshObject_t *obj;
	model_t *m = e->model;

	if (!m || m->type != mod_md5 || !m->md5)
	{
		Com_Printf("R_DrawMD5Model call with invalid model type.\n");
		return;
	}

	R_SetupEntityMatrix(e);

	GL_DepthMask(1);

	SetModelsLight();

	root = m->md5;

	//run through and render all objects

	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_COLOR);
	qglEnableVertexAttribArray(ATT_TEX0);
	// setup program
	GL_BindProgram(aliasAmbientProgram, 0);

	qglUniform1i(ambientAlias_isEnvMaping, 0);
	qglUniform1i(ambientAlias_isShell, 0);

	qglUniform1f(ambientAlias_colorModulate, r_textureColorScale->value);
	qglUniform1f(ambientAlias_addShift, 1.0);

	qglUniform1f(ambientAlias_envScale, currentmodel->envScale);
	qglUniform1i(ambientAlias_ssao, 0);

	qglUniform3fv(ambientAlias_viewOrg, 1, r_origin);
	qglUniformMatrix4fv(ambientAlias_mvp, 1, qfalse, (const float *)currententity->orMatrix);
	
	obj = root;
	while (obj)
	{
		//this lets us know we need to transform again.
		obj->meshData.renderCountCurrent++;

		//make sure the verts are transformed for the current state.
		Mod_CreateTransformedVerts(e, obj, root, e->anim);

		//set the vertex array for our transformed verts.
		qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, obj->meshData.vertDataTransformed);
		//set the tex coord array
		qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, 0, obj->meshData.uvCoord);
		//set the color pointer
		GL_CalculateMD5ColorPointer(obj, 1.0f);
		qglVertexAttribPointer(ATT_COLOR, 4, GL_FLOAT, qfalse, 0, obj->meshData.colorPointer);

		if (!e->skin)
		{ //no override, so use the obj skin
			if (obj->meshData.skin)
			{
				GL_MBind(GL_TEXTURE0, obj->meshData.skin->texnum);
			}
		}
		else
			{
				GL_MBind(GL_TEXTURE0, e->skin->texnum);
			}

			//now, draw.
			qglDrawElements(GL_TRIANGLES, obj->meshData.numFaces * 3, GL_UNSIGNED_INT, obj->meshData.faceDataFormatted);

		obj = obj->next;
	}

	qglDisableVertexAttribArray(ATT_POSITION);
	qglDisableVertexAttribArray(ATT_COLOR);
	qglDisableVertexAttribArray(ATT_TEX0);
	GL_BindNullProgram();
	GL_DepthMask(0);
}