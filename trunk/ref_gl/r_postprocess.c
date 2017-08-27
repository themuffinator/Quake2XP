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

/*
====================
GLSL Full Screen
Post Process Effects
====================
*/

void R_DrawFullScreenQuad () {

	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_fullScreenQuad);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadTris);
	
	qglEnableVertexAttribArray (ATT_POSITION);
	qglVertexAttribPointer (ATT_POSITION, 2, GL_FLOAT, qfalse, 0, 0);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

	qglDisableVertexAttribArray (ATT_POSITION);
	qglDisableVertexAttribArray (ATT_NORMAL);

	qglBindBuffer(GL_ARRAY_BUFFER, 0);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void R_DrawHalfScreenQuad () {

	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_halfScreenQuad);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadTris);
	
	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 2, GL_FLOAT, qfalse, 0, 0);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

	qglDisableVertexAttribArray(ATT_POSITION);
	
	qglBindBuffer(GL_ARRAY_BUFFER, 0);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void R_DrawQuarterScreenQuad () {
	
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_quarterScreenQuad);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadTris);
	
	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 2, GL_FLOAT, qfalse, 0, 0);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

	qglDisableVertexAttribArray(ATT_POSITION);
	
	qglBindBuffer(GL_ARRAY_BUFFER, 0);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void R_Bloom (void) 
{
	if (!r_bloom->integer)
		return;

	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;

	// downsample and cut color
	GL_MBindRect (GL_TEXTURE0, ScreenMap->texnum);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width, vid.height);

	// setup program
	GL_BindProgram (bloomdsProgram, 0);
	qglUniform1f(bloomDS_threshold, r_bloomThreshold->value);
	qglUniformMatrix4fv(bloomDS_matrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawQuarterScreenQuad ();

	// create bloom texture (set to zero in default state)
	if (!bloomtex) {
		qglGenTextures (1, &bloomtex);
		GL_BindRect (bloomtex);
		qglTexParameteri (GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameteri (GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglCopyTexImage2D (GL_TEXTURE_RECTANGLE, 0, GL_RGB, 0, 0, vid.width*0.25, vid.height*0.25, 0);
	}

	// generate star shape
	GL_BindRect (bloomtex);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

	GL_BindProgram (glareProgram, 0);
	qglUniform1f(glare_params, r_bloomWidth->value);
	qglUniformMatrix4fv(glare_matrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawQuarterScreenQuad ();
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

	// blur x
	GL_BindRect (bloomtex);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

	GL_BindProgram (gaussXProgram, 0);
	qglUniformMatrix4fv(gaussx_matrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawQuarterScreenQuad ();

	// blur y
	GL_BindRect (bloomtex);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

	GL_BindProgram (gaussYProgram, 0);
	qglUniformMatrix4fv(gaussy_matrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawQuarterScreenQuad ();

	// store 2 pass gauss blur 
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

	//final pass
	GL_BindProgram (bloomfpProgram, 0);
	GL_MBindRect (GL_TEXTURE0, ScreenMap->texnum);
	GL_MBindRect (GL_TEXTURE1, bloomtex);
	qglUniform1f(bloomFP_params, r_bloomIntens->value);
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
		GL_MBindRect(GL_TEXTURE0, thermaltex);
		qglTexParameteri (GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameteri (GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglCopyTexImage2D (GL_TEXTURE_RECTANGLE, 0, GL_RGB, 0, 0, vid.width, vid.height, 0);
	}
	else {
		GL_MBindRect(GL_TEXTURE0, thermaltex);
		qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width, vid.height);
	}

	// setup program
	GL_BindProgram (thermalProgram, 0);
	qglUniformMatrix4fv(therm_matrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawHalfScreenQuad ();

	// blur x
	GL_BindRect (thermaltex);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width*0.5, vid.height*0.5);

	GL_BindProgram (gaussXProgram, 0);
	qglUniformMatrix4fv(gaussx_matrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawHalfScreenQuad ();

	// blur y
	GL_BindRect (thermaltex);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width*0.5, vid.height*0.5);

	GL_BindProgram (gaussYProgram, 0);
	qglUniformMatrix4fv(gaussy_matrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawHalfScreenQuad ();

	// store 2 pass gauss blur 
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width*0.5, vid.height*0.5);

	//final pass
	GL_BindProgram (thermalfpProgram, 0);

	GL_BindRect (thermaltex);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width, vid.height);
	qglUniformMatrix4fv(thermf_matrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad ();

	GL_BindNullProgram ();
}



void R_RadialBlur (void) 
{
	float	blur;

	if (!r_radialBlur->integer)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
	
	if (r_newrefdef.fov_x <= r_radialBlurFov->value)
		goto hack;

	if (r_newrefdef.rdflags & (RDF_UNDERWATER | RDF_PAIN)) {

	hack:

	
		GL_MBindRect (GL_TEXTURE0, ScreenMap->texnum);
		qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width, vid.height);

		// setup program
		GL_BindProgram (radialProgram, 0);

		if (r_newrefdef.rdflags & RDF_UNDERWATER)
			blur = 0.0065;
		else
			blur = 0.01;

		// xy = radial center screen space position, z = radius attenuation, w = blur strength
		qglUniform4f(rb_params, vid.width*0.5, vid.height*0.5, 1.0 / vid.height, blur);
		qglUniformMatrix4fv(rb_matrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

		R_DrawFullScreenQuad ();

		GL_BindNullProgram ();
	}
}

extern float v_blend[4];

void R_ScreenBlend(void)
{

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
	
	if (!v_blend[3])
		return;

		GL_MBindRect(GL_TEXTURE0, ScreenMap->texnum);
		qglCopyTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width, vid.height);

		// setup program
		GL_BindProgram(genericProgram, 0);

		qglUniform1i(gen_attribConsole, 0);
		qglUniform1i(gen_attribColors, 0);
		qglUniform1i(gen_sky, 0);
		qglUniform1i(gen_3d, 0);
		qglUniform4f(gen_color, v_blend[0], v_blend[1], v_blend[2], v_blend[3]);
		qglUniformMatrix4fv(gen_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

		GL_Enable(GL_BLEND);

		R_DrawFullScreenQuad();

		GL_Disable(GL_BLEND);

		GL_BindNullProgram();

}
void R_DofBlur (void) 
{
	float			tmpDist[5], tmpMins[3];
	vec2_t          dofParams;
	trace_t			trace;
	vec3_t			end_trace, v_f, v_r, v_up, tmp, left, right, up, dn;

	if (!r_dof->integer)
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
	qglUniformMatrix4fv(dof_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	GL_MBindRect (GL_TEXTURE0, ScreenMap->texnum);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width, vid.height);
	GL_MBindRect (GL_TEXTURE1, depthMap->texnum);

	R_DrawFullScreenQuad ();

	GL_BindNullProgram ();
}

void R_FXAA (void) {

	if (!r_fxaa->integer)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	// setup program
	GL_BindProgram (fxaaProgram, 0);

	if (!fxaatex) {
		qglGenTextures (1, &fxaatex);
		GL_MBind (GL_TEXTURE0, fxaatex);
		qglTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglCopyTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, 0, 0, vid.width, vid.height, 0);
	}
	GL_MBind (GL_TEXTURE0, fxaatex);
	qglCopyTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, 0, 0, vid.width, vid.height);

	qglUniform2f(fxaa_screenSize, vid.width, vid.height);
	qglUniformMatrix4fv(fxaa_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad ();

	GL_BindNullProgram ();

}

void R_FilmFilter (void) 
{

	if (!r_filmFilterType->integer)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
	 
	// setup program
	GL_BindProgram (filmGrainProgram, 0);

	GL_MBindRect (GL_TEXTURE0, ScreenMap->texnum);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width, vid.height);

	qglUniform2f (film_screenRes, vid.width, vid.height);
	qglUniform1f (film_rand, crand());
	qglUniform1i (film_frameTime, r_framecount);
	qglUniform4f (film_params,	r_filmFilterType->integer, r_filmFilterNoiseIntens->value, 
								r_filmFilterScratchIntens->value, r_filmFilterVignetIntens->value);

	qglUniformMatrix4fv(film_matrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad ();

	GL_BindNullProgram ();
}

void R_GammaRamp (void) 
{
	GL_BindProgram (gammaProgram, 0);

	GL_MBindRect (GL_TEXTURE0, ScreenMap->texnum);
	qglCopyTexSubImage2D (GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width, vid.height);

	qglUniform4f (gamma_control, r_brightness->value, r_contrast->value, r_saturation->value, 1 / r_gamma->value);
	qglUniformMatrix4fv(gamma_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad ();

	GL_BindNullProgram ();
}


void R_MotionBlur (void) 
{

	vec2_t	angles, delta;

	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;

	// calc camera offsets
	angles[0] = r_newrefdef.viewanglesOld[1] - r_newrefdef.viewangles[1]; //YAW left-right
	angles[1] = r_newrefdef.viewanglesOld[0] - r_newrefdef.viewangles[0]; //PITCH up-down
	
	float blur = r_motionBlurFrameLerp->value;
	delta[0] = (angles[0] / r_newrefdef.fov_x) * blur;
	delta[1] = (angles[1] / r_newrefdef.fov_y) * blur;

	vec3_t velocity;
	VectorSet(velocity, delta[0], delta[1], 1.0);
	VectorNormalize(velocity);

	// setup program
	GL_BindProgram(motionBlurProgram, 0);

	qglUniform3f(mb_params, velocity[0], velocity[1], r_motionBlurSamples->value);
	qglUniformMatrix4fv(mb_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	GL_MBindRect(GL_TEXTURE0, ScreenMap->texnum);
	qglCopyTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width, vid.height);

	R_DrawFullScreenQuad();

	GL_BindNullProgram();
//	Com_Printf("X %f || Y %f\n", r_newrefdef.fov_x, r_newrefdef.fov_y);
}

void R_DownsampleDepth(void) 
{
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	if (r_newrefdef.rdflags & RDF_IRGOGGLES)
		return;

	if (!r_ssao->integer)
		return;

	GL_DepthRange(0.0, 1.0);
	// downsample the depth buffer
	qglBindFramebuffer(GL_FRAMEBUFFER, fboId);
	qglDrawBuffer(GL_COLOR_ATTACHMENT2);

	GL_BindProgram(depthDownsampleProgram, 0);
	GL_MBindRect(GL_TEXTURE0, depthMap->texnum);

	qglUniform2f(depthDS_params, r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);
	qglUniformMatrix4fv(depthDS_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawHalfScreenQuad();

	// restore settings
	qglBindFramebuffer(GL_FRAMEBUFFER, 0);
	GL_BindNullProgram();
}

void R_SSAO (void) 
{
	int i, j, numSamples;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	if (r_newrefdef.rdflags & RDF_IRGOGGLES)
		return;

	if (!r_ssao->integer)
		return;
	
	// process
	qglBindFramebuffer(GL_FRAMEBUFFER, fboId);
	qglDrawBuffer(GL_COLOR_ATTACHMENT0);

	GL_BindProgram (ssaoProgram, 0);
	GL_MBindRect(GL_TEXTURE0, fboDN->texnum);
	GL_MBind(GL_TEXTURE1, r_randomNormalTex->texnum);

	qglUniform2f (ssao_params, max(r_ssaoIntensity->value, 0.f), r_ssaoScale->value);
	qglUniform2f (ssao_vp, vid.width, vid.height);
	qglUniformMatrix4fv(ssao_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawHalfScreenQuad();

	// blur
	fboColorIndex = 0;

	if (r_ssaoBlur->integer) {
		qglBindFramebuffer(GL_FRAMEBUFFER, fboId);
		GL_MBindRect(GL_TEXTURE1, fboDN->texnum);

		GL_BindProgram(ssaoBlurProgram, 0);

		qglUniformMatrix4fv(ssaoB_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

		numSamples = (int)rintf(4.f * vid.height / 1080.f);
		qglUniform1i(ssaoB_sapmles, max(numSamples, 1));

		for (i = 0; i < r_ssaoBlur->integer; i++) {
#if 1
			// two-pass shader
			for (j = 0; j < 2; j++) {
				GL_MBindRect(GL_TEXTURE0, fboColor[j]->texnum);
				qglDrawBuffer(GL_COLOR_ATTACHMENT0 + (j ^ 1));
				qglUniform2f(ssaoB_axisMask, j ? 0.f : 1.f, j ? 1.f : 0.f);
				R_DrawHalfScreenQuad();
			}
#else
			// single-pass shader
			GL_MBindRect(GL_TEXTURE0, fboColor[fboColorIndex]);
			fboColorIndex ^= 1;
			qglDrawBuffer(GL_COLOR_ATTACHMENT0 + fboColorIndex);
			R_DrawHalfScreenQuad();
#endif
		}
	}

	// restore
	qglBindFramebuffer(GL_FRAMEBUFFER, 0);
	GL_BindNullProgram ();
}

/*
===========================================
 Based on Giliam de Carpentier work
 http://www.decarpentier.nl/lens-distortion
===========================================
*/

void R_FixFov(void) {

	vec4_t params;

	if (!r_fixFovStrength->value)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	// setup program
	GL_BindProgram(fixFovProgram, 0);

	if (!fovCorrTex) {
		qglGenTextures(1, &fovCorrTex);
		GL_MBind(GL_TEXTURE0, fovCorrTex);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, vid.width, vid.height, 0);
	}
	GL_MBind(GL_TEXTURE0, fovCorrTex);
	qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, vid.width, vid.height);

	params[0] = r_fixFovStrength->value;
	params[1] = tan(DEG2RAD(r_newrefdef.fov_x) / 2.0) / (vid.width / vid.height);
	params[2] = vid.width / vid.height;
	params[3] = r_fixFovDistroctionRatio->value;

	qglUniform4fv(fixfov_params, 1, params);
	qglUniformMatrix4fv(fixfov_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad();

	GL_BindNullProgram();

}

void R_MenuBackGround() {
	
	GL_Disable(GL_BLEND);
	GL_BindProgram(menuProgram, 0);
	GL_MBindRect(GL_TEXTURE0, ScreenMap->texnum);
	qglCopyTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, 0, 0, vid.width, vid.height);

	qglUniform2f(menu_params, vid.width, vid.height);
	qglUniformMatrix4fv(menu_orthoMatrix, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad();

	GL_BindNullProgram();
	GL_Enable(GL_BLEND);
}