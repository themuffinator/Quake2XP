/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
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

	GL_BindVAO(vao.fsq);
	GL_DrawElements(GL_TRIANGLES, QUAD_INDICES, GL_UNSIGNED_BYTE, NULL);
}

void R_RestoreViewPortAndScissor() {

	GL_Disable(GL_SCISSOR_TEST);
	GL_Scissor(r_newrefdef.viewport[0], r_newrefdef.viewport[1], r_newrefdef.viewport[2], r_newrefdef.viewport[3]);
	GL_Viewport(r_newrefdef.viewport[0], r_newrefdef.viewport[1], r_newrefdef.viewport[2], r_newrefdef.viewport[3]);
}

void R_SetViewPortAndScissor(int x, int y, int w, int h) {
	GL_Enable(GL_SCISSOR_TEST);
	GL_Scissor(x, y, w, h);
	GL_Viewport(x, y, w, h);
}

void R_Bloom (void) {
	
	if (!r_hdrBloom->integer)
		return;

	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;

	int w = vid.width	* 0.5;
	int h = vid.height	* 0.5;

	qglBindFramebuffer(GL_READ_FRAMEBUFFER, fb.hdrBase->id);
	qglBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb.bloomCompute->id);
	qglBlitFramebuffer(0, 0, vid.width, vid.height, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	qglBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb.hdrBase->id);
	R_SetViewPortAndScissor(0, 0, w, h);

	GL_BindProgram(brightProgram);
	qglUniform1i(U_PARAM_INT_0, 0);
	GL_SetBindlessTexture(U_TMU0, i_bloomIn->handle);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float *)r_newrefdef.orthoMatrix);
	R_DrawFullScreenQuad();
	glCopyTextureSubImage2D(i_bloomIn->texnum, 0, 0, 0, 0, 0, w, h);

	qglMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	int const group_size = 64;
	
	GL_BindProgram(blur_xComputeProgram);
	qglBindImageTexture(0, i_bloomIn->texnum,		0, GL_FALSE, 0, GL_READ_ONLY,	GL_RGBA16F);
	qglBindImageTexture(1, i_bloomInterim->texnum,	0, GL_FALSE, 0, GL_WRITE_ONLY,	GL_R11F_G11F_B10F);
	qglDispatchCompute((w + group_size - 1) / group_size, h, 1);

	qglMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	
	GL_BindProgram(blur_yComputeProgram);
	qglBindImageTexture(0, i_bloomInterim->texnum,	0, GL_FALSE, 0, GL_READ_ONLY,	GL_R11F_G11F_B10F);
	qglBindImageTexture(1, i_bloomOut->texnum,		0, GL_FALSE, 0, GL_WRITE_ONLY,	GL_R11F_G11F_B10F);
	qglDispatchCompute(w, (h + group_size - 1) / group_size, 1);

	qglMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
	
	R_RestoreViewPortAndScissor();

	//final pass
	GL_BindProgram (bloomFinalProgram);
	qglUniform1f(U_PARAM_FLOAT_0, r_hdrBloomIntens->value);
	GL_SetBindlessTexture(U_TMU0, i_hdrBaseInterim->handle);
	GL_SetBindlessTexture(U_TMU1, i_bloomOut->handle);

	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float *)r_newrefdef.orthoMatrix);
	R_DrawFullScreenQuad ();
	glCopyTextureSubImage2D(i_hdrBaseInterim->texnum, 0, 0, 0, 0, 0, vid.width, vid.height);
}

