/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/*
Copyright (C) 2004-2014 Quake2xp Team.

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

void GL_SelectTexture(GLenum texture)
{
	int tmu;

	tmu = texture - GL_TEXTURE0;

	if (tmu == gl_state.currenttmu)
		return;
	
	qglActiveTexture(texture);
	gl_state.currenttmu = tmu;
}

void GL_Bind(int texnum)
{
	if (gl_state.currenttextures[gl_state.currenttmu] == texnum)
		return;

	qglBindTexture(GL_TEXTURE_2D, texnum);
	gl_state.currenttextures[gl_state.currenttmu] = texnum;
}

void GL_Bind3d(int texnum)
{
	if (gl_state.currenttextures[gl_state.currenttmu] == texnum)
		return;
	
	qglBindTexture(GL_TEXTURE_3D, texnum);
	gl_state.currenttextures[gl_state.currenttmu] = texnum;
}

void GL_BindCube(int texnum)
{
	if (gl_state.currenttextures[gl_state.currenttmu] == texnum)
		return;
	
	qglBindTexture(GL_TEXTURE_CUBE_MAP, texnum);
	gl_state.currenttextures[gl_state.currenttmu] = texnum;
}

void GL_BindRect(int texnum)
{
	if (gl_state.currenttextures[gl_state.currenttmu] == texnum)
		return;

	qglBindTexture(GL_TEXTURE_RECTANGLE, texnum);
	gl_state.currenttextures[gl_state.currenttmu] = texnum;
}

void GL_MBind(GLenum target, int texnum)
{
	int targ = target - GL_TEXTURE0;
	GL_SelectTexture(target);

	if (gl_state.currenttextures[targ] == texnum)
		return;
	GL_Bind(texnum);
}


void GL_MBindCube(GLenum target, int texnum)
{
	int targ = target - GL_TEXTURE0;
	GL_SelectTexture(target);

	if (gl_state.currenttextures[targ] == texnum)
		return;
	GL_BindCube(texnum);
}

void GL_MBindRect(GLenum target, int texnum)
{
	int targ = target - GL_TEXTURE0;
	GL_SelectTexture(target);

	if (gl_state.currenttextures[targ] == texnum)
		return;

	GL_BindRect(texnum);
}

void GL_MBind3d(GLenum target, int texnum)
{
	int targ = target - GL_TEXTURE0;
	GL_SelectTexture(target);

	if (gl_state.currenttextures[targ] == texnum)
		return;

	GL_Bind3d(texnum);
}

/*
=============
GL_CullFace

=============
*/
void GL_CullFace(GLenum mode) {
	if (gl_state.cullMode != mode) {
		qglCullFace(mode);
		gl_state.cullMode = mode;
	}
}

/*
=============
GL_FrontFace

=============
*/
void GL_FrontFace(GLenum mode) {
	if (gl_state.frontFace != mode) {
		qglFrontFace(mode);
		gl_state.frontFace = mode;
	}
}

/*
=============
GL_DepthFunc

=============
*/
void GL_DepthFunc(GLenum func) {
	if (gl_state.depthFunc != func) {
		qglDepthFunc(func);
		gl_state.depthFunc = func;
	}
}

/*
=============
GL_BlendFunc

=============
*/
void GL_BlendFunc(GLenum src, GLenum dst) {
	if (gl_state.blendSrc != src || gl_state.blendDst != dst) {
		qglBlendFunc(src, dst);

		gl_state.blendSrc = src;
		gl_state.blendDst = dst;
	}
}

/*
===============
GL_StencilFunc

===============
*/
void GL_StencilFunc(GLenum func, GLint ref, GLuint mask) {
	if (gl_state.stencilFunc != func || gl_state.stencilRef != ref || gl_state.stencilRefMask != mask) {
		qglStencilFunc(func, ref, mask);

		gl_state.stencilFunc = func;
		gl_state.stencilRef = ref;
		gl_state.stencilRefMask = mask;
	}
}

/*
==============
GL_StencilOp

==============
*/
void GL_StencilOp(GLenum fail, GLenum zFail, GLenum zPass) {
	if (gl_state.stencilFail != fail || gl_state.stencilZFail != zFail || gl_state.stencilZPass != zPass) {
		qglStencilOp(fail, zFail, zPass);

		gl_state.stencilFail = fail;
		gl_state.stencilZFail = zFail;
		gl_state.stencilZPass = zPass;
	}
}

