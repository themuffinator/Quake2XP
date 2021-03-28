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

	glBindVertexArray(vao.fullscreenQuad);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

	glBindVertexArray(0);
}

void R_DrawHalfScreenQuad () {

	glBindVertexArray(vao.halfScreenQuad);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

	glBindVertexArray(0);
}

void R_DrawQuarterScreenQuad () {
	
	glBindVertexArray(vao.quaterScreenQuad);

	qglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
	
	glBindVertexArray(0);
}

void R_Bloom (void) 
{
	if (!r_bloom->integer)
		return;

	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;

	// downsample and cut color
	glCopyTextureSubImage2D(r_screenTex->texnum, 0, 0, 0, 0, 0, vid.width, vid.height);

	// setup program
	GL_BindProgram (bloomdsProgram);
	GL_SetBindlessTexture(U_TMU0, r_screenTex->handle);
	qglUniform1f(U_PARAM_FLOAT_0, r_bloomThreshold->value);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawQuarterScreenQuad ();

	// generate star shape
	glCopyTextureSubImage2D(r_bloomImage->texnum, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

	GL_BindProgram (glareProgram);
	GL_SetBindlessTexture(U_TMU0, r_bloomImage->handle);
	qglUniform1f(U_PARAM_FLOAT_0, r_bloomWidth->value);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);
	R_DrawQuarterScreenQuad ();

	// blur x
	glCopyTextureSubImage2D(r_bloomImage->texnum, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

	GL_BindProgram (gaussXProgram);
	GL_SetBindlessTexture(U_TMU0, r_bloomImage->handle);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);
	R_DrawQuarterScreenQuad ();

	// blur y
	glCopyTextureSubImage2D(r_bloomImage->texnum, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

	GL_BindProgram (gaussYProgram);
	GL_SetBindlessTexture(U_TMU0, r_bloomImage->handle);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);
	R_DrawQuarterScreenQuad ();

	// store 2 pass gauss blur 
	glCopyTextureSubImage2D(r_bloomImage->texnum, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

	//final pass
	GL_BindProgram (bloomfpProgram);
	GL_SetBindlessTexture(U_TMU0, r_screenTex->handle);
	GL_SetBindlessTexture(U_TMU1, r_bloomImage->handle);
	qglUniform1f(U_PARAM_FLOAT_0, r_bloomIntens->value);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);
	R_DrawFullScreenQuad ();
}



void R_ThermalVision (void) 
{
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	if (!(r_newrefdef.rdflags & RDF_IRGOGGLES))
		return;
	
	// grab screen
	glCopyTextureSubImage2D(r_screenTex->texnum, 0, 0, 0, 0, 0, vid.width, vid.height);

	// setup program
	GL_BindProgram (thermalProgram);
	GL_SetBindlessTexture(U_TMU0, r_screenTex->handle);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);
	R_DrawHalfScreenQuad ();

	// blur x
	glCopyTextureSubImage2D(r_thermalImage->texnum, 0, 0, 0, 0, 0, vid.width*0.5, vid.height*0.5);

	GL_BindProgram (gaussXProgram);
	GL_SetBindlessTexture(U_TMU0, r_thermalImage->handle);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);
	R_DrawHalfScreenQuad ();

	// blur y
	glCopyTextureSubImage2D(r_thermalImage->texnum, 0, 0, 0, 0, 0, vid.width * 0.5, vid.height * 0.5);

	GL_BindProgram (gaussYProgram);
	GL_SetBindlessTexture(U_TMU0, r_thermalImage->handle);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);
	R_DrawHalfScreenQuad ();

	// store 2 pass gauss blur 
	glCopyTextureSubImage2D(r_thermalImage->texnum, 0, 0, 0, 0, 0, vid.width * 0.5, vid.height * 0.5);
	//final pass
	GL_BindProgram (thermalfpProgram);
	GL_SetBindlessTexture(U_TMU0, r_thermalImage->handle);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);
	R_DrawFullScreenQuad ();
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

	
		R_CaptureColorBuffer();

		// setup program
		GL_BindProgram (radialProgram);

		if (r_newrefdef.rdflags & RDF_UNDERWATER)
			blur = 0.0065;
		else
			blur = 0.01;

		GL_SetBindlessTexture(U_TMU0, r_screenTex->handle);
		// xy = radial center screen space position, z = radius attenuation, w = blur strength
		qglUniform4f(U_PARAM_VEC4_0, vid.width*0.5, vid.height*0.5, 1.0 / vid.height, blur);
		qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

		R_DrawFullScreenQuad ();
	}
}