void R_ThermalVision (void) {
	int i;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	if (!(r_newrefdef.rdflags & RDF_IRGOGGLES))
		return;
	
	qglBindFramebuffer(GL_READ_FRAMEBUFFER, fb.hdrBase->id);
	qglBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb.thermal->id);

	qglBlitFramebuffer(0, 0, vid.width, vid.height, 0, 0, vid.width * 0.5, vid.height * 0.5, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	qglBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb.hdrBase->id);

	R_SetViewPortAndScissor(0, 0, vid.width * 0.5, vid.height * 0.5);

	// process colors
	GL_BindProgram (thermalProgram);
	GL_SetBindlessTexture(U_TMU0, i_thermal->handle);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float *)r_newrefdef.orthoMatrix);
	R_DrawFullScreenQuad();
	glCopyTextureSubImage2D(i_thermal->texnum, 0, 0, 0, 0, 0, vid.width * 0.5, vid.height * 0.5);

	//blur
	GL_BindProgram(bloomBlurProgram);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float *)r_newrefdef.orthoMatrix);
	GL_SetBindlessTexture(U_TMU0, i_thermal->handle);

	for (i = 0; i < 2; i++) {
		qglUniform1i(U_PARAM_INT_0, i);
		R_DrawFullScreenQuad();
		glCopyTextureSubImage2D(i_thermal->texnum, 0, 0, 0, 0, 0, vid.width * 0.5, vid.height * 0.5);
	}

	R_RestoreViewPortAndScissor();

	//final pass
	GL_BindProgram (thermalfpProgram);
	GL_SetBindlessTexture(U_TMU0, i_thermal->handle);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float *)r_newrefdef.orthoMatrix);
	R_DrawFullScreenQuad ();
	glCopyTextureSubImage2D(i_hdrBaseInterim->texnum, 0, 0, 0, 0, 0, vid.width, vid.height);
}



void R_RadialBlur (void) {
	float	blur;

	if (!r_radialBlur->integer)
		return;

	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;
	
	if (r_newrefdef.fov_x <= r_radialBlurFov->value)
		goto hack;

	if (r_newrefdef.rdflags & (RDF_UNDERWATER | RDF_PAIN)) {

	hack:	
		// setup program
		GL_BindProgram (radialProgram);

		if (r_newrefdef.rdflags & RDF_UNDERWATER)
			blur = 0.0065;
		else
			blur = 0.01;

		GL_SetBindlessTexture(U_TMU0, i_hdrBaseInterim->handle);
		// xy = radial center screen space position, z = radius attenuation, w = blur strength
		qglUniform4f(U_PARAM_VEC4_0, vid.width*0.5, vid.height*0.5, 1.0 / vid.height, blur);
		qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float *)r_newrefdef.orthoMatrix);
		R_DrawFullScreenQuad ();
		glCopyTextureSubImage2D(i_hdrBaseInterim->texnum, 0, 0, 0, 0, 0, vid.width, vid.height);
	}
}

extern float v_blend[4];

void R_ScreenBlend(void){

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
	
	if (!v_blend[3] || !r_screenBlend->integer)
		return;

	GL_BindProgram(screenFlashProgram);
	GL_SetBindlessTexture(U_TMU0, i_hdrBaseInterim->handle);
	qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);
	qglUniform1f(U_PARAM_FLOAT_0, r_screenBlendIntensity->value);
	qglUniform3f(U_PARAM_VEC3_0, v_blend[0], v_blend[1], v_blend[2]);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float*)r_newrefdef.orthoMatrix);
	R_DrawFullScreenQuad();
	glCopyTextureSubImage2D(i_hdrBaseInterim->texnum, 0, 0, 0, 0, 0, vid.width, vid.height);
}

void R_DofBlur (void) {
	float			tmpDist[5], tmpMins[3];
	vec2_t          dofParams;
	trace_t			trace;
	vec3_t			end_trace, v_f, v_r, v_up, tmp, left, right, up, dn;

	if (!r_dof->integer)
		return;

	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;

	//dof autofocus
	if (!r_dofFocus->integer) {

		AngleVectors (r_newrefdef.viewangles, v_f, v_r, v_up);
		VectorMA (r_newrefdef.vieworg, 4096.0, v_f, end_trace);

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

		if (trace.surface->flags & SURF_SKY) 
			dofParams[0] = 17000.0;
		else
			dofParams[0] = min (tmpMins[2], tmpDist[4]);
		dofParams[1] = r_dofBias->value;
	}
	else {
		dofParams[0] = r_dofFocus->value;
		dofParams[1] = r_dofBias->value;
	}
	// setup program
	GL_BindProgram (dofProgram);

	qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);
	qglUniform2f(U_PARAM_VEC2_0, dofParams[0], dofParams[1]);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float *)r_newrefdef.orthoMatrix);
	GL_SetBindlessTexture(U_TMU0, i_hdrBaseInterim->handle);
	GL_SetBindlessTexture(U_TMU1, i_linearDepth->handle);
	R_DrawFullScreenQuad ();
	glCopyTextureSubImage2D(i_hdrBaseInterim->texnum, 0, 0, 0, 0, 0, vid.width, vid.height);
}

