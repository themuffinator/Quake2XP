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
	unsigned	flareIndex[MAX_INDICES];
	int			flareVert = 0, index = 0;

	if (!r_skipStaticLights->value)
		return;

	if (light->surf->ent) {

		if (!VectorCompare (light->surf->ent->angles, vec3_origin))
			return;

		qglPushMatrix ();
		R_RotateForEntity (light->surf->ent);

	}

	light->surf->visframe = r_framecount;

	if (flareVert) {

		if (gl_state.DrawRangeElements && r_DrawRangeElements->value)
			qglDrawRangeElementsEXT (GL_TRIANGLES, 0, flareVert, index, GL_UNSIGNED_INT, flareIndex);
		else
			qglDrawElements (GL_TRIANGLES, index, GL_UNSIGNED_INT, flareIndex);

		flareVert = 0;
		index = 0;
	}

	// Color Fade
	VectorSubtract (light->origin, r_origin, v);
	dist = VectorLength (v) * (light->size * 0.01);
	dist2 = VectorLength (v);

	scale = ((1024 - dist2) / 1024) * 0.5;

	VectorScale (light->color, scale, tmp);

	VectorMA (light->origin, -1 - dist, vup, vert_array[flareVert + 0]);
	VectorMA (vert_array[flareVert + 0], 1 + dist, vright, vert_array[flareVert + 0]);
	VA_SetElem2 (tex_array[flareVert + 0], 0, 1);
	VA_SetElem4 (color_array[flareVert + 0], tmp[0], tmp[1], tmp[2], 1);

	VectorMA (light->origin, -1 - dist, vup, vert_array[flareVert + 1]);
	VectorMA (vert_array[flareVert + 1], -1 - dist, vright, vert_array[flareVert + 1]);
	VA_SetElem2 (tex_array[flareVert + 1], 0, 0);
	VA_SetElem4 (color_array[flareVert + 1], tmp[0], tmp[1], tmp[2], 1);

	VectorMA (light->origin, 1 + dist, vup, vert_array[flareVert + 2]);
	VectorMA (vert_array[flareVert + 2], -1 - dist, vright, vert_array[flareVert + 2]);
	VA_SetElem2 (tex_array[flareVert + 2], 1, 0);
	VA_SetElem4 (color_array[flareVert + 2], tmp[0], tmp[1], tmp[2], 1);

	VectorMA (light->origin, 1 + dist, vup, vert_array[flareVert + 3]);
	VectorMA (vert_array[flareVert + 3], 1 + dist, vright, vert_array[flareVert + 3]);
	VA_SetElem2 (tex_array[flareVert + 3], 1, 1);
	VA_SetElem4 (color_array[flareVert + 3], tmp[0], tmp[1], tmp[2], 1);

	flareIndex[index++] = flareVert + 0;
	flareIndex[index++] = flareVert + 1;
	flareIndex[index++] = flareVert + 3;
	flareIndex[index++] = flareVert + 3;
	flareIndex[index++] = flareVert + 1;
	flareIndex[index++] = flareVert + 2;

	flareVert += 4;


	if (flareVert) {
		if (gl_state.DrawRangeElements && r_DrawRangeElements->value)
			qglDrawRangeElementsEXT (GL_TRIANGLES, 0, flareVert, index, GL_UNSIGNED_INT, flareIndex);
		else
			qglDrawElements (GL_TRIANGLES, index, GL_UNSIGNED_INT, flareIndex);

	}

	if (light->surf->ent)
		qglPopMatrix ();

	c_flares++;
}

qboolean PF_inPVS (vec3_t p1, vec3_t p2);

