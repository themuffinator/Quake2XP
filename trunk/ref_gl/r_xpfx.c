/*
Copyright (C) 1997-2001 Quake2xp Team.

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


cvar_t	*r_anisotropic;
cvar_t	*r_maxAnisotropy;

cvar_t	*r_causticIntens;

cvar_t	*r_displayRefresh;

cvar_t	*r_screenShot;
cvar_t	*r_screenShotJpegQuality;

cvar_t	*r_overBrightBits;
cvar_t	*r_textureCompression;

cvar_t	*r_shadows;
cvar_t	*r_shadowWorldLightScale;
cvar_t	*r_shadowVolumesDebug;
cvar_t	*r_playerShadow;
cvar_t	*r_shadowCapOffset;

cvar_t	*r_radarSize;			// GLOOM radar
cvar_t	*r_radarZoom;
cvar_t	*r_radar;

cvar_t	*r_arbSamples;
cvar_t	*r_nvSamplesCoverange;
cvar_t  *r_nvMultisampleFilterHint;

cvar_t	*deathmatch;

cvar_t	*r_drawFlares;
cvar_t	*r_flaresIntens;
cvar_t	*r_flareWeldThreshold;

cvar_t	*r_customWidth;
cvar_t	*r_customHeight;

cvar_t	*r_bloom;
cvar_t	*r_bloomThreshold;
cvar_t	*r_bloomIntens;

cvar_t	*sys_priority;
cvar_t	*sys_affinity;

cvar_t	*r_DrawRangeElements;
cvar_t	*r_pplWorldAmbient;
cvar_t	*r_bumpAlias;
cvar_t	*r_bumpWorld;
cvar_t	*r_ambientLevel;
cvar_t	*r_pplMaxDlights;

cvar_t	*hunk_bsp;
cvar_t	*hunk_model;
cvar_t	*hunk_sprite;

//cvar_t	*r_vbo;
cvar_t	*r_maxTextureSize;

cvar_t	*r_parallax;
cvar_t	*r_parallaxScale;
cvar_t	*r_parallaxSteps;

cvar_t	*r_dof;
cvar_t	*r_dofBias;
cvar_t	*r_dofFocus;

cvar_t	*r_radialBlur;
cvar_t	*r_radialBlurFov;
cvar_t	*r_radialBlurSamples;
cvar_t	*r_tbnSmoothAngle;

cvar_t	*r_softParticles;
cvar_t	*r_ignoreGlErrors;


//--------------------

void GL_Overbrights(qboolean enable)
{
	
	if (enable) {				// turn on
		GL_TexEnv(GL_COMBINE_ARB);

		qglTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
		qglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
		qglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PREVIOUS_ARB);
		qglTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, (int) r_overBrightBits->value);

		qglTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_MODULATE);
		qglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
		qglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, GL_PREVIOUS_ARB);
		qglTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);
	} else {					// turn off
		GL_TexEnv(GL_MODULATE);

		qglTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
		qglTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);
	}
}


void GL_BindRect(int texnum);

void R_CaptureColorBuffer()
{
		
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
		
	qglEnable(GL_TEXTURE_RECTANGLE_ARB);
	GL_MBindRect(GL_TEXTURE0_ARB, ScreenMap->texnum);
	qglCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
	qglDisable(GL_TEXTURE_RECTANGLE_ARB);
}

void R_CaptureDepthBuffer()
{
		
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
		
	qglEnable(GL_TEXTURE_RECTANGLE_ARB);
	GL_MBindRect(GL_TEXTURE0_ARB, depthMap->texnum);
	qglCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
	qglDisable(GL_TEXTURE_RECTANGLE_ARB);

}

/*
==========================
Vic's occulusions
==========================
*/
void Matrix4_Multiply_Vector(const mat4x4_t m, const vec4_t v, vec4_t out)
{
	out[0] = m[0] * v[0] + m[4] * v[1] + m[8] * v[2] + m[12] * v[3];
	out[1] = m[1] * v[0] + m[5] * v[1] + m[9] * v[2] + m[13] * v[3];
	out[2] = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14] * v[3];
	out[3] = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15] * v[3];
}