void R_FilmFx(void) {

	if (!r_filmicFx->integer)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
	
	// hdr glares
	qglBindFramebuffer(GL_READ_FRAMEBUFFER, fb.hdrBase->id);
	qglBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb.glare->id);

	qglBlitFramebuffer(0, 0, vid.width, vid.height, 0, 0, vid.width * 0.25, vid.height * 0.25, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	qglBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb.hdrBase->id);

	R_SetViewPortAndScissor(0, 0, vid.width * 0.25, vid.height * 0.25);

	GL_BindProgram(brightProgram);
	qglUniform1i(U_PARAM_INT_0, 1);
	GL_SetBindlessTexture(U_TMU0, i_glare->handle);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float *)r_newrefdef.orthoMatrix);
	R_DrawFullScreenQuad();
	glCopyTextureSubImage2D(i_glare->texnum, 0, 0, 0, 0, 0, vid.width * 0.25, vid.height * 0.25);

	//glare
	GL_BindProgram(glareProgram);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float *)r_newrefdef.orthoMatrix);
	GL_SetBindlessTexture(U_TMU0, i_glare->handle);
	qglUniform1f(U_PARAM_FLOAT_0, r_hdrGlareIntens->value);

	for (int i = 0; i < r_hdrGlarePasses->integer; i++) {
		R_DrawFullScreenQuad();
		glCopyTextureSubImage2D(i_glare->texnum, 0, 0, 0, 0, 0, vid.width * 0.25, vid.height * 0.25);
	}

	R_RestoreViewPortAndScissor();

	//mix glare with screen pass
	GL_BindProgram(glareFinalProgram);
	qglUniform1f(U_PARAM_FLOAT_0, r_hdrGlareIntens->value);

	GL_SetBindlessTexture(U_TMU0, i_hdrBaseInterim->handle);
	GL_SetBindlessTexture(U_TMU1, i_glare->handle);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float *)r_newrefdef.orthoMatrix);
	R_DrawFullScreenQuad();
	glCopyTextureSubImage2D(i_hdrBaseInterim->texnum, 0, 0, 0, 0, 0, vid.width, vid.height);

	//-----------------
	// setup program
	GL_BindProgram (filmicFxProgram);
	qglBindFramebuffer(GL_READ_FRAMEBUFFER, fb.hdrBase->id);
	qglBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb.hdrBase2D->id);

	qglBlitFramebuffer(0, 0, vid.width, vid.height, 0, 0, vid.width, vid.height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	qglBindFramebuffer(GL_FRAMEBUFFER, fb.hdrBase->id);

	GL_SetBindlessTexture(U_TMU0,	i_hdrInterim2D->handle);
	GL_SetBindlessTexture(U_TMU1,	i_lensDirt->handle);

	qglUniform2f (U_SCREEN_SIZE,	vid.width, vid.height);
	qglUniform1f (U_PARAM_FLOAT_0,	r_filmicFxVignetSize->value);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float *)r_newrefdef.orthoMatrix);
	R_DrawFullScreenQuad ();
	glCopyTextureSubImage2D(i_hdrBaseInterim->texnum, 0, 0, 0, 0, 0, vid.width, vid.height);
}