void R_RenderFlares (void) {
	int i, id;
	flare_t *fl;
	unsigned defBits = 0;

	if (!r_drawFlares->value)
		return;
	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;

	qglEnableVertexAttribArray (ATRB_POSITION);
	qglEnableVertexAttribArray (ATRB_TEX0);
	qglEnableVertexAttribArray (ATRB_COLOR);

	qglVertexAttribPointer (ATRB_POSITION, 3, GL_FLOAT, false, 0, vert_array);
	qglVertexAttribPointer (ATRB_TEX0, 2, GL_FLOAT, false, 0, tex_array);
	qglVertexAttribPointer (ATRB_COLOR, 4, GL_FLOAT, false, 0, color_array);

	GL_DepthMask (0);
	GL_Enable (GL_BLEND);
	GL_BlendFunc (GL_ONE, GL_ONE);

	GL_BindProgram (particlesProgram, defBits);
	id = particlesProgram->id[defBits];

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

		if (r_softParticles->value)
			qglUniform1f (qglGetUniformLocation (id, "u_thickness"), fl->size * 1.5);
		else
			qglUniform1f (qglGetUniformLocation (id, "u_thickness"), 0.0);

		R_BuildFlares (fl);

	}
	GL_BindNullProgram ();
	qglDisableVertexAttribArray (ATRB_POSITION);
	qglDisableVertexAttribArray (ATRB_TEX0);
	qglDisableVertexAttribArray (ATRB_COLOR);
	GL_Disable (GL_BLEND);
	GL_DepthMask (1);
	//	GL_SelectTexture(GL_TEXTURE0_ARB);
}

/*
====================
GLSL Full Screen
Post Process Effects
====================
*/

void R_DrawFullScreenQuad () {
	qglBindBuffer (GL_ARRAY_BUFFER_ARB, gl_state.vbo_fullScreenQuad);
	qglEnableVertexAttribArray (ATRB_POSITION);
	qglVertexAttribPointer (ATRB_POSITION, 2, GL_FLOAT, false, 0, 0);

	qglDrawArrays (GL_TRIANGLE_FAN, 0, 4);

	qglDisableVertexAttribArray (ATRB_POSITION);
	qglBindBuffer (GL_ARRAY_BUFFER_ARB, 0);
}

void R_DrawHalfScreenQuad () {
	qglBindBuffer (GL_ARRAY_BUFFER_ARB, gl_state.vbo_halfScreenQuad);
	qglEnableVertexAttribArray (ATRB_POSITION);
	qglVertexAttribPointer (ATRB_POSITION, 2, GL_FLOAT, false, 0, 0);

	qglDrawArrays (GL_TRIANGLE_FAN, 0, 4);

	qglDisableVertexAttribArray (ATRB_POSITION);
	qglBindBuffer (GL_ARRAY_BUFFER_ARB, 0);
}

void R_DrawQuarterScreenQuad () {
	qglBindBuffer (GL_ARRAY_BUFFER_ARB, gl_state.vbo_quarterScreenQuad);
	qglEnableVertexAttribArray (ATRB_POSITION);
	qglVertexAttribPointer (ATRB_POSITION, 2, GL_FLOAT, false, 0, 0);

	qglDrawArrays (GL_TRIANGLE_FAN, 0, 4);

	qglDisableVertexAttribArray (ATRB_POSITION);
	qglBindBuffer (GL_ARRAY_BUFFER_ARB, 0);
}