void GL_StencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask){
	if (gl_state.stencilFace != face || gl_state.stencilFunc != func || gl_state.stencilRef != ref || gl_state.stencilRefMask != mask) {
		qglStencilFuncSeparate(face, func, ref, mask);

		gl_state.stencilFace = face;
		gl_state.stencilFunc = func;
		gl_state.stencilRef = ref;
		gl_state.stencilRefMask = mask;
	}
}

void GL_StencilOpSeparate(GLenum face, GLenum fail, GLenum zFail, GLenum zPass) {
	if (gl_state.stencilFace != face || gl_state.stencilFail != fail || gl_state.stencilZFail != zFail || gl_state.stencilZPass != zPass) {
		qglStencilOpSeparate(face, fail, zFail, zPass);

		gl_state.stencilFace = face;
		gl_state.stencilFail = fail;
		gl_state.stencilZFail = zFail;
		gl_state.stencilZPass = zPass;
	}
}
/*
=============
GL_ColorMask

=============
*/
void GL_ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
	if (gl_state.colorMask[0] != red || gl_state.colorMask[1] != green || gl_state.colorMask[2] != blue || gl_state.colorMask[3] != alpha) {
		qglColorMask(red, green, blue, alpha);

		gl_state.colorMask[0] = red;
		gl_state.colorMask[1] = green;
		gl_state.colorMask[2] = blue;
		gl_state.colorMask[3] = alpha;
	}
}

/*
=============
GL_StencilMask

=============
*/
void GL_StencilMask(GLuint mask) {
	if (gl_state.stencilMask != mask) {
		qglStencilMask(mask);
		gl_state.stencilMask = mask;
	}
}

/*
=============
GL_DepthMask

=============
*/
void GL_DepthMask(GLboolean flag) {
	if (gl_state.depthMask != flag) {
		qglDepthMask(flag);
		gl_state.depthMask = flag;
	}
}

void GL_AlphaFunc(GLenum func, GLclampf ref)
{
	if (gl_state.alphaFunc == func && gl_state.alphaRef == ref)
		return;
	gl_state.alphaFunc = func;
	gl_state.alphaRef = ref;
	qglAlphaFunc(func, ref);
}
/*
=============
GL_Scissor

=============
*/
void GL_Scissor(GLint x, GLint y, GLint width, GLint height) {
	if (gl_state.scissor[0] != x || gl_state.scissor[1] != y || gl_state.scissor[2] != width || gl_state.scissor[3] != height) {
		qglScissor(x, y, width, height);

		gl_state.scissor[0] = x;
		gl_state.scissor[1] = y;
		gl_state.scissor[2] = width;
		gl_state.scissor[3] = height;
	}
}

/*
=============
GL_DepthRange

=============
*/
void GL_DepthRange(GLclampd n, GLclampd f) {
	if (gl_state.depthRange[0] != n || gl_state.depthRange[1] != f) {
		qglDepthRange(n, f);

		gl_state.depthRange[0] = n;
		gl_state.depthRange[1] = f;
	}
}


/*
=============
GL_PolygonOffset

=============
*/
void GL_PolygonOffset(GLfloat factor, GLfloat units) {
	if (gl_state.polygonOffsetFactor != factor || gl_state.polygonOffsetUnits != units) {
		qglPolygonOffset(factor, units);

		gl_state.polygonOffsetFactor = factor;
		gl_state.polygonOffsetUnits = units;
	}
}

/*
=============
GL_DepthBoundsTest

=============
*/
void GL_DepthBoundsTest(GLfloat mins, GLfloat maxs) {
	if (gl_state.depthBoundsMins != mins || gl_state.depthBoundsMax != maxs) {
		glDepthBoundsEXT(mins, maxs);

		gl_state.depthBoundsMins = mins;
		gl_state.depthBoundsMax = maxs;
	}
}