/*
=============
R_TransformToScreen_Vec3
=============
*/
void R_TransformToScreen_Vec3(vec3_t in, vec3_t out)
{
	vec4_t temp, temp2;

	temp[0] = in[0];
	temp[1] = in[1];
	temp[2] = in[2];
	temp[3] = 1.0f;
	Matrix4_Multiply_Vector(r_world_matrix, temp, temp2);
	Matrix4_Multiply_Vector(r_project_matrix, temp2, temp);

	if (!temp[3])
		return;
	out[0] =
		r_newrefdef.x + (temp[0] / temp[3] +
						 1.0f) * r_newrefdef.width * 0.5f;
	out[1] =
		r_newrefdef.y + (temp[1] / temp[3] +
						 1.0f) * r_newrefdef.height * 0.5f;
	out[2] = (temp[2] / temp[3] + 1.0f) * 0.5f;
}

void Matrix_TransformVector( vec3_t m[3], vec3_t v, vec3_t out )
{
	out[0] = m[0][0]*v[0] + m[0][1]*v[1] + m[0][2]*v[2];
	out[1] = m[1][0]*v[0] + m[1][1]*v[1] + m[1][2]*v[2];
	out[2] = m[2][0]*v[0] + m[2][1]*v[1] + m[2][2]*v[2];
}

/*
===========
Q_sincos

===========
*/

#define Q_INLINE		__forceinline

Q_INLINE void Q_sincos(float a, float *s, float *c) {
#if defined _WIN32 && defined ASM_X86
	__asm {
		fld	a
		mov	ecx, s
		mov	edx, c
		fsincos
		fstp	dword ptr [edx]
		fstp	dword ptr [ecx]
	}
#else
	*s = (float)sin(a);
	*c = (float)cos(a);
#endif
}

/*
=================
AnglesToMat3

=================
*/
void AnglesToMat3(const vec3_t angles, mat3_t m) {
     float     sp, sy, sr, cp, cy, cr;

     Q_sincos(DEG2RAD(angles[PITCH]), &sp, &cp);
     Q_sincos(DEG2RAD(angles[YAW]), &sy, &cy);
     Q_sincos(DEG2RAD(angles[ROLL]), &sr, &cr);

     m[0][0] = cp * cy;
     m[0][1] = cp * sy;
     m[0][2] = -sp;
     m[1][0] = sr * sp * cy - cr * sy;
     m[1][1] = sr * sp * sy + cr * cy;
     m[1][2] = sr * cp;
     m[2][0] = cr * sp * cy + sr * sy;
     m[2][1] = cr * sp * sy - sr * cy;
     m[2][2] = cr * cp;
}

/*
=================
Mat4_SetOrientation

'rotation' is transposed.
=================
*/
void Mat4_SetOrientation(mat4_t m, const mat3_t rotation, const vec3_t translation) {
	m[0][0] = rotation[0][0];
	m[0][1] = rotation[1][0];
	m[0][2] = rotation[2][0];
	m[0][3] = translation[0];

	m[1][0] = rotation[0][1];
	m[1][1] = rotation[1][1];
	m[1][2] = rotation[2][1];
	m[1][3] = translation[1];

	m[2][0] = rotation[0][2];
	m[2][1] = rotation[1][2];
	m[2][2] = rotation[2][2];
	m[2][3] = translation[2];

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;
}

/*
================
Mat3_TransposeMultiplyVector

Need this to receive local space coords in one quick operation.
================
*/
void Mat3_TransposeMultiplyVector (const mat3_t m, const vec3_t in, vec3_t out) {
	out[0] = m[0][0] * in[0] + m[0][1] * in[1] + m[0][2] * in[2];
	out[1] = m[1][0] * in[0] + m[1][1] * in[1] + m[1][2] * in[2];
	out[2] = m[2][0] * in[0] + m[2][1] * in[1] + m[2][2] * in[2];
}