void R_Bloom (void) {

	unsigned	defBits = 0;
	int			id;

	if (!r_bloom->value)
		return;
	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;

	// downsample and cut color
	GL_MBindRect (GL_TEXTURE0_ARB, ScreenMap->texnum);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);

	// setup program
	GL_BindProgram (bloomdsProgram, defBits);
	id = bloomdsProgram->id[defBits];

	qglUniform1f (qglGetUniformLocation (id, "u_BloomThreshold"), r_bloomThreshold->value);
	qglUniform1i (qglGetUniformLocation (id, "u_map"), 0);

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

	GL_BindProgram (blurStarProgram, defBits);
	id = blurStarProgram->id[defBits];
	qglUniform1i (qglGetUniformLocation (id, "u_map"), 0);
	qglUniform1f (qglGetUniformLocation (id, "u_starIntens"), r_bloomStarIntens->value);

	R_DrawQuarterScreenQuad ();
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

	// blur x
	GL_BindRect (bloomtex);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

	GL_BindProgram (gaussXProgram, defBits);
	id = gaussXProgram->id[defBits];
	qglUniform1i (qglGetUniformLocation (id, "u_map"), 0);

	R_DrawQuarterScreenQuad ();

	// blur y
	GL_BindRect (bloomtex);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

	GL_BindProgram (gaussYProgram, defBits);
	id = gaussYProgram->id[defBits];
	qglUniform1i (qglGetUniformLocation (id, "u_map"), 0);

	R_DrawQuarterScreenQuad ();

	// store 2 pass gauss blur 
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

	//final pass
	GL_BindProgram (bloomfpProgram, defBits);
	id = bloomfpProgram->id[defBits];

	GL_MBindRect (GL_TEXTURE0_ARB, ScreenMap->texnum);
	qglUniform1i (qglGetUniformLocation (id, "u_map0"), 0);

	GL_MBindRect (GL_TEXTURE1_ARB, bloomtex);
	qglUniform1i (qglGetUniformLocation (id, "u_map1"), 1);
	qglUniform3f (qglGetUniformLocation (id, "u_bloomParams"), r_bloomIntens->value, r_bloomBright->value, r_bloomExposure->value);

	R_DrawFullScreenQuad ();

	GL_BindNullProgram ();
}



void R_ThermalVision (void) {

	unsigned defBits = 0;
	int	id;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
	if (!(r_newrefdef.rdflags & RDF_IRGOGGLES))
		return;

	GL_SelectTexture (GL_TEXTURE0_ARB);

	if (!thermaltex) {
		qglGenTextures (1, &thermaltex);
		GL_BindRect (thermaltex);
		qglTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglCopyTexImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, 0, 0, vid.width, vid.height, 0);
	}
	else {
		GL_BindRect (thermaltex);
		qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
	}

	// setup program
	GL_BindProgram (thermalProgram, defBits);
	id = thermalProgram->id[defBits];
	qglUniform1i (qglGetUniformLocation (id, "u_screenTex"), 0);

	R_DrawHalfScreenQuad ();

	// create thermal texture (set to zero in default state)
	if (!thermaltex) {
		qglGenTextures (1, &thermaltex);
		GL_BindRect (thermaltex);
		qglTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglCopyTexImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, 0, 0, vid.width*0.5, vid.height*0.5, 0);
	}

	// blur x
	GL_BindRect (thermaltex);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.5, vid.height*0.5);

	GL_BindProgram (gaussXProgram, defBits);
	id = gaussXProgram->id[defBits];
	qglUniform1i (qglGetUniformLocation (id, "u_map"), 0);

	R_DrawHalfScreenQuad ();

	// blur y
	GL_BindRect (thermaltex);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.5, vid.height*0.5);

	GL_BindProgram (gaussYProgram, defBits);
	id = gaussYProgram->id[defBits];
	qglUniform1i (qglGetUniformLocation (id, "u_map"), 0);

	R_DrawHalfScreenQuad ();

	// store 2 pass gauss blur 
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.5, vid.height*0.5);

	//final pass
	GL_BindProgram (thermalfpProgram, defBits);
	id = thermalfpProgram->id[defBits];

	GL_BindRect (thermaltex);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
	qglUniform1i (qglGetUniformLocation (id, "u_map"), 0);

	R_DrawFullScreenQuad ();

	GL_BindNullProgram ();
}



void R_RadialBlur (void) {

	unsigned	defBits = 0;
	int			id;
	float		blur;

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
		GL_BindProgram (radialProgram, defBits);
		id = radialProgram->id[defBits];
		qglUniform1i (qglGetUniformLocation (id, "u_screenMap"), 0);

		if (r_newrefdef.rdflags & RDF_PAIN)
			blur = 0.01;
		else if (r_newrefdef.rdflags & RDF_UNDERWATER)
			blur = 0.0085;
		else
			blur = 0.01;

		// xy = radial center screen space position, z = radius attenuation, w = blur strength
		qglUniform4f (qglGetUniformLocation (id, "u_radialBlurParams"), vid.width*0.5, vid.height*0.5, 1.0 / vid.height, blur);

		R_DrawFullScreenQuad ();

		GL_BindNullProgram ();
	}
}