/*
===========
GL_Enable

Handles state of the common caps.
===========
*/
void GL_Enable(GLenum cap) {
	switch (cap) {
	case GL_BLEND:
		if (gl_state.blend)
			return;
		gl_state.blend = qtrue;
		break;
	case GL_CULL_FACE:
		if (gl_state.cullFace)
			return;
		gl_state.cullFace = qtrue;
		break;
	case GL_DEPTH_TEST:
		if (gl_state.depthTest)
			return;
		gl_state.depthTest = qtrue;
		break;
	case GL_DEPTH_BOUNDS_TEST_EXT:
		if (gl_state.glDepthBoundsTest)
		return;
		gl_state.glDepthBoundsTest = qtrue;
		break;
	case GL_SCISSOR_TEST:
		if (gl_state.scissorTest)
			return;
		gl_state.scissorTest = qtrue;
		break;
	case GL_STENCIL_TEST:
		if (gl_state.stencilTest)
			return;
		gl_state.stencilTest = qtrue;
		break;
	case GL_POLYGON_OFFSET_FILL:
		if (gl_state.polygonOffsetFill)
			return;
		gl_state.polygonOffsetFill = qtrue;
		break;
	case GL_LINE_SMOOTH:
		if (gl_state.lineSmooth)
			return;
		gl_state.lineSmooth = qtrue;
		break;
	case GL_DEPTH_CLAMP:
		if (gl_state.depthClamp)
			return;
		gl_state.depthClamp = qtrue;

	case GL_ALPHA_TEST:
		if (gl_state.alphaTest)
			return;
		gl_state.alphaTest = qtrue;

	}

	qglEnable(cap);
}

/*
===========
GL_Disable

===========
*/
void GL_Disable(GLenum cap) {
	switch (cap) {
	case GL_BLEND:
		if (!gl_state.blend)
			return;
		gl_state.blend = qfalse;
		break;
	case GL_CULL_FACE:
		if (!gl_state.cullFace)
			return;
		gl_state.cullFace = qfalse;
		break;
	case GL_DEPTH_TEST:
		if (!gl_state.depthTest)
			return;
		gl_state.depthTest = qfalse;
		break;
	case GL_DEPTH_BOUNDS_TEST_EXT:
		if (!gl_state.glDepthBoundsTest)
		return;
		gl_state.glDepthBoundsTest = qfalse;
		break;
	case GL_SCISSOR_TEST:
		if (!gl_state.scissorTest)
			return;
		gl_state.scissorTest = qfalse;
		break;
	case GL_STENCIL_TEST:
		if (!gl_state.stencilTest)
			return;
		gl_state.stencilTest = qfalse;
		break;
	case GL_POLYGON_OFFSET_FILL:
		if (!gl_state.polygonOffsetFill)
			return;
		gl_state.polygonOffsetFill = qfalse;
		break;
	case GL_LINE_SMOOTH:
		if (!gl_state.lineSmooth)
			return;
		gl_state.lineSmooth = qfalse;
		break;
	case GL_DEPTH_CLAMP:
		if (!gl_state.depthClamp)
			return;
		gl_state.depthClamp = qfalse;
	case GL_ALPHA_TEST:
		if (!gl_state.alphaTest)
			return;
		gl_state.alphaTest = qfalse;
	}

	qglDisable(cap);
}

/*
** GL_Strings_f
*/
void GL_Strings_f(void) {
	int			profile, i;
	uint		n, major, minor;
	const char* profileName[] = { "core", "compatibility" };
	char* string = "";

	Com_Printf("\n");
	Com_Printf("GL_VENDOR:    "S_COLOR_GREEN"%s\n", gl_config.vendor_string);
	Com_Printf("GL_RENDERER:  "S_COLOR_GREEN"%s\n", gl_config.renderer_string);
	Com_Printf("GL_VERSION:   "S_COLOR_GREEN"%s\n", gl_config.version_string);

#ifdef _WIN32

	string = (char*)glw_state.wglExtsString;
	qglGetIntegerv(WGL_CONTEXT_PROFILE_MASK_ARB, &profile);
	qglGetIntegerv(GL_MAJOR_VERSION, &major);
	qglGetIntegerv(GL_MINOR_VERSION, &minor);

	Com_Printf("Using OpenGL: "S_COLOR_GREEN"%i.%i"S_COLOR_WHITE" %s profile context\n\n", major, minor, profileName[profile == WGL_CONTEXT_CORE_PROFILE_BIT_ARB ? 0 : 1]);
	Com_Printf("WGL_EXTENSIONS:\n"S_COLOR_YELLOW"%s\n\n", string);
#endif

	qglGetIntegerv(GL_NUM_EXTENSIONS, &n);
	Com_Printf("GL_EXTENSIONS:\n");
	for (i = 0; i < n; i++) {
		gl_config.extensions3_string = (const char*)glGetStringi(GL_EXTENSIONS, i);
		Com_Printf(S_COLOR_YELLOW"%s\n", gl_config.extensions3_string);
	}
}


