/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
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

void R_CalcAliasFrameLerp (md2Header *paliashdr, float shellScale) {
	md2Frame_t	*frame, *oldFrame;
	md2Vertex_t		*v, *ov, *verts;
	float			frontlerp, backlerp, *lerp;
	vec3_t			move, vectors[3];
	vec3_t			frontv, backv;
	int				i;
	bool		noLerp = false;

	frame		= (md2Frame_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->frame * paliashdr->framesize);
	verts		= v = frame->verts;
	oldFrame	= (md2Frame_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->oldFrame * paliashdr->framesize);
	ov			= oldFrame->verts;
	backlerp	= currententity->backLerp;
	frontlerp	= 1.0 - backlerp;

	// move should be the delta back to the previous frame * backlerp
	VectorSubtract (currententity->oldOrigin, currententity->origin, move);

	if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2]) {
		vec3_t	temp;
		VectorCopy (move, temp);
		AngleVectors (currententity->angles, vectors[0], vectors[1], vectors[2]);
		move[0] = DotProduct (temp, vectors[0]);
		move[1] = -DotProduct (temp, vectors[1]);
		move[2] = DotProduct (temp, vectors[2]);
	}

	VectorAdd (move, oldFrame->translate, move);

	for (i = 0; i < 3; i++) {
		move[i]		= backlerp	* move[i] + frontlerp * frame->translate[i];
		frontv[i]	= frontlerp * frame->scale[i];
		backv[i]	= backlerp	* oldFrame->scale[i];
	}

	lerp = s_lerped[0];
	
	if (frame == oldFrame)
		noLerp = true;

	if (currententity->flags & (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_GOD)) {
		for (i = 0; i < paliashdr->num_xyz; i++, v++, ov++, lerp += 3) {
			float *normal = q_byteDirs[verts[i].lightnormalindex];
			if(noLerp){
				lerp[0] = move[0] + v->v[0] * frame->scale[0] + normal[0] * shellScale;
				lerp[1] = move[1] + v->v[1] * frame->scale[1] + normal[1] * shellScale;
				lerp[2] = move[2] + v->v[2] * frame->scale[2] + normal[2] * shellScale;
			}
			else {
				lerp[0] = move[0] + ov->v[0] * backv[0] + v->v[0] * frontv[0] + normal[0] * shellScale;
				lerp[1] = move[1] + ov->v[1] * backv[1] + v->v[1] * frontv[1] + normal[1] * shellScale;
				lerp[2] = move[2] + ov->v[2] * backv[2] + v->v[2] * frontv[2] + normal[2] * shellScale;
			}
		}
	}
	else {
		for (i = 0; i < paliashdr->num_xyz; i++, v++, ov++, lerp += 3) {
			if(noLerp){
				lerp[0] = move[0] + v->v[0] * frame->scale[0];
				lerp[1] = move[1] + v->v[1] * frame->scale[1];
				lerp[2] = move[2] + v->v[2] * frame->scale[2];
			}
			else {
				lerp[0] = move[0] + ov->v[0] * backv[0] + v->v[0] * frontv[0];
				lerp[1] = move[1] + ov->v[1] * backv[1] + v->v[1] * frontv[1];
				lerp[2] = move[2] + ov->v[2] * backv[2] + v->v[2] * frontv[2];
			}
		}
	}
}

int CL_PMpointcontents (vec3_t point);