extern float v_blend[4];

void R_ScreenBlend(void)
{

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
	
	if (!v_blend[3] || !r_screenBlend->integer)
		return;

	glCopyTextureSubImage2D(r_screenTex->texnum, 0, 0, 0, 0, 0, vid.width, vid.height);

	GL_BindProgram(screenFlashProgram);
	GL_SetBindlessTexture(U_TMU0, r_screenTex->handle);
	qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);
	qglUniform3f(U_PARAM_VEC3_0, v_blend[0], v_blend[1], v_blend[2]);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float*)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad();
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
	if (!r_dofFocus->integer) {

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

	R_CaptureColorBuffer();
	// setup program
	GL_BindProgram (dofProgram);

	qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);
	qglUniform4f(U_PARAM_VEC4_0, dofParams[0], dofParams[1], r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	GL_SetBindlessTexture(U_TMU0, r_screenTex->handle);
	GL_SetBindlessTexture(U_TMU1, r_depthTex->handle);

	R_DrawFullScreenQuad ();
}

void R_FXAA (void) {

	if (!r_fxaa->integer)
		return;
	
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	// setup program
	GL_BindProgram (fxaaProgram);

	glCopyTextureSubImage2D(r_fxaaTex->texnum, 0, 0, 0, 0, 0, vid.width, vid.height);

	GL_SetBindlessTexture(U_TMU0, r_fxaaTex->handle);
	qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad ();
}

void R_FilmFilter (void) 
{

	if (!r_filmFilter->integer)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
	 
	// setup program
	GL_BindProgram (filmGrainProgram);

	R_CaptureColorBuffer();
	GL_SetBindlessTexture(U_TMU0, r_screenTex->handle);

	qglUniform2f (U_SCREEN_SIZE,	vid.width, vid.height);
	qglUniform1f (U_PARAM_FLOAT_0,	crand());
	qglUniform1i (U_PARAM_INT_0,	r_framecount);
	qglUniform3f (U_PARAM_VEC3_0,	r_filmFilterNoiseIntens->value, r_filmFilterScratchIntens->value, r_filmFilterVignetIntens->value);

	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad ();
}

void R_GammaRamp (void){

	if (!r_useColorCorrection->integer)
		return;

	GL_BindProgram (gammaProgram);

	glCopyTextureSubImage2D(r_screenTex->texnum, 0, 0, 0, 0, 0, vid.width, vid.height);

	GL_SetBindlessTexture(U_TMU0, r_screenTex->handle);

	qglUniform4f (U_COLOR_PARAMS,	r_brightness->value, 
									r_contrast->value, 
									r_saturation->value, 
									1.0 / r_gamma->value);

	qglUniform3f (U_COLOR_VIBRANCE, r_colorBalanceRed->value	* r_colorVibrance->value,
									r_colorBalanceGreen->value	* r_colorVibrance->value, 
									r_colorBalanceBlue->value	* r_colorVibrance->value);

	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);
	R_DrawFullScreenQuad ();
}

void R_ColorTemperatureCorrection(void){

	if (r_colorTempK->value < 1000.0)
		return;
	
	if (!r_useColorCorrection->integer)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	GL_BindProgram(whiteBalanceProgram);

	R_CaptureColorBuffer();

	qglUniform1f(U_PARAM_FLOAT_0, r_colorTempK->value);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);
	GL_SetBindlessTexture(U_TMU0, r_screenTex->handle);
	R_DrawFullScreenQuad();

}