void R_DofBlur (void) {
	unsigned		defBits = 0;
	int				id;
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

		trace = CL_PMTraceWorld (r_newrefdef.vieworg, vec3_origin, vec3_origin, right, MASK_SHOT, true);
		VectorSubtract (trace.endpos, r_newrefdef.vieworg, tmp);
		tmpDist[0] = VectorLength (tmp);

		trace = CL_PMTraceWorld (r_newrefdef.vieworg, vec3_origin, vec3_origin, left, MASK_SHOT, true);
		VectorSubtract (trace.endpos, r_newrefdef.vieworg, tmp);
		tmpDist[1] = VectorLength (tmp);

		trace = CL_PMTraceWorld (r_newrefdef.vieworg, vec3_origin, vec3_origin, up, MASK_SHOT, true);
		VectorSubtract (trace.endpos, r_newrefdef.vieworg, tmp);
		tmpDist[2] = VectorLength (tmp);

		trace = CL_PMTraceWorld (r_newrefdef.vieworg, vec3_origin, vec3_origin, dn, MASK_SHOT, true);
		VectorSubtract (trace.endpos, r_newrefdef.vieworg, tmp);
		tmpDist[3] = VectorLength (tmp);

		trace = CL_PMTraceWorld (r_newrefdef.vieworg, vec3_origin, vec3_origin, end_trace, MASK_SHOT, true);
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
	GL_BindProgram (dofProgram, defBits);
	id = dofProgram->id[defBits];
	qglUniform2f (qglGetUniformLocation (id, "u_screenSize"), vid.width, vid.height);
	qglUniform4f (qglGetUniformLocation (id, "u_dofParams"), dofParams[0], dofParams[1], r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);

	GL_MBindRect (GL_TEXTURE0_ARB, ScreenMap->texnum);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
	qglUniform1i (qglGetUniformLocation (id, "u_ScreenTex"), 0);

	GL_MBindRect (GL_TEXTURE1_ARB, depthMap->texnum);
	qglUniform1i (qglGetUniformLocation (id, "u_DepthTex"), 1);

	R_DrawFullScreenQuad ();

	GL_BindNullProgram ();
}

void R_FXAA (void) {

	unsigned	defBits = 0;
	int			id;

	if (!r_fxaa->value)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	// setup program
	GL_BindProgram (fxaaProgram, defBits);
	id = fxaaProgram->id[defBits];

	if (!fxaatex) {
		qglGenTextures (1, &fxaatex);
		GL_MBind (GL_TEXTURE0_ARB, fxaatex);
		qglTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglCopyTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, 0, 0, vid.width, vid.height, 0);
	}
	GL_MBind (GL_TEXTURE0_ARB, fxaatex);
	qglCopyTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, 0, 0, vid.width, vid.height);
	qglUniform1i (qglGetUniformLocation (id, "u_ScreenTex"), 0);
	qglUniform2f (qglGetUniformLocation (id, "u_ScreenSize"), vid.width, vid.height);

	R_DrawFullScreenQuad ();

	GL_BindNullProgram ();

}

void R_FilmGrain (void) {

	unsigned	defBits = 0;
	int			id;

	if (!r_filmGrain->value)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	// setup program
	GL_BindProgram (filmGrainProgram, defBits);
	id = filmGrainProgram->id[defBits];

	GL_MBindRect (GL_TEXTURE0_ARB, ScreenMap->texnum);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
	qglUniform1i (qglGetUniformLocation (id, "u_ScreenTex"), 0);
	qglUniform1f (qglGetUniformLocation (id, "u_scroll"), -3 * (r_newrefdef.time / 40.0));

	R_DrawFullScreenQuad ();

	GL_BindNullProgram ();
}

void R_GammaRamp (void) {

	unsigned	defBits = 0;
	int			id;

	// setup program
	GL_BindProgram (gammaProgram, defBits);
	id = gammaProgram->id[defBits];

	GL_MBindRect (GL_TEXTURE0_ARB, ScreenMap->texnum);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
	qglUniform1i (gamma_screenMap, 0);
	qglUniform4f (gamma_control, r_brightness->value, r_contrast->value, r_saturation->value, 1 / r_gamma->value);

	R_DrawFullScreenQuad ();

	GL_BindNullProgram ();
}