void R_FXAA(void) {

	if (!r_fxaa->integer)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	// setup program
	GL_BindProgram(fxaaProgram);

	qglBindFramebuffer(GL_READ_FRAMEBUFFER, fb.hdrBase->id);
	qglBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb.hdrBase2D->id);

	qglBlitFramebuffer(0, 0, vid.width, vid.height, 0, 0, vid.width, vid.height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	qglBindFramebuffer(GL_FRAMEBUFFER, fb.hdrBase->id);

	GL_SetBindlessTexture(U_TMU0, i_hdrInterim2D->handle);
	qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float *)r_newrefdef.orthoMatrix);
	R_DrawFullScreenQuad();
	glCopyTextureSubImage2D(i_hdrBaseInterim->texnum, 0, 0, 0, 0, 0, vid.width, vid.height);
}


void R_ToneMaping(void) {

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	qglBindFramebuffer(GL_READ_FRAMEBUFFER, fb.hdrBase->id);
	qglBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb.hdrLum->id);
	qglBlitFramebuffer(0, 0, vid.width, vid.height, 0, 0, 128, 128, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glGenerateTextureMipmap(i_hdrLuminance->texnum);
	qglBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb.hdrBase->id);

	GL_BindProgram(tonemapProgram);

	GL_SetBindlessTexture(U_TMU0,	i_hdrBaseInterim->handle);
	GL_SetBindlessTexture(U_TMU1,	i_hdrLuminance->handle);

	qglUniform1f(U_PARAM_FLOAT_1,	r_gamma->value);
	qglUniform1f(U_PARAM_FLOAT_2,	r_hdrEVcomp->value);

	qglUniform1i(U_PARAM_INT_0,		r_hdrColorSpace->integer);
	qglUniform1i(U_PARAM_INT_1,		gl_config.useHdrDisplay);

	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float*)r_newrefdef.orthoMatrix);
	R_DrawFullScreenQuad();

	R_FXAA(); // apply fxaa AFTER tonemap!!!!

//===========================================================

	qglBindFramebuffer(GL_READ_FRAMEBUFFER, fb.hdrBase->id);
	qglBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb.ldrBase->id);
	qglBlitFramebuffer(0, 0, vid.width, vid.height, 0, 0, vid.width, vid.height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	qglBindFramebuffer(GL_FRAMEBUFFER, 0);

 // blit to screen
	GL_BindProgram(finalPassProgram);

	GL_SetBindlessTexture(U_TMU0, i_ldrBase->handle);

	qglUniform3f(U_COLOR_PARAMS, r_brightness->value, r_contrast->value, r_saturation->value);

	qglUniform3f(U_COLOR_VIBRANCE,	r_colorBalanceRed->value * r_colorVibrance->value,
									r_colorBalanceGreen->value * r_colorVibrance->value,
									r_colorBalanceBlue->value * r_colorVibrance->value);

	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float*)r_newrefdef.orthoMatrix);
	R_DrawFullScreenQuad();
	glCopyTextureSubImage2D(i_ldrBase->texnum, 0, 0, 0, 0, 0, vid.width, vid.height);
}

void R_ColorTemperatureCorrection(void) {

	if (r_colorTempK->value < 1000.0)
		return;
	
	if (!r_useColorCorrection->integer)
		return;

	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;

	GL_BindProgram(whiteBalanceProgram);
	qglUniform1f(U_PARAM_FLOAT_0, r_colorTempK->value);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float *)r_newrefdef.orthoMatrix);
	GL_SetBindlessTexture(U_TMU0, i_ldrBase->handle);
	R_DrawFullScreenQuad();
}

/*
void R_lutCorrection(void) {

	if (!lutCount)
		return;

	if (!r_useColorCorrection->integer)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	r_lutId->integer = ClampCvarInteger(0, lutCount - 1, r_lutId->integer);

	int lutID = r_lutId->integer;

	GL_BindProgram(lutProgram);

	R_CaptureColorBuffer();

	GL_SetBindlessTexture(U_TMU0, r_hdrScreenCopy->handle);
	GL_SetBindlessTexture(U_TMU1, r_3dLut[lutID]->handle);

	qglUniform3f(U_PARAM_VEC3_0, r_3dLut[lutID]->lutSize, r_3dLut[lutID]->lutSize, r_3dLut[lutID]->lutSize);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad();

}
*/