int GL_MsgGLError(char* Info)
{
	char	S[1024];
	int		n = qglGetError();
	
	if(r_ignoreGlErrors->value)
		return false;

	if(n == GL_NO_ERROR) return false;

	switch(n) {        
		case GL_INVALID_ENUM: 
			sprintf(S, "%s GL_INVALID_ENUM An unacceptable value is specified for an enumerated argument. The offending command is ignored, having no side effect other than to set the error flag.\n",Info);
			break;
		case GL_INVALID_VALUE: 
			sprintf(S, "%s GL_INVALID_VALUE A numeric argument is out of range. The offending command is ignored, having no side effect other than to set the error flag.\n",Info);
			break;
		case GL_INVALID_OPERATION: 
			sprintf(S, "%s GL_INVALID_OPERATION The specified operation is not allowed in the current state. The offending command is ignored, having no side effect other than to set the error flag.\n",Info);
			break;
		case GL_STACK_OVERFLOW: 
			sprintf(S, "%s GL_STACK_OVERFLOW This command would cause a stack overflow. The offending command is ignored, having no side effect other than to set the error flag.\n",Info);
			break;
		case GL_STACK_UNDERFLOW: 
			sprintf(S, "%s GL_STACK_UNDERFLOW This command would cause a stack underflow. The offending command is ignored, having no side effect other than to set the error flag.\n",Info);
			break;
		case GL_OUT_OF_MEMORY: 
			sprintf(S, "%s GL_OUT_OF_MEMORY There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.\n",Info);
			break;
		default: 
			sprintf(S, "%s UNKNOWN GL ERROR\n");
			break;
	}

	Con_Printf(PRINT_ALL, S);

	return n;
}