void GL_DrawAliasFrameLerp (md2Header *paliashdr, vec3_t lightColor) {
	int				index_xyz, *order,	count, numVerts = 0;
	image_t			*albedo, *normalMap, *emissive;
	float			alphaShift, s, os, shade, backlerp, frontlerp;
	bool			noLerp = false;
	md2Triangle_t		*tris;
	md2Frame_t	*frame,		*oldFrame;
	md2Vertex_t		*verts,		*oldVerts;
	vec3_t			*normals,	*oldNormals;
	vec3_t			*tangents,	*oldTangents;
	vec3_t			*binormals,	*oldBinormals;
	uint			offs;

	alphaShift = sin (ref_realtime * 5.666);
	alphaShift = (alphaShift + 3.0) * 0.5f;
	alphaShift = clamp (alphaShift, 0.01, 6.0);

	float *shadedots = r_avertexnormal_dots[((int)(currententity->angles[1] *
		(SHADEDOT_QUANT / 360.0))) & (SHADEDOT_QUANT - 1)];

	if (currententity->flags & RF_NOCULL) {
		GL_Disable(GL_CULL_FACE);
	}

	if (currententity->flags & (RF_WEAPONMODEL))
		GL_DepthMask(1);

	if (currententity->flags & (RF_VIEWERMODEL))
		return;

	if (r_skipStaticLights->integer) {

		if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
			VectorSet(lightColor, 0.5, 0.5, 0.5);
	}
	else {
		if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
			VectorSet(lightColor, 0.1, 0.1, 0.1);
	}

	if (r_newrefdef.rdflags & RDF_IRGOGGLES)
		VectorSet(lightColor, 1, 1, 1);
	
	if (currententity->flags & (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_GOD | RF_FULLBRIGHT))
		VectorSet(lightColor, 1, 1, 1);
	
		// select skin
	if (currententity->skin)
		albedo = currententity->skin;	// custom player skin
	else {
		if (currententity->skinnum >= MAX_MD2SKINS) {
			albedo = currentmodel->albedo[0];
			currententity->skinnum = 0;
		}
		else {
			albedo = currentmodel->albedo[currententity->skinnum];
			if (!albedo) {
				albedo = currentmodel->albedo[0];
				currententity->skinnum = 0;
			}
		}
	}

	if (!albedo)
		albedo = i_missingTexture;

	// select skin
	if (currententity->bump)
		normalMap = currententity->bump;	// custom player skin
	else {
		if (currententity->skinnum >= MAX_MD2SKINS) {
			normalMap = currentmodel->normalmap[0];
			currententity->skinnum = 0;
		}
		else {
			normalMap = currentmodel->normalmap[currententity->skinnum];
			if (!albedo) {
				normalMap = currentmodel->normalmap[0];
				currententity->skinnum = 0;
			}
		}
	}
	if (!normalMap)
		normalMap = i_defBump;

	emissive = currentmodel->emissive[currententity->skinnum];

	if (!emissive)
		emissive = i_blackTexture1x1;

	if (!albedo)
		albedo = i_missingTexture;

	R_CalcAliasFrameLerp (paliashdr, 0);

	c_aliasTris += paliashdr->num_tris;
	tris = (md2Triangle_t *)((byte *)paliashdr + paliashdr->ofs_tris);

	oldFrame		= (md2Frame_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->oldFrame * paliashdr->framesize);
	oldVerts		= oldFrame->verts;
	offs			= paliashdr->num_xyz * currententity->oldFrame;
	oldNormals		= currentmodel->normals + offs;
	oldTangents		= currentmodel->tangents + offs;
	oldBinormals	= currentmodel->binormals + offs;

	frame		= (md2Frame_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->frame * paliashdr->framesize);
	verts		= frame->verts;
	offs		= paliashdr->num_xyz * currententity->frame;
	normals		= currentmodel->normals + offs;
	tangents	= currentmodel->tangents + offs;
	binormals	= currentmodel->binormals + offs;

	backlerp	= currententity->backLerp;
	frontlerp	= 1 - backlerp;

	if (frame == oldFrame)
		noLerp = true;

	order = (int *)((byte *)paliashdr + paliashdr->ofs_glcmds);

	while (count = *order++) {
		
		if (!count)
			break;	// done

		if (count < 0)
			count = -count;

		do {
			index_xyz = order[2];

			VectorCopy(s_lerped[index_xyz], tess3d.v[numVerts].pos);

			tess3d.v[numVerts].tc[0] = ((float *)order)[0];
			tess3d.v[numVerts].tc[1] = ((float *)order)[1];

			s		= shadedots[verts[index_xyz].lightnormalindex];
			os		= shadedots[oldVerts[index_xyz].lightnormalindex];
			shade	= os * backlerp + s * frontlerp;

			tess3d.v[numVerts].color[0] = shade * lightColor[0];
			tess3d.v[numVerts].color[1] = shade * lightColor[1];
			tess3d.v[numVerts].color[2] = shade * lightColor[2];
			tess3d.v[numVerts].color[3] = 1.0;

			if (r_debugTbn->integer && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL)) {
				
				if (noLerp) {
					tess3d.v[numVerts].tangent[0] = tangents[index_xyz][0];
					tess3d.v[numVerts].tangent[1] = tangents[index_xyz][1];
					tess3d.v[numVerts].tangent[2] = tangents[index_xyz][2];

					tess3d.v[numVerts].binormal[0] = binormals[index_xyz][0];
					tess3d.v[numVerts].binormal[1] = binormals[index_xyz][1];
					tess3d.v[numVerts].binormal[2] = binormals[index_xyz][2];
				}
				else {
					tess3d.v[numVerts].tangent[0] = oldTangents[index_xyz][0] * backlerp + tangents[index_xyz][0] * frontlerp;
					tess3d.v[numVerts].tangent[1] = oldTangents[index_xyz][1] * backlerp + tangents[index_xyz][1] * frontlerp;
					tess3d.v[numVerts].tangent[2] = oldTangents[index_xyz][2] * backlerp + tangents[index_xyz][2] * frontlerp;

					tess3d.v[numVerts].binormal[0] = oldBinormals[index_xyz][0] * backlerp + binormals[index_xyz][0] * frontlerp;
					tess3d.v[numVerts].binormal[1] = oldBinormals[index_xyz][1] * backlerp + binormals[index_xyz][1] * frontlerp;
					tess3d.v[numVerts].binormal[2] = oldBinormals[index_xyz][2] * backlerp + binormals[index_xyz][2] * frontlerp;
				}
			}
			if (currentmodel->envMap || r_debugTbn->integer) {

				if(noLerp){
					tess3d.v[numVerts].normal[0] = normals[index_xyz][0];
					tess3d.v[numVerts].normal[1] = normals[index_xyz][1];
					tess3d.v[numVerts].normal[2] = normals[index_xyz][2];
				}
				else {
					tess3d.v[numVerts].normal[0] = oldNormals[index_xyz][0] * backlerp + normals[index_xyz][0] * frontlerp;
					tess3d.v[numVerts].normal[1] = oldNormals[index_xyz][1] * backlerp + normals[index_xyz][1] * frontlerp;
					tess3d.v[numVerts].normal[2] = oldNormals[index_xyz][2] * backlerp + normals[index_xyz][2] * frontlerp;
				}
			}

			numVerts++;
			order += 3;

		} while (--count);
	}

	// setup program
	GL_BindProgram (aliasAmbientProgram);

	if (currentmodel->envMap)
		qglUniform1i (U_ENV_PASS, 1);
	else
		qglUniform1i (U_ENV_PASS, 0);

	qglUniform1i (U_SHELL_PASS, 0);
	qglUniform1f (U_COLOR_OFFSET, alphaShift);
	
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		qglUniform1i(U_PARAM_INT_0, 1);
	else
		qglUniform1i(U_PARAM_INT_0, 0);

	GL_SetBindlessTexture(U_TMU0, albedo->handle);
	GL_SetBindlessTexture(U_TMU1, emissive->handle);
	GL_SetBindlessTexture(U_TMU2, i_environment->handle);
	GL_SetBindlessTexture(U_TMU3, normalMap->handle);

	qglUniform1f(U_ENV_SCALE, currentmodel->envScale);

	if (r_ssao->integer && !(currententity->flags & RF_WEAPONMODEL) && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL) && !(r_newrefdef.rdflags & RDF_IRGOGGLES)) {
		GL_SetBindlessTexture(U_TMU4, i_ssaoColor[i_ssaoColorIndex]->handle);
		qglUniform1i(U_USE_SSAO, 1);
	}
	else
		qglUniform1i(U_USE_SSAO, 0);

	qglUniform3fv(U_VIEW_POS, 1, r_origin);
	qglUniformMatrix4fv(U_MVP_MATRIX, 1, false, (const float *)currententity->orMatrix);

	GL_BindVAO(vao.stream3d);
	GL_BindVBO(vbo.stream3d);	
	GL_BindVBO(vbo.dynamicIbo);

	qglInvalidateBufferData(GL_ARRAY_BUFFER);
	qglInvalidateBufferData(GL_ELEMENT_ARRAY_BUFFER);
	qglBufferSubData(GL_ARRAY_BUFFER, 0, numVerts * sizeof(vertex3d_t), &tess3d);
	qglBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, currentmodel->numIndices * sizeof(uint16_t), currentmodel->indexArray);

	GL_DrawElements(GL_TRIANGLES, currentmodel->numIndices, GL_UNSIGNED_SHORT, NULL);

	if (r_debugTbn->integer && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL)) {

		GL_BindProgram(tbnDebugProgram);
		
		qglUniformMatrix4fv(U_MVP_MATRIX, 1, false, (const float*)currententity->orMatrix);
		
		if (currententity->flags & (RF_WEAPONMODEL))
			qglUniform1f(U_PARAM_FLOAT_0, 0.3);
		else
			qglUniform1f(U_PARAM_FLOAT_0, r_debugTbnLen->value);

		GL_DrawElements(GL_TRIANGLES, currentmodel->numIndices, GL_UNSIGNED_SHORT, NULL);
	}

	if (r_showTris->integer && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL)) {

		GL_Disable(GL_DEPTH_TEST);
		qglLineWidth(1.5);
		qglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		GL_BindProgram(showTrisProgram);
		qglUniform3f(U_COLOR, 0.0, 1.0, 1.0);
		qglUniformMatrix4fv(U_MVP_MATRIX, 1, false, (const float *)currententity->orMatrix);

		GL_DrawElements(GL_TRIANGLES, currentmodel->numIndices, GL_UNSIGNED_SHORT, NULL);

		qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		GL_Enable(GL_DEPTH_TEST);
	}

	if (currententity->flags & RF_NOCULL) {
		GL_Enable(GL_CULL_FACE);
	}

	if (currententity->flags & (RF_WEAPONMODEL))
		GL_DepthMask(0);
}