void R_MotionBlur (void) {

	unsigned	defBits = 0;
	int			id;
	float		temp_x, temp_y, delta_x, delta_y;

	if (!r_motionBlur->value)
		return;

	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;

	// setup program
	GL_BindProgram (motionBlurProgram, defBits);
	id = motionBlurProgram->id[defBits];
	// calc camera offsets
	temp_y = r_newrefdef.viewangles_old[0] - r_newrefdef.viewangles[0]; //PITCH up-down
	temp_x = r_newrefdef.viewangles_old[1] - r_newrefdef.viewangles[1]; //YAW left-right
	delta_x = (temp_x * 2.0 / r_newrefdef.fov_x) * r_motionBlurFrameLerp->value;
	delta_y = (temp_y * 2.0 / r_newrefdef.fov_y) * r_motionBlurFrameLerp->value;

	qglUniform2f (qglGetUniformLocation (id, "u_velocity"), delta_x, delta_y);
	qglUniform1i (qglGetUniformLocation (id, "u_numSamples"), r_motionBlurSamples->value);

	GL_MBindRect (GL_TEXTURE0_ARB, ScreenMap->texnum);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
	qglUniform1i (qglGetUniformLocation (id, "u_ScreenTex"), 0);

	GL_MBindRect (GL_TEXTURE1_ARB, weaponHack->texnum);
	qglUniform1i (qglGetUniformLocation (id, "u_MaskTex"), 1);

	R_DrawFullScreenQuad ();

	GL_BindNullProgram ();
}

void R_DownsampleDepth(void) {
	int	id;
	
	if (!r_ssao->value)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	qglViewport (0, 0, vid.width, vid.height);

	GL_LoadIdentity(GL_MODELVIEW);
	GL_LoadIdentity(GL_PROJECTION);
	qglOrtho(0, vid.width, vid.height, 0, -99999, 99999);

	GL_DepthRange(0.0, 1.0);
	GL_DepthMask(1);
	GL_ColorMask(0, 0, 0, 0);
	GL_Disable(GL_CULL_FACE);
	GL_DepthFunc(GL_ALWAYS);

	// downsample the depth buffer
	qglBindFramebuffer(GL_FRAMEBUFFER, gl_state.fboId);
	qglDrawBuffer(GL_DEPTH_ATTACHMENT);

	GL_BindProgram(depthDownsampleProgram, 0);
	id = depthDownsampleProgram->id[0];
	GL_MBindRect(GL_TEXTURE0_ARB, depthMap->texnum);

	qglUniform1i(qglGetUniformLocation(id, "u_depthBufferMap"), 0);
	qglUniform2f(qglGetUniformLocation(id, "u_depthParms"), r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);

	R_DrawHalfScreenQuad();
	qglBindFramebuffer(GL_FRAMEBUFFER, 0);

	// restore settings
	GL_BindNullProgram();

	GL_ColorMask(1, 1, 1, 1);
	GL_Enable(GL_CULL_FACE);
	GL_DepthFunc(GL_LEQUAL);
	GL_LoadMatrix(GL_PROJECTION, r_newrefdef.projectionMatrix);
	GL_LoadMatrix(GL_MODELVIEW, r_newrefdef.modelViewMatrix);

	qglViewport(r_newrefdef.viewport[0], r_newrefdef.viewport[1], r_newrefdef.viewport[2], r_newrefdef.viewport[3]);
}