/*
** GL_SetDefaultState
*/

void GL_SetDefaultState(void) {

	// font color
	colorDefault[0] = 255;
	colorDefault[1] = 255;
	colorDefault[2] = 255;
	colorDefault[3] = 255;

	qglDisable(GL_POLYGON_OFFSET_FILL);
	qglPolygonOffset(0.f, 1.f);
	gl_state.polygonOffsetFill = qfalse;
	gl_state.polygonOffsetFactor = 0.f;
	gl_state.polygonOffsetUnits = 1.f;

	qglDepthRange(0.f, 1.f);
	gl_state.depthRange[0] = 0.f;
	gl_state.depthRange[1] = 1.f;

	// scissor
	qglDisable(GL_SCISSOR_TEST);
	qglScissor(0, 0, vid.width, vid.height);
	gl_state.scissorTest = qfalse;
	gl_state.scissor[0] = 0;
	gl_state.scissor[1] = 0;
	gl_state.scissor[2] = vid.width;
	gl_state.scissor[3] = vid.height;

	// color mask
	qglColorMask(1, 1, 1, 1);
	gl_state.colorMask[0] = GL_TRUE;
	gl_state.colorMask[1] = GL_TRUE;
	gl_state.colorMask[2] = GL_TRUE;
	gl_state.colorMask[3] = GL_TRUE;

	// depth test
	qglDisable(GL_DEPTH_TEST);
	qglDepthFunc(GL_LEQUAL);
	qglDepthMask(1);
	gl_state.depthTest = qfalse;
	gl_state.depthFunc = GL_LEQUAL;
	gl_state.depthMask = qtrue;

	// stencil test
	qglDisable(GL_STENCIL_TEST);
	qglStencilMask(255);
	qglStencilFunc(GL_ALWAYS, 128, 255);
	qglStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	qglStencilOpSeparate(GL_FRONT_AND_BACK, GL_KEEP, GL_KEEP, GL_KEEP);
	gl_state.stencilTest = qfalse;
	gl_state.stencilMask = 255;
	gl_state.stencilFunc = GL_ALWAYS;
	gl_state.stencilRef = 128;
	gl_state.stencilRefMask = 255;
	gl_state.stencilFace = GL_FRONT_AND_BACK;
	gl_state.stencilFail = GL_KEEP;
	gl_state.stencilZFail = GL_KEEP;
	gl_state.stencilZPass = GL_KEEP;

	// blending
	qglDisable(GL_BLEND);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	gl_state.blend = qfalse;
	gl_state.blendSrc = GL_SRC_ALPHA;
	gl_state.blendDst = GL_ONE_MINUS_SRC_ALPHA;

	// face culling
	qglDisable(GL_CULL_FACE);
	qglCullFace(GL_BACK);
	qglFrontFace(GL_CW);
	gl_state.cullFace = qfalse;
	gl_state.cullMode = GL_BACK;
	gl_state.frontFace = GL_CW;

	// depth bounds test
	if (gl_state.depthBoundsTest) {
		gl_state.glDepthBoundsTest = qfalse;
		qglDisable(GL_DEPTH_BOUNDS_TEST_EXT);
		glDepthBoundsEXT(0.f, 1.f);
		gl_state.depthBoundsMins = 0.f;
		gl_state.depthBoundsMax = 1.f;
	}

	gl_state.alphaTest = qfalse;
	gl_state.alphaFunc = GL_GREATER;
	gl_state.alphaRef = 0.666f;

	qglHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);

	qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	GL_TextureMode(r_textureMode->string);

	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	GL_UpdateSwapInterval();
}