//======================================================================================
void R_Register2(void)
{
	
	r_shadows =							Cvar_Get("r_shadows", "1", CVAR_ARCHIVE);
	r_shadowWorldLightScale =			Cvar_Get("r_shadowWorldLightScale", "10", CVAR_ARCHIVE);
	r_shadowVolumesDebug =				Cvar_Get("r_shadowVolumesDebug", "0", 0);
	r_playerShadow =					Cvar_Get("r_playerShadow", "1", CVAR_ARCHIVE);
	r_shadowCapOffset =					Cvar_Get("r_shadowCapOffset", "0.1", CVAR_ARCHIVE);

	r_anisotropic =						Cvar_Get("r_anisotropic", "1", CVAR_ARCHIVE);
	r_maxAnisotropy =					Cvar_Get("r_maxAnisotropy", "0", 0);
	r_maxTextureSize=					Cvar_Get("r_maxTextureSize", "0", CVAR_ARCHIVE);
	r_overBrightBits =					Cvar_Get("r_overBrightBits", "2", CVAR_ARCHIVE);
	r_textureCompression =				Cvar_Get("r_textureCompression", "0", CVAR_ARCHIVE);			

	r_causticIntens =					Cvar_Get("r_causticIntens", "2.0", CVAR_ARCHIVE);
	r_displayRefresh =					Cvar_Get("r_displayRefresh", "0", CVAR_ARCHIVE);
	
	
	r_screenShot =						Cvar_Get("r_screenShot", "jpg", CVAR_ARCHIVE);
	r_screenShotJpegQuality =			Cvar_Get("r_screenShotJpegQuality", "99", CVAR_ARCHIVE);
		
	r_radarSize =						Cvar_Get("r_radarSize", "256", CVAR_ARCHIVE);
	r_radarZoom =						Cvar_Get("r_radarZoom", "1", CVAR_ARCHIVE);
	r_radar =							Cvar_Get("r_radar", "0", CVAR_ARCHIVE);
	
	r_arbSamples =						Cvar_Get("r_arbSamples", "1", CVAR_ARCHIVE);
	r_nvMultisampleFilterHint =			Cvar_Get ("r_nvMultisampleFilterHint", "fastest", CVAR_ARCHIVE);
	r_nvSamplesCoverange =				Cvar_Get("r_nvSamplesCoverange", "8", CVAR_ARCHIVE);

	deathmatch =						Cvar_Get("deathmatch", "0", CVAR_SERVERINFO);
	
	r_drawFlares =						Cvar_Get("r_drawFlares", "1", CVAR_ARCHIVE);
	r_flaresIntens =					Cvar_Get("r_flaresIntens", "3", CVAR_ARCHIVE);
	r_flareWeldThreshold =				Cvar_Get("r_flareWeldThreshold", "32", CVAR_ARCHIVE);
	
	r_customWidth =						Cvar_Get("r_customWidth", "1024", CVAR_ARCHIVE);
	r_customHeight =					Cvar_Get("r_customHeight", "500", CVAR_ARCHIVE);

	sys_priority =						Cvar_Get("sys_priority", "0", CVAR_ARCHIVE);
	sys_affinity =						Cvar_Get("sys_affinity", "0", CVAR_ARCHIVE);
		
	r_DrawRangeElements	=				Cvar_Get("r_DrawRangeElements","1",CVAR_ARCHIVE);
			
	hunk_bsp=							Cvar_Get("hunk_bsp", "20", CVAR_ARCHIVE);
	hunk_model=							Cvar_Get("hunk_model", "2.4", CVAR_ARCHIVE);
	hunk_sprite=						Cvar_Get("hunk_sprite", "0.08", CVAR_ARCHIVE);
	
//	r_vbo=								Cvar_Get("r_vbo", "1", CVAR_ARCHIVE);

	r_parallax=							Cvar_Get("r_parallax", "2", CVAR_ARCHIVE);
	r_parallaxScale=					Cvar_Get("r_parallaxScale", "0.5", CVAR_ARCHIVE);
	r_parallaxSteps=					Cvar_Get("r_parallaxSteps", "10", CVAR_ARCHIVE);

	r_pplWorldAmbient = 				Cvar_Get("r_pplWorldAmbient", "0.5", CVAR_ARCHIVE);
	r_bumpAlias =						Cvar_Get("r_bumpAlias", "1", CVAR_ARCHIVE);
	r_bumpWorld =						Cvar_Get("r_bumpWorld", "1", CVAR_ARCHIVE);
	r_ambientLevel =					Cvar_Get("r_ambientLevel", "0.75", CVAR_ARCHIVE);
	r_tbnSmoothAngle =					Cvar_Get("r_tbnSmoothAngle", "30", CVAR_ARCHIVE);
	r_pplMaxDlights =					Cvar_Get("r_pplMaxDlights", "8", CVAR_ARCHIVE);

	r_bloom =							Cvar_Get("r_bloom", "1", CVAR_ARCHIVE);
	r_bloomThreshold =					Cvar_Get("r_bloomThreshold", "0.75", CVAR_ARCHIVE);
	r_bloomIntens =						Cvar_Get("r_bloomIntens", "2.0", CVAR_ARCHIVE);

	r_dof =								Cvar_Get("r_dof", "2", CVAR_ARCHIVE);
	r_dofBias =							Cvar_Get("r_dofBias", "0.002", CVAR_ARCHIVE);
	r_dofFocus =						Cvar_Get("r_dofFocus", "256.0", CVAR_ARCHIVE);

	r_radialBlur =						Cvar_Get("r_radialBlur", "1", CVAR_ARCHIVE);
	r_radialBlurFov =                   Cvar_Get("r_radialBlurFov", "30", CVAR_ARCHIVE);
	r_radialBlurSamples =               Cvar_Get("r_radialBlurSamples", "8", CVAR_ARCHIVE);
	r_softParticles =					Cvar_Get("r_softParticles", "1", CVAR_ARCHIVE);

	r_ignoreGlErrors =					Cvar_Get("r_ignoreGlErrors", "1", 0);


}