void R_lutCorrection(void)
{
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

	GL_SetBindlessTexture(U_TMU0, r_screenTex->handle);
	GL_SetBindlessTexture(U_TMU1, r_3dLut[lutID]->handle);

	qglUniform3f(U_PARAM_VEC3_0, r_3dLut[lutID]->lutSize, r_3dLut[lutID]->lutSize, r_3dLut[lutID]->lutSize);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad();

}

void R_MotionBlur(void)
{
	vec2_t	angles, delta;
	vec3_t velocity;
	float blur;

	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;

	if (!r_motionBlur->integer)
		return;

	// go to 2d
	R_SetupOrthoMatrix();
//	GL_DepthMask(0);

	// calc camera offsets
	angles[0] = r_newrefdef.viewanglesOld[1] - r_newrefdef.viewangles[1]; //YAW left-right
	angles[1] = r_newrefdef.viewanglesOld[0] - r_newrefdef.viewangles[0]; //PITCH up-down

	blur = r_motionBlurFrameLerp->value;
	delta[0] = (angles[0] / r_newrefdef.fov_x) * blur;
	delta[1] = (angles[1] / r_newrefdef.fov_y) * blur;

	VectorSet(velocity, delta[0], delta[1], 1.0);
	VectorNormalize(velocity);

	glCopyTextureSubImage2D(r_screenTex->texnum, 0, 0, 0, 0, 0, vid.width, vid.height);

	// setup program
	GL_BindProgram(motionBlurProgram);

	qglUniform3f(U_PARAM_VEC3_0, velocity[0], velocity[1], r_motionBlurSamples->value);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float*)r_newrefdef.orthoMatrix);

	GL_SetBindlessTexture(U_TMU0, r_screenTex->handle);
	R_DrawFullScreenQuad();

	// restore 3d
	GL_Enable(GL_CULL_FACE);
	GL_Enable(GL_DEPTH_TEST);
//	GL_DepthMask(1);
	qglViewport(r_newrefdef.viewport[0], r_newrefdef.viewport[1],
		r_newrefdef.viewport[2], r_newrefdef.viewport[3]);
}

void R_DownsampleDepth(void) 
{
	if (!r_ssao->integer)
		return;

	GL_DepthRange(0.0, 1.0);
	// downsample the depth buffer
	qglBindFramebuffer(GL_FRAMEBUFFER, fboId);
	qglDrawBuffer(GL_COLOR_ATTACHMENT2);

	GL_BindProgram(depthDownsampleProgram);
	GL_SetBindlessTexture(U_TMU0, r_depthTex->handle);

	qglUniform2f(U_DEPTH_PARAMS, r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawHalfScreenQuad();

	// restore settings
	qglBindFramebuffer(GL_FRAMEBUFFER, 0);
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
	
	R_SetupOrthoMatrix();
	R_DownsampleDepth();
	
//	GL_DepthMask(0);

	// process
	qglBindFramebuffer(GL_FRAMEBUFFER, fboId);
	qglDrawBuffer(GL_COLOR_ATTACHMENT0);

	GL_BindProgram (ssaoProgram);
	GL_SetBindlessTexture(U_TMU0, r_miniDepthTex->handle);
	GL_SetBindlessTexture(U_TMU1, r_randomNormalTex->handle);

	qglUniform2f (U_PARAM_VEC2_0, max(r_ssaoIntensity->value, 0.f), r_ssaoScale->value);
	qglUniform2f (U_SCREEN_SIZE, vid.width, vid.height);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

	R_DrawHalfScreenQuad();

	// blur
	r_ssaoColorTexIndex = 0;

	if (r_ssaoBlur->integer) {
		qglBindFramebuffer(GL_FRAMEBUFFER, fboId);
		GL_SetBindlessTexture(U_TMU1, r_miniDepthTex->handle);

		GL_BindProgram(ssaoBlurProgram);

		qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

		numSamples = (int)rintf(4.f * vid.height / 1080.f);
		qglUniform1i(U_PARAM_INT_0, max(numSamples, 1));

		for (i = 0; i < r_ssaoBlur->integer; i++) {
			// two-pass shader
			for (j = 0; j < 2; j++) {
				GL_SetBindlessTexture(U_TMU0, r_ssaoColorTex[j]->handle);
				qglDrawBuffer(GL_COLOR_ATTACHMENT0 + (j ^ 1));
				qglUniform2f(U_PARAM_VEC2_0, j ? 0.f : 1.f, j ? 1.f : 0.f);
				R_DrawHalfScreenQuad();
			}
		}
	}

	// restore
	qglBindFramebuffer(GL_FRAMEBUFFER, 0);

	GL_Enable(GL_CULL_FACE);
	GL_Enable(GL_DEPTH_TEST);
//	GL_DepthMask(1);
	qglViewport(r_newrefdef.viewport[0], r_newrefdef.viewport[1],
				r_newrefdef.viewport[2], r_newrefdef.viewport[3]);
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

	glCopyTextureSubImage2D(r_fixFovTex->texnum, 0, 0, 0, 0, 0, vid.width, vid.height);

	params[0] = r_fixFovStrength->value;
	params[1] = tan(DEG2RAD(r_newrefdef.fov_x) / 2.0) / (vid.width / vid.height);
	params[2] = vid.width / vid.height;
	params[3] = r_fixFovDistroctionRatio->value;

	qglUniform4fv(U_PARAM_VEC4_0, 1, params);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);
	GL_SetBindlessTexture(U_TMU0, r_fixFovTex->handle);
	R_DrawFullScreenQuad();
}