void R_SSAO (void) {
	int id, numSamples;

	if (!r_ssao->value)
		return;
	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;

	// set 2D virtual screen size
	qglViewport (0, 0, vid.width, vid.height);

	qglMatrixMode (GL_PROJECTION);
	qglPushMatrix ();
	qglLoadIdentity ();
	qglOrtho (0, vid.width, vid.height, 0, -99999, 99999);

	qglMatrixMode (GL_MODELVIEW);
	qglPushMatrix ();
	qglLoadIdentity ();

	GL_Disable (GL_DEPTH_TEST);
	GL_Disable (GL_CULL_FACE);
	GL_DepthMask(0);
	GL_ColorMask(1, 1, 1, 1);

	// ssao process
	qglBindFramebuffer(GL_FRAMEBUFFER, gl_state.fboId);
	qglDrawBuffer(GL_COLOR_ATTACHMENT0);

	GL_BindProgram (ssaoProgram, 0);
	id = ssaoProgram->id[0];
	GL_MBindRect(GL_TEXTURE0_ARB, fboDepth);
	GL_MBind(GL_TEXTURE1_ARB, r_randomNormalTex->texnum);

	qglUniform1i (qglGetUniformLocation (id, "u_depthBufferMiniMap"), 0);
	qglUniform1i (qglGetUniformLocation (id, "u_randomNormalMap"), 1);
	qglUniform2f (qglGetUniformLocation (id, "u_depthParms"), r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);
	qglUniform2f (qglGetUniformLocation (id, "u_ssaoParms"), max(r_ssaoIntensity->value, 0.f), r_ssaoScale->value);
	qglUniform2f (qglGetUniformLocation (id, "u_viewport"), vid.width, vid.height);

	R_DrawHalfScreenQuad();

	// ssao blur
	qglBindFramebuffer(GL_FRAMEBUFFER, gl_state.fboId);
	GL_MBindRect(GL_TEXTURE1_ARB, fboDepth);

	GL_BindProgram(ssaoBlurProgram, 0);
	id = ssaoBlurProgram->id[0];
	qglUniform1i(qglGetUniformLocation(id, "u_colorMiniMap"), 0);
	qglUniform1i(qglGetUniformLocation(id, "u_depthBufferMiniMap"), 1);
	qglUniform2f(qglGetUniformLocation(id, "u_depthParms"), r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);

	numSamples = 4 * vid.height / 1080;
	qglUniform1i(qglGetUniformLocation(id, "u_numSamples"), numSamples);

	// two-pass shader
	if (r_ssaoBlur->value) {
		// horizontal
		GL_MBindRect(GL_TEXTURE0_ARB, fboColor[0]);
		qglDrawBuffer(GL_COLOR_ATTACHMENT1);
		qglUniform2f(qglGetUniformLocation(id, "u_axisMask"), 1.f, 0.f);

		R_DrawHalfScreenQuad();

		// vertical
		GL_MBindRect(GL_TEXTURE0_ARB, fboColor[1]);
		qglDrawBuffer(GL_COLOR_ATTACHMENT0);
		qglUniform2f(qglGetUniformLocation(id, "u_axisMask"), 0.f, 1.f);

		R_DrawHalfScreenQuad();
	}

	fboColorIndex = 0;

/*
	// for 4 sample shader, multi-pass
	if (r_ssaoBlur->value) {
		GL_MBindRect(GL_TEXTURE0_ARB, fboColor1);
		qglDrawBuffer(GL_COLOR_ATTACHMENT0);
		R_DrawHalfScreenQuad();
		GL_MBindRect(GL_TEXTURE0_ARB, fboColor0);
		qglDrawBuffer(GL_COLOR_ATTACHMENT1);
		R_DrawHalfScreenQuad();
		GL_MBindRect(GL_TEXTURE0_ARB, fboColor1);
		qglDrawBuffer(GL_COLOR_ATTACHMENT0);
		R_DrawHalfScreenQuad();
		GL_MBindRect(GL_TEXTURE0_ARB, fboColor0);
		qglDrawBuffer(GL_COLOR_ATTACHMENT1);
		R_DrawHalfScreenQuad();
	}
*/

	// restore
	qglBindFramebuffer(GL_FRAMEBUFFER, 0);
	GL_BindNullProgram ();

	qglPopMatrix ();
	qglMatrixMode (GL_PROJECTION);
	qglPopMatrix ();
	qglMatrixMode (GL_MODELVIEW);
	GL_DepthMask(1);
	GL_Enable (GL_CULL_FACE);
	GL_Enable (GL_DEPTH_TEST);
}