void GL_DrawAliasFrameLerpShell (md2Header *paliashdr) {
	int				index_xyz, *order, count, numVerts = 0;
	md2Triangle_t		*tris;
	float			backlerp, frontlerp;
	bool		noLerp = false;
	md2Frame_t	*frame, *oldFrame;
	md2Vertex_t		*verts, *oldVerts;
	vec3_t			*normals, *oldNormals;
	uint			offs;

	if (currententity->flags & (RF_VIEWERMODEL))
		return;

	if (currententity->flags & RF_WEAPONMODEL)
		R_CalcAliasFrameLerp (paliashdr, 0.1);
	else if (currententity->flags & RF_CAMERAMODEL2)
		R_CalcAliasFrameLerp (paliashdr, 0.0);
	else
		R_CalcAliasFrameLerp (paliashdr, 0.5);

	c_aliasTris += paliashdr->num_tris;

	tris		= (md2Triangle_t *)((byte *)paliashdr + paliashdr->ofs_tris);
	oldFrame	= (md2Frame_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->oldFrame * paliashdr->framesize);
	oldVerts	= oldFrame->verts;
	offs		= paliashdr->num_xyz * currententity->oldFrame;
	oldNormals	= currentmodel->normals + offs;

	frame	= (md2Frame_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->frame * paliashdr->framesize);
	verts	= frame->verts;
	offs	= paliashdr->num_xyz * currententity->frame;
	normals = currentmodel->normals + offs;

	backlerp	= currententity->backLerp;
	frontlerp	= 1 - backlerp;
	
	if (frame == oldFrame)
		noLerp = true;

	// setup program
	GL_BindProgram (aliasAmbientProgram);
	
	vec2_t shellParams = { r_newrefdef.time * 0.45, 0.0f };

	qglUniform1i (U_SHELL_PASS, 1);
	qglUniform1i (U_ENV_PASS, 1);
	qglUniform2fv(U_SHELL_PARAMS, 1, shellParams);
	qglUniform3fv(U_VIEW_POS, 1, r_origin);

	qglUniformMatrix4fv(U_MVP_MATRIX, 1, false, (const float *)currententity->orMatrix);

	if (currententity->flags & RF_SHELL_BLUE)
		GL_SetBindlessTexture(U_TMU0, r_texshell[0]->handle);
	if (currententity->flags & RF_SHELL_RED)
		GL_SetBindlessTexture(U_TMU0, r_texshell[1]->handle);
	if (currententity->flags & RF_SHELL_GREEN)
		GL_SetBindlessTexture(U_TMU0, r_texshell[2]->handle);
	if (currententity->flags & RF_SHELL_GOD)
		GL_SetBindlessTexture(U_TMU0, r_texshell[3]->handle);
	if (currententity->flags & RF_SHELL_HALF_DAM)
		GL_SetBindlessTexture(U_TMU0, r_texshell[4]->handle);
	if (currententity->flags & RF_SHELL_DOUBLE)
		GL_SetBindlessTexture(U_TMU0, r_texshell[5]->handle);	

	order = (int *)((byte *)paliashdr + paliashdr->ofs_glcmds);

	while (count = *order++) {

		if (!count)
			break;	// done

		if (count < 0)
			count = -count;

		do {
			index_xyz = order[2];

			VectorCopy(s_lerped[index_xyz], tess3d.v[numVerts].pos);

			if (noLerp) {
				tess3d.v[numVerts].normal[0] = normals[index_xyz][0];
				tess3d.v[numVerts].normal[1] = normals[index_xyz][1];
				tess3d.v[numVerts].normal[2] = normals[index_xyz][2];
			}
			else {
				tess3d.v[numVerts].normal[0] = oldNormals[index_xyz][0] * backlerp + normals[index_xyz][0] * frontlerp;
				tess3d.v[numVerts].normal[1] = oldNormals[index_xyz][1] * backlerp + normals[index_xyz][1] * frontlerp;
				tess3d.v[numVerts].normal[2] = oldNormals[index_xyz][2] * backlerp + normals[index_xyz][2] * frontlerp;
			}
			numVerts++;
			order += 3;

		} while (--count);
	}

	GL_BindVAO(vao.stream3d);
	GL_BindVBO(vbo.stream3d);
	GL_BindVBO(vbo.dynamicIbo);

	qglInvalidateBufferData(GL_ARRAY_BUFFER);
	qglInvalidateBufferData(GL_ELEMENT_ARRAY_BUFFER);
	qglBufferSubData(GL_ARRAY_BUFFER, 0, numVerts * sizeof(vertex3d_t), &tess3d);
	qglBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, currentmodel->numIndices * sizeof(uint16_t), currentmodel->indexArray);
	
	GL_DrawElements(GL_TRIANGLES, currentmodel->numIndices, GL_UNSIGNED_SHORT, NULL);
}