void R_MotionBlur(void) {
	vec2_t	angles, delta;
	vec3_t velocity;
	float blur;
	
	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;

	if (!r_motionBlur->integer)
		return;

	// go to 2d
	R_SetupOrthoMatrix();

	// calc camera offsets
	angles[0] = r_newrefdef.viewanglesOld[1] - r_newrefdef.viewangles[1]; //YAW left-right
	angles[1] = r_newrefdef.viewanglesOld[0] - r_newrefdef.viewangles[0]; //PITCH up-down

	blur = r_motionBlurFrameLerp->value;
	delta[0] = (angles[0] / r_newrefdef.fov_x) * blur;
	delta[1] = (angles[1] / r_newrefdef.fov_y) * blur;

	VectorSet(velocity, delta[0], delta[1], 1.0);
	VectorNormalize(velocity);

	// setup program
	GL_BindProgram(motionBlurProgram);

	qglUniform3f(U_PARAM_VEC3_0, velocity[0], velocity[1], r_motionBlurSamples->value);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float*)r_newrefdef.orthoMatrix);

	GL_SetBindlessTexture(U_TMU0, i_hdrBaseInterim->handle);
	R_DrawFullScreenQuad();
	glCopyTextureSubImage2D(i_hdrBaseInterim->texnum, 0, 0, 0, 0, 0, vid.width, vid.height);

	// restore 3d
	GL_Enable(GL_CULL_FACE);
	GL_Enable(GL_DEPTH_TEST);

	GL_Viewport(r_newrefdef.viewport[0], r_newrefdef.viewport[1], r_newrefdef.viewport[2], r_newrefdef.viewport[3]);
}


void R_SSAO (void) {
	int i, j, numSamples;

	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;

	if (!r_ssao->integer)
		return;
	
	R_SetupOrthoMatrix();
	
	// downsample the depth buffer
	GL_DepthRange(0.0, 1.0);
	qglBindFramebuffer(GL_FRAMEBUFFER, fb.ssao->id);
	qglDrawBuffer(GL_COLOR_ATTACHMENT2);

	R_SetViewPortAndScissor(0, 0, vid.width * 0.5, vid.height * 0.5);

	GL_BindProgram(depthDownsampleProgram);
	GL_SetBindlessTexture(U_TMU0, i_linearDepth->handle);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float*)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad();

	// process
	qglDrawBuffer(GL_COLOR_ATTACHMENT0);
	GL_BindProgram (ssaoProgram);
	GL_SetBindlessTexture(U_TMU0, i_ssaoDepth->handle);
	GL_SetBindlessTexture(U_TMU1, i_ssaoRandomNormal->handle);

	qglUniform2f (U_PARAM_VEC2_0, max(r_ssaoIntensity->value, 0.f), r_ssaoScale->value);
	qglUniform2f (U_SCREEN_SIZE, vid.width, vid.height);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad();

	// blur
	i_ssaoColorIndex = 0;

	GL_BindProgram(ssaoBlurProgram);
	GL_SetBindlessTexture(U_TMU1, i_ssaoDepth->handle);

	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float *)r_newrefdef.orthoMatrix);

	numSamples = (int)rintf(4.f * vid.height / 1080.f);
	qglUniform1i(U_PARAM_INT_0, max(numSamples, 1));

	for (i = 0; i < r_ssaoBlur->integer; i++) {
		// two-pass shader
		for (j = 0; j < 2; j++) {
			qglDrawBuffer(GL_COLOR_ATTACHMENT0 + (j ^ 1));
			GL_SetBindlessTexture(U_TMU0, i_ssaoColor[j]->handle);
			qglUniform2f(U_PARAM_VEC2_0, j ? 0.f : 1.f, j ? 1.f : 0.f);
			R_DrawFullScreenQuad();
		}
	}
	
	// restore
	qglBindFramebuffer(GL_FRAMEBUFFER, fb.hdrBase->id);
	qglDrawBuffer(GL_COLOR_ATTACHMENT0);

	GL_Enable(GL_CULL_FACE);
	GL_Enable(GL_DEPTH_TEST);

	R_RestoreViewPortAndScissor();
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
	GL_BindProgram(fixFovProgram);

	qglBindFramebuffer(GL_READ_FRAMEBUFFER, fb.hdrBase->id);
	qglBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb.hdrBase2D->id);

	qglBlitFramebuffer(0, 0, vid.width, vid.height, 0, 0, vid.width, vid.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	qglBindFramebuffer(GL_FRAMEBUFFER, fb.hdrBase->id);

	params[0] = r_fixFovStrength->value;
	params[1] = tan(DEG2RAD(r_newrefdef.fov_x) / 2.0) / (vid.width / vid.height);
	params[2] = vid.width / vid.height;
	params[3] = r_fixFovDistroctionRatio->value;

	qglUniform4fv(U_PARAM_VEC4_0, 1, params);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float *)r_newrefdef.orthoMatrix);
	GL_SetBindlessTexture(U_TMU0, i_hdrInterim2D->handle);
	R_DrawFullScreenQuad();
	glCopyTextureSubImage2D(i_hdrBaseInterim->texnum, 0, 0, 0, 0, 0, vid.width, vid.height);
}