void R_MenuBackGround() {
	
	GL_Disable(GL_BLEND);

	glCopyTextureSubImage2D(r_screenTex->texnum, 0, 0, 0, 0, 0, vid.width, vid.height);

	GL_BindProgram(menuProgram);
	GL_SetBindlessTexture(U_TMU0, r_screenTex->handle);
	qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float *)r_newrefdef.orthoMatrix);

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
	fprintf(f, "type %i\n",						fog.type);
	fprintf(f, "worldColor %.3f %.3f %.3f\n",	fog.worldColor[0], fog.worldColor[1], fog.worldColor[2]);
	fprintf(f, "skyColor %.3f %.3f %.3f\n",		fog.skyColor[0], fog.skyColor[1], fog.skyColor[2]);
	fprintf(f, "worldDensity %.5f\n",			fog.worldDensity);
	fprintf(f, "skyDensity %.5f\n",				fog.skyDensity);
	fprintf(f, "worldBias %.3f\n",				fog.worldBias);
	fprintf(f, "skyBias %.3f\n",				fog.skyBias);
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
	VectorSet(fog.worldColor, 1.0, 1.0, 0.5);
	VectorSet(fog.skyColor, 1.0, 0.5, 0.3);
	fog.worldDensity = 0.02500;
	fog.skyDensity = 0.005;
	fog.worldBias = 0.0;
	fog.skyBias = 0.0;
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
		Com_Printf("<worldColor> " S_COLOR_YELLOW "world fog color: " S_COLOR_GREEN "R G B\n");
		Com_Printf("<skyColor> " S_COLOR_YELLOW "sky fog color: " S_COLOR_GREEN "R G B\n");
		Com_Printf("<worldDensity> " S_COLOR_YELLOW "world fog density: " S_COLOR_GREEN "value\n");
		Com_Printf("<skyDensity> " S_COLOR_YELLOW "sky fog density: " S_COLOR_GREEN "value\n");
		Com_Printf("<worldBias> " S_COLOR_YELLOW "world fog bias: " S_COLOR_GREEN "value\n");
		Com_Printf("<skyBias> " S_COLOR_YELLOW "sky fog bias: " S_COLOR_GREEN "value\n");
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
	
	if (!strcmp(Cmd_Argv(1), "worldColor")) {
		if (Cmd_Argc() != 5) {
			Com_Printf("usage: fogEdit %s R G B\nCurrent world fog color: " S_COLOR_YELLOW "%.3f %.3f %.3f\n", 
				Cmd_Argv(0),
				fog.worldColor[0],
				fog.worldColor[1],
				fog.worldColor[2]);
		return;
		}
		fog.worldColor[0] = atof(Cmd_Argv(2));
		fog.worldColor[1] = atof(Cmd_Argv(3));
		fog.worldColor[2] = atof(Cmd_Argv(4));
	} 
		
	if (!strcmp(Cmd_Argv(1), "skyColor")) {
		if (Cmd_Argc() != 5) {
			Com_Printf("usage: fogEdit %s R G B\nCurrent sky fog color: " S_COLOR_YELLOW "%.3f %.3f %.3f\n",
					Cmd_Argv(0),
					fog.skyColor[0],
					fog.skyColor[1],
					fog.skyColor[2]);
			return;
			}
			fog.skyColor[0] = atof(Cmd_Argv(2));
			fog.skyColor[1] = atof(Cmd_Argv(3));
			fog.skyColor[2] = atof(Cmd_Argv(4));
		} 

		if (!strcmp(Cmd_Argv(1), "worldDensity")) {
			if (Cmd_Argc() != 3) {
				Com_Printf("usage: fogEdit %s value\nCurrent world fog density: " S_COLOR_YELLOW "%.3f\n", Cmd_Argv(0), fog.worldDensity);
				return;
			}
				fog.worldDensity = atof(Cmd_Argv(2));
			} 

		if (!strcmp(Cmd_Argv(1), "skyDensity")) {
			if (Cmd_Argc() != 3) {
				Com_Printf("usage: fogEdit %s value\nCurrent sky fog density: " S_COLOR_YELLOW "%.3f\n", Cmd_Argv(0), fog.skyDensity);
				return;
			}
			fog.skyDensity = atof(Cmd_Argv(2));
		}
		
		if (!strcmp(Cmd_Argv(1), "worldBias")) {
			if (Cmd_Argc() != 3) {
				Com_Printf("usage: fogEdit %s value\nCurrent world fog bias: " S_COLOR_YELLOW "%.3f\n", Cmd_Argv(0), fog.worldBias);
				return;
			}
			fog.worldBias = atof(Cmd_Argv(2));
		}
		
		if (!strcmp(Cmd_Argv(1), "skyBias")) {
			if (Cmd_Argc() != 3) {
				Com_Printf("usage: fogEdit %s value\nCurrent sky fog bias: " S_COLOR_YELLOW "%.3f\n", Cmd_Argv(0), fog.skyBias);
				return;
			}
			fog.skyBias = atof(Cmd_Argv(2));
		}
}

