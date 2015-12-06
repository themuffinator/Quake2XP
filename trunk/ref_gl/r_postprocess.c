/*
Copyright (C) 2006-2011 Quake2xp Team

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



int r_numflares;
flare_t r_flares[MAX_FLARES];
vec3_t vert[1];
int numFlareOcc;
static vec3_t vert_array[MAX_FLARES_VERTEX];
static vec2_t tex_array[MAX_FLARES_VERTEX];
static vec4_t color_array[MAX_FLARES_VERTEX];

/*===============
World lens flares
===============*/

void R_BuildFlares (flare_t * light) {

	float		dist, dist2, scale;
	vec3_t		v, tmp;
	int			flareVert = 0;

	if (!r_skipStaticLights->value)
		return;

	light->surf->visframe = r_framecount;

	// Color Fade
	VectorSubtract (light->origin, r_origin, v);
	dist = VectorLength (v) * (light->size * 0.01);
	dist2 = VectorLength (v);

	scale = ((1024 - dist2) / 1024) * 0.5;

	VectorScale (light->color, scale, tmp);

	VectorMA (light->origin, -1 - dist, vup, vert_array[0]);
	VectorMA (vert_array[0], 1 + dist, vright, vert_array[0]);
	VA_SetElem2 (tex_array[0], 0, 1);
	VA_SetElem4 (color_array[0], tmp[0], tmp[1], tmp[2], 1);

	VectorMA (light->origin, -1 - dist, vup, vert_array[1]);
	VectorMA (vert_array[1], -1 - dist, vright, vert_array[1]);
	VA_SetElem2 (tex_array[1], 0, 0);
	VA_SetElem4 (color_array[1], tmp[0], tmp[1], tmp[2], 1);

	VectorMA (light->origin, 1 + dist, vup, vert_array[2]);
	VectorMA (vert_array[2], -1 - dist, vright, vert_array[2]);
	VA_SetElem2 (tex_array[2], 1, 0);
	VA_SetElem4 (color_array[2], tmp[0], tmp[1], tmp[2], 1);

	VectorMA (light->origin, 1 + dist, vup, vert_array[3]);
	VectorMA (vert_array[3], 1 + dist, vright, vert_array[3]);
	VA_SetElem2 (tex_array[3], 1, 1);
	VA_SetElem4 (color_array[3], tmp[0], tmp[1], tmp[2], 1);

	qglDrawElements	(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

	c_flares++;
}

qboolean PF_inPVS (vec3_t p1, vec3_t p2);

void R_RenderFlares (void) {
	int i, id;
	flare_t *fl;

	if (!r_drawFlares->value)
		return;
	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;

	qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadTris);
	qglEnableVertexAttribArray (ATT_POSITION);
	qglEnableVertexAttribArray (ATT_TEX0);
	qglEnableVertexAttribArray (ATT_COLOR);

	qglVertexAttribPointer (ATT_POSITION, 3, GL_FLOAT, qfalse, 0, vert_array);
	qglVertexAttribPointer (ATT_TEX0, 2, GL_FLOAT, qfalse, 0, tex_array);
	qglVertexAttribPointer (ATT_COLOR, 4, GL_FLOAT, qfalse, 0, color_array);

	GL_DepthMask (0);
	GL_Enable (GL_BLEND);
	GL_BlendFunc (GL_ONE, GL_ONE);

	GL_BindProgram (particlesProgram, 0);
	id = particlesProgram->id[0];

	GL_MBind (GL_TEXTURE0_ARB, r_flare->texnum);
	qglUniform1i (qglGetUniformLocation (id, "u_map0"), 0);

	GL_MBindRect (GL_TEXTURE1_ARB, depthMap->texnum);
	qglUniform1i (qglGetUniformLocation (id, "u_depthBufferMap"), 1);
	qglUniform2f (qglGetUniformLocation (id, "u_depthParms"), r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);
	qglUniform2f (qglGetUniformLocation (id, "u_mask"), 1.0, 0.0);
	qglUniform1f (qglGetUniformLocation (id, "u_colorScale"), 1.0);

	fl = r_flares;
	for (i = 0; i < r_numflares; i++, fl++) {
		int sidebit;
		float viewplane;

		if (fl->ignore)
			continue;

		if (!PF_inPVS (fl->origin, r_origin))
			continue;

		if (fl->surf->visframe != r_framecount)	// pvs culling... haha, nicest optimisation!
			continue;

		viewplane = DotProduct (r_origin, fl->surf->plane->normal) - fl->surf->plane->dist;
		if (viewplane >= 0)
			sidebit = 0;
		else
			sidebit = MSURF_PLANEBACK;

		if ((fl->surf->flags & MSURF_PLANEBACK) != sidebit)
			continue;			// wrong light poly side!

		qglUniform1f (qglGetUniformLocation (id, "u_thickness"), fl->size * 1.5);

		R_BuildFlares (fl);

	}
	GL_BindNullProgram ();
	qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
	qglDisableVertexAttribArray (ATT_POSITION);
	qglDisableVertexAttribArray (ATT_TEX0);
	qglDisableVertexAttribArray (ATT_COLOR);
	GL_Disable (GL_BLEND);
	GL_DepthMask (1);
}