void GL_DrawAliasFrameLerpLight (md2Header *paliashdr) {
	int				index_xyz,	*order, count, numVerts = 0;
	vec3_t			*binormals, *oldBinormals;
	vec3_t			*tangents,	*oldTangents;
	vec3_t			*normals,	*oldNormals;
	md2Triangle_t		*tris;
	md2Frame_t	*frame, *oldFrame;
	md2Vertex_t		*verts, *oldVerts;
	float			backlerp, frontlerp;
	uint			offs;
	vec3_t			maxs;
	image_t			*albedo, *normalMap, *pbr;
	bool		inWater, noLerp = false;

	if (currententity->flags & (RF_VIEWERMODEL))
		return;

	if (currentmodel->noSelfShadow && r_shadows->integer)
		GL_Disable(GL_STENCIL_TEST);

	tris			= (md2Triangle_t *)((byte *)paliashdr + paliashdr->ofs_tris);
	oldFrame		= (md2Frame_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->oldFrame * paliashdr->framesize);
	oldVerts		= oldFrame->verts;
	offs			= paliashdr->num_xyz * currententity->oldFrame;
	oldBinormals	= currentmodel->binormals + offs;
	oldTangents		= currentmodel->tangents + offs;
	oldNormals		= currentmodel->normals + offs;

	frame			= (md2Frame_t *)((byte *)paliashdr + paliashdr->ofs_frames + currententity->frame * paliashdr->framesize);
	verts			= frame->verts;
	offs			= paliashdr->num_xyz * currententity->frame;
	binormals		= currentmodel->binormals + offs;
	tangents		= currentmodel->tangents + offs;
	normals			= currentmodel->normals + offs;
	
	backlerp	= currententity->backLerp;
	frontlerp	= 1 - backlerp;
	
	if (frame = oldFrame)
		noLerp = true;

	// select skin
	if (currententity->skin)
		albedo = currententity->skin;	// custom player skin
	else {
		if (currententity->skinnum >= MAX_MD2SKINS) {
			albedo = currentmodel->albedo[0];
			currententity->skinnum = 0;
		}
		else {
			albedo = currentmodel->albedo[currententity->skinnum];
			if (!albedo) {
				albedo = currentmodel->albedo[0];
				currententity->skinnum = 0;
			}
		}
	}
	if (!albedo)
		albedo = i_missingTexture;

	// select skin
	if (currententity->bump)
		normalMap = currententity->bump;	// custom player skin
	else {
		if (currententity->skinnum >= MAX_MD2SKINS) {
			normalMap = currentmodel->normalmap[0];
			currententity->skinnum = 0;
		}
		else {
			normalMap = currentmodel->normalmap[currententity->skinnum];
			if (!albedo) {
				normalMap = currentmodel->normalmap[0];
				currententity->skinnum = 0;
			}
		}
	}
	if (!normalMap)
		normalMap = i_defBump;
	
	pbr = currentmodel->pbr[currententity->skinnum];
	if (!pbr)
		pbr = i_blackTexture1x1;

	R_CalcAliasFrameLerp(paliashdr, 0);			/// Просто сюда переместили вычисления Lerp...
	
	c_litAliasTris += paliashdr->num_tris;
	order = (int *)((byte *)paliashdr + paliashdr->ofs_glcmds);

	while (count = *order++) {

		if (!count)
			break;	// done

		if (count < 0)
			count = -count;

		do {
			index_xyz = order[2];

			VectorCopy(s_lerped[index_xyz], tess3d.v[numVerts].pos);

			tess3d.v[numVerts].tc[0] = ((float *)order)[0];
			tess3d.v[numVerts].tc[1] = ((float *)order)[1];

			if (noLerp) {
				tess3d.v[numVerts].tangent[0]	= tangents[index_xyz][0];
				tess3d.v[numVerts].tangent[1]	= tangents[index_xyz][1];
				tess3d.v[numVerts].tangent[2]	= tangents[index_xyz][2];

				tess3d.v[numVerts].binormal[0]	= binormals[index_xyz][0];
				tess3d.v[numVerts].binormal[1]	= binormals[index_xyz][1];
				tess3d.v[numVerts].binormal[2]	= binormals[index_xyz][2];

				tess3d.v[numVerts].normal[0]	= normals[index_xyz][0];
				tess3d.v[numVerts].normal[1]	= normals[index_xyz][1];
				tess3d.v[numVerts].normal[2]	= normals[index_xyz][2];
			}
			else {
				tess3d.v[numVerts].tangent[0]	= oldTangents[index_xyz][0] * backlerp + tangents[index_xyz][0] * frontlerp;
				tess3d.v[numVerts].tangent[1]	= oldTangents[index_xyz][1] * backlerp + tangents[index_xyz][1] * frontlerp;
				tess3d.v[numVerts].tangent[2]	= oldTangents[index_xyz][2] * backlerp + tangents[index_xyz][2] * frontlerp;

				tess3d.v[numVerts].binormal[0]	= oldBinormals[index_xyz][0] * backlerp + binormals[index_xyz][0] * frontlerp;
				tess3d.v[numVerts].binormal[1]	= oldBinormals[index_xyz][1] * backlerp + binormals[index_xyz][1] * frontlerp;
				tess3d.v[numVerts].binormal[2]	= oldBinormals[index_xyz][2] * backlerp + binormals[index_xyz][2] * frontlerp;

				tess3d.v[numVerts].normal[0]	= oldNormals[index_xyz][0] * backlerp + normals[index_xyz][0] * frontlerp;
				tess3d.v[numVerts].normal[1]	= oldNormals[index_xyz][1] * backlerp + normals[index_xyz][1] * frontlerp;
				tess3d.v[numVerts].normal[2]	= oldNormals[index_xyz][2] * backlerp + normals[index_xyz][2] * frontlerp;
			}
			numVerts++;
			order += 3;
		} while (--count);
	}

	// setup program
	GL_BindProgram (aliasBumpProgram);

	VectorAdd (currententity->origin, currententity->model->maxs, maxs);
	if (CL_PMpointcontents (maxs) & MASK_WATER)
		inWater = true;
	else
		inWater = false;

	R_UpdateLightAliasUniforms();
	
	if (r_imageAutoBump->integer && normalMap == i_defBump) {
		qglUniform1i(U_USE_AUTOBUMP, 1);
		qglUniform2f(U_AUTOBUMP_PARAMS, r_imageAutoBumpScale->value, r_imageAutoSpecularScale->value);
	}
	else
		qglUniform1i(U_USE_AUTOBUMP, 0);

	if ((inWater && currentShadowLight->castCaustics && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL)) || (!inWater && currentShadowLight->castCaustics2 && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL)))
		qglUniform1i(U_USE_CAUSTICS, 1);
	else
		qglUniform1i(U_USE_CAUSTICS, 0);

	if (r_ssao->integer && !(r_newrefdef.rdflags & RDF_IRGOGGLES) && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL) && !(currententity->flags & RF_WEAPONMODEL))
		qglUniform1i(U_USE_SSAO, 1);
	else
		qglUniform1i(U_USE_SSAO, 0);
	
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		qglUniform1i(U_PARAM_INT_5, 1);
	else
		qglUniform1i(U_PARAM_INT_5, 0);

	GL_SetBindlessTexture(U_TMU0, normalMap->handle);
	GL_SetBindlessTexture(U_TMU1, albedo->handle);
	GL_SetBindlessTexture(U_TMU2, r_caustic[((int)(r_newrefdef.time * 15)) & (MAX_CAUSTICS - 1)]->handle);
	GL_SetBindlessTexture(U_TMU3, r_lightCubeMap[currentShadowLight->filter]->handle);
	GL_SetBindlessTexture(U_TMU4, pbr->handle);
	GL_SetBindlessTexture(U_TMU5, i_skinBump->handle);
	GL_SetBindlessTexture(U_TMU8, i_ssaoColor[i_ssaoColorIndex]->handle);

	if (pbr == i_blackTexture1x1)
		qglUniform1i(U_USE_RGH_MAP, 0);
	else 
		qglUniform1i(U_USE_RGH_MAP, 1);
	
	qglUniform1i(U_PARAM_INT_1, 0);
	qglUniform1i(U_PARAM_INT_2, 0);
	qglUniform1i(U_PARAM_INT_4, 0);

	GL_BindVAO(vao.stream3d);
	GL_BindVBO(vbo.stream3d);
	GL_BindVBO(vbo.dynamicIbo);

	qglInvalidateBufferData(GL_ARRAY_BUFFER);
	qglInvalidateBufferData(GL_ELEMENT_ARRAY_BUFFER);
	qglBufferSubData(GL_ARRAY_BUFFER, 0, numVerts * sizeof(vertex3d_t), &tess3d);
	qglBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, currentmodel->numIndices * sizeof(uint16_t), currentmodel->indexArray);

	GL_DrawElements(GL_TRIANGLES, currentmodel->numIndices, GL_UNSIGNED_SHORT, NULL);
}