void R_GlobalFog() {

	if (!r_globalFog->integer)
		return;

	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL | RDF_IRGOGGLES))
		return;

	if (!r_worldmodel)
		return;

//	GL_DepthMask(0);
	R_SetupOrthoMatrix();

	GL_BindProgram(globalFogProgram);

	GL_SetBindlessTexture(U_TMU0, r_screenTex->handle);
	GL_SetBindlessTexture(U_TMU1, r_depthTex->handle);

	qglUniform2f(U_DEPTH_PARAMS, r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);

	qglUniform1i(U_PARAM_INT_0, fog.type);
	qglUniform4f(U_PARAM_VEC4_0, fog.worldColor[0], fog.worldColor[1], fog.worldColor[2], fog.worldDensity);
	qglUniform4f(U_PARAM_VEC4_1, fog.skyColor[0], fog.skyColor[1], fog.skyColor[2], fog.skyDensity);
	qglUniform2f(U_PARAM_VEC2_0, fog.worldBias, fog.skyBias);

	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, qfalse, (const float*)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad();

	// restore 3d
	GL_Enable(GL_CULL_FACE);
	GL_Enable(GL_DEPTH_TEST);

//	GL_DepthMask(1);
	qglViewport(r_newrefdef.viewport[0], r_newrefdef.viewport[1],
				r_newrefdef.viewport[2], r_newrefdef.viewport[3]);
}