/*
====================
GLSL Full Screen
Post Process Effects
====================
*/

void R_DrawFullScreenQuad () {

	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo.vbo_fullScreenQuad);
	qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadTris);
	
	qglEnableVertexAttribArray (ATT_POSITION);
	qglVertexAttribPointer (ATT_POSITION, 2, GL_FLOAT, qfalse, 0, 0);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

	qglDisableVertexAttribArray (ATT_POSITION);
	
	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void R_DrawHalfScreenQuad () {

	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo.vbo_halfScreenQuad);
	qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadTris);
	
	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 2, GL_FLOAT, qfalse, 0, 0);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

	qglDisableVertexAttribArray(ATT_POSITION);
	
	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void R_DrawQuarterScreenQuad () {
	
	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo.vbo_quarterScreenQuad);
	qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadTris);
	
	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 2, GL_FLOAT, qfalse, 0, 0);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

	qglDisableVertexAttribArray(ATT_POSITION);
	
	qglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	qglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void R_Bloom (void) 
{
	if (!r_bloom->value)
		return;

	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;

	// downsample and cut color
	GL_MBindRect (GL_TEXTURE0_ARB, ScreenMap->texnum);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);

	// setup program
	GL_BindProgram (bloomdsProgram, 0);
	qglUniform1f(bloomDS_threshold, r_bloomThreshold->value);
	qglUniform1i (bloomDS_map, 0);
	qglUniformMatrix4fv(bloomDS_matrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawQuarterScreenQuad ();

	// create bloom texture (set to zero in default state)
	if (!bloomtex) {
		qglGenTextures (1, &bloomtex);
		GL_BindRect (bloomtex);
		qglTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglCopyTexImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, 0, 0, vid.width*0.25, vid.height*0.25, 0);
	}

	// star blur
	GL_BindRect (bloomtex);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

	GL_BindProgram (blurStarProgram, 0);
	qglUniform1i(star_tex, 0);
	qglUniform1f(star_intens, r_bloomStarIntens->value);
	qglUniformMatrix4fv(star_matrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawQuarterScreenQuad ();
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

	// blur x
	GL_BindRect (bloomtex);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

	GL_BindProgram (gaussXProgram, 0);
	qglUniform1i(gaussx_tex, 0);
	qglUniformMatrix4fv(gaussx_matrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawQuarterScreenQuad ();

	// blur y
	GL_BindRect (bloomtex);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

	GL_BindProgram (gaussYProgram, 0);
	qglUniform1i(gaussy_tex, 0);
	qglUniformMatrix4fv(gaussy_matrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawQuarterScreenQuad ();

	// store 2 pass gauss blur 
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

	//final pass
	GL_BindProgram (bloomfpProgram, 0);
	GL_MBindRect (GL_TEXTURE0_ARB, ScreenMap->texnum);
	qglUniform1i(bloomFP_map0, 0);

	GL_MBindRect (GL_TEXTURE1_ARB, bloomtex);
	qglUniform1i(bloomFP_map1, 1);
	qglUniform3f(bloomFP_params, r_bloomIntens->value, r_bloomBright->value, r_bloomExposure->value);
	qglUniformMatrix4fv(bloom_FP_matrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad ();

	GL_BindNullProgram ();
}



void R_ThermalVision (void) 
{
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	if (!(r_newrefdef.rdflags & RDF_IRGOGGLES))
		return;

	if (!thermaltex) {
		qglGenTextures (1, &thermaltex);
		GL_MBindRect(GL_TEXTURE0_ARB, thermaltex);
		qglTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglCopyTexImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, 0, 0, vid.width, vid.height, 0);
	}
	else {
		GL_MBindRect(GL_TEXTURE0_ARB, thermaltex);
		qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
	}

	// setup program
	GL_BindProgram (thermalProgram, 0);
	qglUniform1i (therm_map, 0);
	qglUniformMatrix4fv(therm_matrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawHalfScreenQuad ();

	// blur x
	GL_BindRect (thermaltex);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.5, vid.height*0.5);

	GL_BindProgram (gaussXProgram, 0);
	qglUniform1i (gaussx_tex, 0);
	qglUniformMatrix4fv(gaussx_matrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawHalfScreenQuad ();

	// blur y
	GL_BindRect (thermaltex);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.5, vid.height*0.5);

	GL_BindProgram (gaussYProgram, 0);
	qglUniform1i (gaussy_tex, 0);
	qglUniformMatrix4fv(gaussy_matrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawHalfScreenQuad ();

	// store 2 pass gauss blur 
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.5, vid.height*0.5);

	//final pass
	GL_BindProgram (thermalfpProgram, 0);

	GL_BindRect (thermaltex);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
	qglUniform1i (thermf_map, 0);
	qglUniformMatrix4fv(thermf_matrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad ();

	GL_BindNullProgram ();
}



void R_RadialBlur (void) 
{
	float blur;

	if (!r_radialBlur->value)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	if (r_newrefdef.fov_x <= r_radialBlurFov->value)
		goto hack;

	if (r_newrefdef.rdflags & (RDF_UNDERWATER | RDF_PAIN)) {

	hack:

		GL_MBindRect (GL_TEXTURE0_ARB, ScreenMap->texnum);
		qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);

		// setup program
		GL_BindProgram (radialProgram, 0);
		qglUniform1i(rb_tex, 0);

		if (r_newrefdef.rdflags & RDF_PAIN)
			blur = 0.01;
		else if (r_newrefdef.rdflags & RDF_UNDERWATER)
			blur = 0.0085;
		else
			blur = 0.01;

		// xy = radial center screen space position, z = radius attenuation, w = blur strength
		qglUniform4f(rb_params, vid.width*0.5, vid.height*0.5, 1.0 / vid.height, blur);
		qglUniformMatrix4fv(rb_matrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

		R_DrawFullScreenQuad ();

		GL_BindNullProgram ();
	}
}

void R_DofBlur (void) 
{
	float			tmpDist[5], tmpMins[3];
	vec2_t          dofParams;
	trace_t			trace;
	vec3_t			end_trace, v_f, v_r, v_up, tmp, left, right, up, dn;

	if (!r_dof->value)
		return;
	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;

	//dof autofocus
	if (!r_dofFocus->value) {

		AngleVectors (r_newrefdef.viewangles, v_f, v_r, v_up);
		VectorMA (r_newrefdef.vieworg, 4096, v_f, end_trace);

		VectorMA (end_trace, 96, v_r, right);
		VectorMA (end_trace, -96, v_r, left);
		VectorMA (end_trace, 96, v_up, up);
		VectorMA (end_trace, -96, v_up, dn);

		trace = CL_PMTraceWorld (r_newrefdef.vieworg, vec3_origin, vec3_origin, right, MASK_SHOT, qtrue);
		VectorSubtract (trace.endpos, r_newrefdef.vieworg, tmp);
		tmpDist[0] = VectorLength (tmp);

		trace = CL_PMTraceWorld (r_newrefdef.vieworg, vec3_origin, vec3_origin, left, MASK_SHOT, qtrue);
		VectorSubtract (trace.endpos, r_newrefdef.vieworg, tmp);
		tmpDist[1] = VectorLength (tmp);

		trace = CL_PMTraceWorld (r_newrefdef.vieworg, vec3_origin, vec3_origin, up, MASK_SHOT, qtrue);
		VectorSubtract (trace.endpos, r_newrefdef.vieworg, tmp);
		tmpDist[2] = VectorLength (tmp);

		trace = CL_PMTraceWorld (r_newrefdef.vieworg, vec3_origin, vec3_origin, dn, MASK_SHOT, qtrue);
		VectorSubtract (trace.endpos, r_newrefdef.vieworg, tmp);
		tmpDist[3] = VectorLength (tmp);

		trace = CL_PMTraceWorld (r_newrefdef.vieworg, vec3_origin, vec3_origin, end_trace, MASK_SHOT, qtrue);
		VectorSubtract (trace.endpos, r_newrefdef.vieworg, tmp);
		tmpDist[4] = VectorLength (tmp);

		tmpMins[0] = min (tmpDist[0], tmpDist[1]);
		tmpMins[1] = min (tmpDist[2], tmpDist[3]);
		tmpMins[2] = min (tmpMins[0], tmpMins[1]);

		dofParams[0] = min (tmpMins[2], tmpDist[4]);
		dofParams[1] = r_dofBias->value;
	}
	else {
		dofParams[0] = r_dofFocus->value;
		dofParams[1] = r_dofBias->value;
	}

	// setup program
	GL_BindProgram (dofProgram, 0);
	qglUniform2f(dof_screenSize, vid.width, vid.height);
	qglUniform4f (dof_params, dofParams[0], dofParams[1], r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);
	qglUniformMatrix4fv(dof_matrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);
	qglUniform1i(dof_tex, 0);
	qglUniform1i(dof_depth, 1);

	GL_MBindRect (GL_TEXTURE0_ARB, ScreenMap->texnum);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
	GL_MBindRect (GL_TEXTURE1_ARB, depthMap->texnum);

	R_DrawFullScreenQuad ();

	GL_BindNullProgram ();
}

void R_FXAA (void) {

	if (!r_fxaa->value)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	// setup program
	GL_BindProgram (fxaaProgram, 0);

	if (!fxaatex) {
		qglGenTextures (1, &fxaatex);
		GL_MBind (GL_TEXTURE0_ARB, fxaatex);
		qglTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglCopyTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, 0, 0, vid.width, vid.height, 0);
	}
	GL_MBind (GL_TEXTURE0_ARB, fxaatex);
	qglCopyTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, 0, 0, vid.width, vid.height);
	qglUniform1i(fxaa_screenTex, 0);
	qglUniform2f(fxaa_screenSize, vid.width, vid.height);
	qglUniformMatrix4fv(fxaa_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad ();

	GL_BindNullProgram ();

}

void R_FilmGrain (void) 
{

	if (!r_filmGrain->value)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	// setup program
	GL_BindProgram (filmGrainProgram, 0);

	GL_MBindRect (GL_TEXTURE0_ARB, ScreenMap->texnum);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);

	qglUniform1i (film_tex, 0);
	qglUniform1f (film_scroll, -3 * (r_newrefdef.time / 40.0));
	qglUniformMatrix4fv(film_matrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad ();

	GL_BindNullProgram ();
}

void R_GammaRamp (void) 
{
	GL_BindProgram (gammaProgram, 0);

	GL_MBindRect (GL_TEXTURE0_ARB, ScreenMap->texnum);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);

	qglUniform1i (gamma_screenMap, 0);
	qglUniform4f (gamma_control, r_brightness->value, r_contrast->value, r_saturation->value, 1 / r_brightness->value);
	qglUniformMatrix4fv(gamma_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad ();

	GL_BindNullProgram ();
}

void R_MotionBlur (void) 
{
	float	temp_x, temp_y, 
			delta_x, delta_y;

	if (!r_motionBlur->value)
		return;

	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;

	// calc camera offsets
	temp_y = r_newrefdef.viewangles_old[0] - r_newrefdef.viewangles[0]; //PITCH up-down
	temp_x = r_newrefdef.viewangles_old[1] - r_newrefdef.viewangles[1]; //YAW left-right
	delta_x = (temp_x * 2.0 / r_newrefdef.fov_x) * r_motionBlurFrameLerp->value;
	delta_y = (temp_y * 2.0 / r_newrefdef.fov_y) * r_motionBlurFrameLerp->value;

	// setup program
	GL_BindProgram(motionBlurProgram, 0);

	qglUniform2f (mb_vel, delta_x, delta_y);
	qglUniform1i (mb_samples, r_motionBlurSamples->value);
	qglUniformMatrix4fv(mb_matrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);
	qglUniform1i (mb_tex, 0);
	qglUniform1i (mb_mask, 1);

	GL_MBindRect (GL_TEXTURE0_ARB, ScreenMap->texnum);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);

	GL_MBindRect (GL_TEXTURE1_ARB, weaponHack->texnum);

	R_DrawFullScreenQuad ();

	GL_BindNullProgram ();
}

void R_DownsampleDepth(void) 
{
	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;
	
	if (!r_ssao->value)
		return;

//	qglViewport (0, 0, vid.width, vid.height);

//	GL_LoadIdentity(GL_MODELVIEW);
//	GL_LoadIdentity(GL_PROJECTION);
//	qglOrtho(0, vid.width, vid.height, 0, -99999, 99999);

	GL_DepthRange(0.0, 1.0);
	GL_DepthMask(0);
	GL_ColorMask(1, 1, 1, 1);
	GL_Disable(GL_CULL_FACE);
	GL_Disable(GL_DEPTH_TEST);

	// downsample the depth buffer
	qglBindFramebuffer(GL_FRAMEBUFFER, fboId);
	qglDrawBuffer(GL_COLOR_ATTACHMENT2);

	GL_BindProgram(depthDownsampleProgram, 0);
	GL_MBindRect(GL_TEXTURE0_ARB, depthMap->texnum);

	qglUniform1i(depthDS_depth, 0);
	qglUniform2f(depthDS_params, r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);
	qglUniformMatrix4fv(depthDS_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawHalfScreenQuad();

	// restore settings
	qglBindFramebuffer(GL_FRAMEBUFFER, 0);
	GL_BindNullProgram();

//	GL_LoadMatrix(GL_PROJECTION, r_newrefdef.projectionMatrix);
//	GL_LoadMatrix(GL_MODELVIEW, r_newrefdef.modelViewMatrix);
	qglViewport(r_newrefdef.viewport[0], r_newrefdef.viewport[1], r_newrefdef.viewport[2], r_newrefdef.viewport[3]);
}

void R_SSAO (void) 
{
	int i, j, id, numSamples;

	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;

	if (!r_ssao->value)
		return;
	
//	qglViewport(0, 0, vid.width, vid.height);

//	GL_LoadIdentity(GL_MODELVIEW);
//	GL_LoadIdentity(GL_PROJECTION);
//	qglOrtho(0, vid.width, vid.height, 0, -99999, 99999);
	
	GL_Disable (GL_DEPTH_TEST);
	GL_Disable (GL_CULL_FACE);
	GL_DepthMask(0);
	GL_ColorMask(1, 1, 1, 1);

	// process
	qglBindFramebuffer(GL_FRAMEBUFFER, fboId);
	qglDrawBuffer(GL_COLOR_ATTACHMENT0);

	GL_BindProgram (ssaoProgram, 0);
	GL_MBindRect(GL_TEXTURE0_ARB, fboDN);
	GL_MBind(GL_TEXTURE1_ARB, r_randomNormalTex->texnum);

	qglUniform1i (ssao_mini, 0);
	qglUniform1i (ssao_rand, 1);
	qglUniform2f (ssao_params, max(r_ssaoIntensity->value, 0.f), r_ssaoScale->value);
	qglUniform2f (ssao_vp, vid.width, vid.height);
	qglUniformMatrix4fv(ssao_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawHalfScreenQuad();

	// blur
	fboColorIndex = 0;

	if (r_ssaoBlur->value) {
		qglBindFramebuffer(GL_FRAMEBUFFER, fboId);
		GL_MBindRect(GL_TEXTURE1_ARB, fboDN);

		GL_BindProgram(ssaoBlurProgram, 0);
		id = ssaoBlurProgram->id[0];
		qglUniform1i(ssaoB_mColor, 0);
		qglUniform1i(ssaoB_mDepth, 1);
		qglUniformMatrix4fv(ssaoB_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

		numSamples = (int)rintf(4.f * vid.height / 1080.f);
		qglUniform1i(ssaoB_sapmles, max(numSamples, 1));

		for (i = 0; i < (int)r_ssaoBlur->value; i++) {
#if 1
			// two-pass shader
			for (j = 0; j < 2; j++) {
				GL_MBindRect(GL_TEXTURE0_ARB, fboColor[j]);
				qglDrawBuffer(GL_COLOR_ATTACHMENT0 + (j ^ 1));
				qglUniform2f(ssaoB_axisMask, j ? 0.f : 1.f, j ? 1.f : 0.f);
				R_DrawHalfScreenQuad();
			}
#else
			// single-pass shader
			GL_MBindRect(GL_TEXTURE0_ARB, fboColor[fboColorIndex]);
			fboColorIndex ^= 1;
			qglDrawBuffer(GL_COLOR_ATTACHMENT0 + fboColorIndex);
			R_DrawHalfScreenQuad();
#endif
		}
	}

	// restore
	qglBindFramebuffer(GL_FRAMEBUFFER, 0);
	GL_BindNullProgram ();

	GL_Enable(GL_CULL_FACE);
	GL_Enable(GL_DEPTH_TEST);
	GL_DepthMask(1);

//	GL_LoadMatrix(GL_PROJECTION, r_newrefdef.projectionMatrix);
//	GL_LoadMatrix(GL_MODELVIEW, r_newrefdef.modelViewMatrix);
	qglViewport(r_newrefdef.viewport[0], r_newrefdef.viewport[1], r_newrefdef.viewport[2], r_newrefdef.viewport[3]);
}