/*
=============================================================
ALIAS MODELS
=============================================================
*/

float	shadelight[3];
float	ref_realtime = 0;

void	GL_DrawAliasFrameLerp(md2Header *paliashdr, vec3_t color);
void	GL_DrawAliasFrameLerpShell(md2Header *paliashdr);

/*
** R_CullAliasModel
*/
bool R_CullAliasModel(vec3_t bbox[8], entity_t *e)
{
	int i;
	vec3_t		mins, maxs;
	md2Header		*paliashdr;
	vec3_t		vectors[3];
	vec3_t		thismins, oldmins, thismaxs, oldmaxs;
	md2Frame_t *pframe, *poldframe;
	vec3_t tmp;

	paliashdr = (md2Header *)currentmodel->extraData;

	if ((e->frame >= paliashdr->num_frames) || (e->frame < 0)) {
		Com_Printf("R_CullAliasModel %s: no such frame %d\n",
			currentmodel->name, e->frame);
		e->frame = 0;
	}
	if ((e->oldFrame >= paliashdr->num_frames) || (e->oldFrame < 0)) {
		Com_Printf("R_CullAliasModel %s: no such oldframe %d\n", currentmodel->name, e->oldFrame);
		e->oldFrame = 0;
	}

	pframe = (md2Frame_t *)((byte *)paliashdr + paliashdr->ofs_frames + e->frame * paliashdr->framesize);

	poldframe = (md2Frame_t *)((byte *)paliashdr + paliashdr->ofs_frames + e->oldFrame * paliashdr->framesize);

	if (pframe == poldframe) {
		for (i = 0; i < 3; i++) {
			mins[i] = pframe->translate[i];
			maxs[i] = mins[i] + pframe->scale[i] * 255;
		}
	}
	else {
		for (i = 0; i < 3; i++) {
			thismins[i] = pframe->translate[i];
			thismaxs[i] = thismins[i] + pframe->scale[i] * 255;

			oldmins[i] = poldframe->translate[i];
			oldmaxs[i] = oldmins[i] + poldframe->scale[i] * 255;

			if (thismins[i] < oldmins[i])
				mins[i] = thismins[i];
			else
				mins[i] = oldmins[i];

			if (thismaxs[i] > oldmaxs[i])
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
			return true;
		}

		return false;
	}
}