void R_MenuBackGround() {

	GL_Disable(GL_BLEND);
	glCopyTextureSubImage2D(i_hdrBaseInterim->texnum, 0, 0, 0, 0, 0, vid.width, vid.height);
	GL_BindProgram(menuProgram);
	GL_SetBindlessTexture(U_TMU0, i_hdrBaseInterim->handle);
	qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float *)r_newrefdef.orthoMatrix);
	R_DrawFullScreenQuad();
	GL_Enable(GL_BLEND);
}

/*===============================
Global Fog Post-processing Effect
With Ingame Editor
===============================*/

void R_SaveFogParams() {

	char	name[MAX_QPATH], path[MAX_QPATH];
	FILE	*f;

	FS_StripExtension(r_worldmodel->name, name, sizeof(name));
	Com_sprintf(path, sizeof(path), "%s/%s.fog", FS_Gamedir(), name);

	f = fopen(path, "w");
	if (!f) {
		Com_Printf("Could not open %s.\n", path);
		return;
	}

	clamp(fog.type, 0, 1);

	fprintf(f, "//Fog Script for %s\n//Generated by quake2xp\n\n", r_worldmodel->name);
	fprintf(f, "type %i\n",					fog.type);
	fprintf(f, "color %.3f %.3f %.3f\n",	fog.color[0], fog.color[1], fog.color[2]);
	fprintf(f, "density %.5f\n",			fog.density);
	fprintf(f, "bias %.5f\n",				fog.bias);
	fclose(f);

	Com_Printf(""S_COLOR_MAGENTA"R_SaveFogParams: "S_COLOR_WHITE"Save Fog Script To "S_COLOR_GREEN"%s.fog\n", name);
}

void R_RemoveFogParams() {

	char	name[MAX_QPATH], path[MAX_QPATH];

	FS_StripExtension(r_worldmodel->name, name, sizeof(name));
	Com_sprintf(path, sizeof(path), "%s/%s.fog", FS_Gamedir(), name);
	remove(path); //remove it!
	Com_Printf(""S_COLOR_MAGENTA"R_RemoveFogParams: "S_COLOR_WHITE"Remove Fog Script To "S_COLOR_GREEN"%s.fog\n", name);

	fog.type = 0;
	VectorSet(fog.color, 1.0, 0.3, 0.1);
	fog.density = 0.025;
	fog.bias = 0.0;
}