void SetModelsLight()
{
	int i;
	float mid;

	if (currententity->flags & (RF_FULLBRIGHT | RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE
		| RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_GOD)) {

		for (i = 0; i < 3; i++)
			shadelight[i] = 1.0;
	}
	else
		R_LightPoint(currententity->origin, shadelight);

	if (currententity->flags & RF_MINLIGHT) {
		
		for (i = 0; i < 3; i++)
			if (shadelight[i] > 0.0019)
				break;

		if (i == 3) {
			shadelight[0] = 0.0019;
			shadelight[1] = 0.0019;
			shadelight[2] = 0.0019;
		}
	}

	// player lighting hack for communication back to server
	// big hack!
	if (currententity->flags & RF_WEAPONMODEL) {
		mid = max(max(shadelight[0], shadelight[1]), shadelight[2]);

		if (mid <= 0.01)
			mid = 0.015;

		mid *= 2.0;
		r_lightLevel->value = 150 * mid;
	}

	// =================
	// PGM	ir goggles color override
	if (r_newrefdef.rdflags & RDF_IRGOGGLES) {
		shadelight[0] = 1.0;
		shadelight[1] = 1.0;
		shadelight[2] = 1.0;
	}
	// PGM
	// =================
}

/*
=================
R_DrawAliasModel
=================
*/

void R_DrawAliasModel(entity_t *e)
{
	md2Header		*paliashdr;
	vec3_t		bbox[8];

	if (!(e->flags & RF_WEAPONMODEL)) {
		if (R_CullAliasModel(bbox, e))
			return;
	}

	if (e->flags & RF_WEAPONMODEL) {
		if (r_leftHand->integer == 2)
			return;
	}

	paliashdr = (md2Header *)currentmodel->extraData;

	if (currententity->flags & RF_DEPTHHACK) // hack the depth range to prevent view model from poking into walls
		GL_DepthRange(gldepthmin, gldepthmin + 0.3 * (gldepthmax - gldepthmin));

	SetModelsLight();

	if ((currententity->frame >= paliashdr->num_frames) || (currententity->frame < 0)) {

		Com_Printf("R_DrawAliasModel %s: no such frame %d\n", currentmodel->name, currententity->frame);
		currententity->frame = 0;
		currententity->oldFrame = 0;
	}

	if ((currententity->oldFrame >= paliashdr->num_frames) || (currententity->oldFrame < 0)) {
		Com_Printf("R_DrawAliasModel %s: no such oldframe %d\n", currentmodel->name, currententity->oldFrame);
		currententity->frame = 0;
		currententity->oldFrame = 0;
	}

	R_SetupEntityMatrix(e);

	if (currententity->flags & (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_GOD))
		GL_DrawAliasFrameLerpShell(paliashdr);
	else
		GL_DrawAliasFrameLerp(paliashdr, shadelight);

	if (currententity->flags & RF_DEPTHHACK)
		GL_DepthRange(gldepthmin, gldepthmax);

}