void R_FogEditor_f(void) {
	
	if (!r_fogEditor->integer || !r_globalFog->integer) {
		Com_Printf(S_COLOR_MAGENTA"You mast set 'r_globalFog 1' and 'r_fogEditor 1' to enable Fog Editor.\n");
		return;
	}

	if ( (!strcmp(Cmd_Argv(1), "?")) || (!strcmp(Cmd_Argv(1), "help")) || (!strcmp(Cmd_Argv(1), "")) )
	{
		Com_Printf("Fog Editor commands:\n\n");
		Com_Printf("<?> or <help> or <> " S_COLOR_YELLOW "this list.\n");
		Com_Printf("<type> " S_COLOR_YELLOW "fog mode: " S_COLOR_GREEN "0 - exp, 1 - exp2\n");
		Com_Printf("<color> " S_COLOR_YELLOW "fog color: " S_COLOR_GREEN "R G B\n");
		Com_Printf("<density> " S_COLOR_YELLOW "fog density: " S_COLOR_GREEN "value\n");
		Com_Printf("<bias> " S_COLOR_YELLOW "fog bias: " S_COLOR_GREEN "value\n");
		Com_Printf("<save> " S_COLOR_YELLOW "save fog params to mapname.fog script.\n");
		Com_Printf("<remove> " S_COLOR_YELLOW "remove mapname.fog script and set default params.\n");
		return;
	}

	if (!strcmp(Cmd_Argv(1), "save")) {
		R_SaveFogParams();
		return;
	}

	if (!strcmp(Cmd_Argv(1), "remove")) {
		R_RemoveFogParams();
		return;
	}

	if (!strcmp(Cmd_Argv(1), "type")) {
		if (Cmd_Argc() != 3) {
			Com_Printf("usage: fogEdit %s integer (0 - exp, 1 - exp2)\nCurrent fog mode: " S_COLOR_YELLOW "%i\n", Cmd_Argv(0), fog.type);
			return;
		}
		fog.type = atoi(Cmd_Argv(2));
	}
	
	if (!strcmp(Cmd_Argv(1), "color")) {
		if (Cmd_Argc() != 5) {
			Com_Printf("usage: fogEdit %s R G B\nCurrent fog color: " S_COLOR_YELLOW "%.3f %.3f %.3f\n", 
				Cmd_Argv(0),
				fog.color[0],
				fog.color[1],
				fog.color[2]);
		return;
		}
		fog.color[0] = atof(Cmd_Argv(2));
		fog.color[1] = atof(Cmd_Argv(3));
		fog.color[2] = atof(Cmd_Argv(4));
	} 

	if (!strcmp(Cmd_Argv(1), "density")) {
		if (Cmd_Argc() != 3) {
			Com_Printf("usage: fogEdit %s value\nCurrent fog density: " S_COLOR_YELLOW "%.5f\n", Cmd_Argv(0), fog.density);
			return;
		}
		fog.density = atof(Cmd_Argv(2));
	} 

	if (!strcmp(Cmd_Argv(1), "bias")) {
		if (Cmd_Argc() != 3) {
			Com_Printf("usage: fogEdit %s value\nCurrent fog bias: " S_COLOR_YELLOW "%.5f\n", Cmd_Argv(0), fog.bias);
			return;
		}
		fog.bias = atof(Cmd_Argv(2));
	}
}

void R_GlobalFog() {

	if (!r_globalFog->integer)
		return;

	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;

	if (!r_worldmodel)
		return;

	R_SetupOrthoMatrix();

	GL_BindProgram(globalFogProgram);

	GL_SetBindlessTexture(U_TMU0, i_hdrBaseInterim->handle);
	GL_SetBindlessTexture(U_TMU1, i_linearDepth->handle);

	qglUniform1i(U_PARAM_INT_0,		fog.type);
	qglUniform4f(U_PARAM_VEC4_0,	fog.color[0], fog.color[1], fog.color[2], fog.density);
	qglUniform1f(U_PARAM_FLOAT_0,	fog.bias);

	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float*)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad();
	glCopyTextureSubImage2D(i_hdrBaseInterim->texnum, 0, 0, 0, 0, 0, vid.width, vid.height);

	// restore 3d
	GL_Enable(GL_CULL_FACE);
	GL_Enable(GL_DEPTH_TEST);

	GL_Viewport(r_newrefdef.viewport[0], r_newrefdef.viewport[1],
				r_newrefdef.viewport[2], r_newrefdef.viewport[3]);
}