void R_DrawAliasModelLightPass(bool weapon_model)
{
	md2Header	*paliashdr;
	vec3_t	bbox[8];
	vec3_t	oldLight, oldView, tmp;

	if (!r_drawEntities->integer)
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
		if (!weapon_model || r_leftHand->integer == 2)
			return;
	}

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL) {
		if (!currentShadowLight->isNoWorldModel)
			return;
		goto visible;
	}

	if (!R_AliasInLightBound())
		return;

visible:

	paliashdr = (md2Header *)currentmodel->extraData;


	if (currententity->flags & RF_DEPTHHACK) // hack the depth range to prevent view model from poking into walls
		GL_DepthRange(gldepthmin, gldepthmin + 0.3 * (gldepthmax - gldepthmin));

	if ((currententity->frame >= paliashdr->num_frames) || (currententity->frame < 0)) {

		Com_Printf("R_DrawAliasModel %s: no such frame %d\n", currentmodel->name, currententity->frame);
		currententity->frame = 0;
		currententity->oldFrame = 0;
	}

	if ((currententity->oldFrame >= paliashdr->num_frames) || (currententity->oldFrame < 0)) {

		Com_Printf("R_DrawAliasModel %s: no such oldframe %d\n", currentmodel->name, currententity->oldFrame);
		currententity->frame = 0;
		currententity->oldFrame = 0;
	}

	R_SetupEntityMatrix(currententity);

	VectorCopy(currentShadowLight->origin, oldLight);
	VectorCopy(r_origin, oldView);

	VectorSubtract(currentShadowLight->origin, currententity->origin, tmp);
	Mat3_TransposeMultiplyVector(currententity->axis, tmp, currentShadowLight->origin);

	VectorSubtract(r_origin, currententity->origin, tmp);
	Mat3_TransposeMultiplyVector(currententity->axis, tmp, r_origin);

	GL_StencilFunc(GL_EQUAL, 128, 255);
	GL_StencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	GL_StencilMask(0);
	GL_DepthFunc(GL_LEQUAL);
	
	GL_PolygonOffset(-1.0, -1.0);

	GL_DrawAliasFrameLerpLight(paliashdr);

	VectorCopy(oldLight, currentShadowLight->origin);
	VectorCopy(oldView, r_origin);

	if (currententity->flags & RF_DEPTHHACK)
		GL_DepthRange(gldepthmin, gldepthmax);
}