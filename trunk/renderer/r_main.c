/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/*
Copyright (C) 1997-2001 Id Software, Inc.

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
// r_main.c
#include "r_local.h"

/*
#ifndef _WIN32
#include <dlfcn.h>
#define qwglGetProcAddress( a ) dlsym( glw_state.hinstOpenGL, a )
#endif
*/

#ifndef _WIN32 // fix sdl bug
#include <SDL.h>
#define qwglGetProcAddress( a ) SDL_GL_GetProcAddress ( a )
#endif

viddef_t vid;

model_t *r_worldmodel;

double gldepthmin, gldepthmax;

glconfig_t gl_config;
glstate_t gl_state;
entity_t *currententity;
model_t *currentmodel;

cplane_t frustum[6];

int r_visframecount;			// bumped when going to a new PVS
int r_framecount;				// used for dlight push checking

float v_blend[4];				// final blending color

void GL_Strings_f(void);

//
// view origin
//
vec3_t vup;
vec3_t vpn;
vec3_t vright;
vec3_t r_origin;

//
// screen size info
//
refdef_t r_newrefdef;

glwstate_t glw_state;

int r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;
int	occ_framecount;
void R_SetupEntityMatrix(entity_t* e);

/*=================
GL_ARB_Debug_output
=================*/
//					
void glDebugOutput(GLenum source, GLenum   type, GLuint   id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
	char	debSource[64],
		debType[64],
		debSev[64];

	if (!gl_state.glDebugOutput)
		return;

	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		return;

	if (source == GL_DEBUG_SOURCE_API)
		strcpy(debSource, "OpenGL");
	else if (source == GL_DEBUG_SOURCE_WINDOW_SYSTEM)
		strcpy(debSource, "Windows");
	else if (source == GL_DEBUG_SOURCE_SHADER_COMPILER)
		strcpy(debSource, "Shader Compiler");
	else if (source == GL_DEBUG_SOURCE_THIRD_PARTY)
		strcpy(debSource, "Third Party");
	else if (source == GL_DEBUG_SOURCE_APPLICATION)
		strcpy(debSource, "Application");
	else if (source == GL_DEBUG_SOURCE_OTHER_ARB)
		strcpy(debSource, "Other");
	else
		strcpy(debSource, va("Source 0x%X", source));

	if (type == GL_DEBUG_TYPE_ERROR)
		strcpy(debType, "Error");
	else if (type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR)
		strcpy(debType, "Deprecated behavior");
	else if (type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
		strcpy(debType, "Undefined behavior");
	else if (type == GL_DEBUG_TYPE_PORTABILITY)
		strcpy(debType, "Portability");
	else if (type == GL_DEBUG_TYPE_PERFORMANCE)
		strcpy(debType, "Performance");
	else if (type == GL_DEBUG_TYPE_OTHER)
		strcpy(debType, "Other");
	else
		strcpy(debType, va("Type 0x%X", type));

	if (severity == GL_DEBUG_SEVERITY_HIGH)
		strcpy(debSev, "High");
	else if (severity == GL_DEBUG_SEVERITY_MEDIUM)
		strcpy(debSev, "Medium");
	else if (severity == GL_DEBUG_SEVERITY_LOW)
		strcpy(debSev, "Low");
	else
		strcpy(debSev, va("0x%X", severity));

	Com_Printf("GL_DEBUG: %s %s\nSeverity '%s': '%s'\nID: '%d'\n", debSource, debType, debSev, message, id);
}

void GetDebugMessages() {

	if (!gl_state.glDebugOutput)
		return;

	GLint maxMsgLen = 0;
	qglGetIntegerv(GL_MAX_DEBUG_MESSAGE_LENGTH, &maxMsgLen);

#define			MAX_GL_DEBUG_MESSAGES   16
	unsigned        sources[MAX_GL_DEBUG_MESSAGES];
	unsigned        types[MAX_GL_DEBUG_MESSAGES];
	unsigned        ids[MAX_GL_DEBUG_MESSAGES];
	unsigned        severities[MAX_GL_DEBUG_MESSAGES];
	int             lengths[MAX_GL_DEBUG_MESSAGES];
	char            messageLog[2048];
	unsigned        count = MAX_GL_DEBUG_MESSAGES;
	int             bufsize = 2048;

	unsigned retVal = glGetDebugMessageLog(count, bufsize, sources, types, ids, severities, lengths, messageLog);
	if (retVal > 0)
	{
		unsigned pos = 0;
		for (unsigned i = 0; i < retVal; i++)
		{
			glDebugOutput(sources[i], types[i], ids[i], severities[i], sizeof(size_t), &messageLog[pos], NULL);
		}
	}
}

/*
=============================================================

  SPRITE MODELS

=============================================================
*/


void R_DrawSpriteModel(entity_t * e)
{
	return;
}


static void R_DrawDistortSpriteModel(entity_t * e)
{
	dsprframe_t *frame;
	float		*up, *right;
	dsprite_t	*psprite;
	int			len, scaled = 1, numVerts=0;

	psprite = (dsprite_t *) currentmodel->extraData;
	e->frame %= psprite->numFrames;
	frame = &psprite->frames[e->frame];

	len = frame->width;

	// normal sprite
	up = vup;
	right = vright;

	qglUniform1f(U_REFR_ALPHA, e->alpha);
	qglUniform1f(U_REFR_THICKNESS0, len * 0.5);
	qglUniform1f(U_REFR_THICKNESS1, len * 0.5);

	if (currententity->flags & RF_BFG_SPRITE) {
		GL_SetBindlessTexture(U_TMU1, gi.blackTexture1x1->handle);
		scaled = 2;
	}
	else		
		GL_SetBindlessTexture(U_TMU1, currentmodel->albedo[e->frame]->handle);
	
	VectorMA (e->origin,		-frame->origin_y * scaled, up, tess3d.v[0].pos);
	VectorMA (tess3d.v[0].pos,	-frame->origin_x * scaled, right, tess3d.v[0].pos);
	
	VectorMA (e->origin,		frame->height * scaled - frame->origin_y * scaled, up, tess3d.v[1].pos);
	VectorMA (tess3d.v[1].pos,	-frame->origin_x * scaled, right, tess3d.v[1].pos);

	VectorMA (e->origin,		frame->height * scaled - frame->origin_y * scaled, up, tess3d.v[2].pos);
	VectorMA (tess3d.v[2].pos,	frame->width * scaled - frame->origin_x * scaled, right, tess3d.v[2].pos);
	
	VectorMA (e->origin,		-frame->origin_y * scaled, up, tess3d.v[3].pos);
	VectorMA (tess3d.v[3].pos,	 frame->width * scaled - frame->origin_x * scaled, right, tess3d.v[3].pos);
	
	VA_SetElem2(tess3d.v[0].tc, 0, 1);
	VA_SetElem2(tess3d.v[1].tc, 0, 0);
	VA_SetElem2(tess3d.v[2].tc, 1, 0);
	VA_SetElem2(tess3d.v[3].tc, 1, 1);

	qglInvalidateBufferData(GL_ARRAY_BUFFER);
	qglBufferSubData(GL_ARRAY_BUFFER, 0, QUAD_INDICES * sizeof(vertex3d_t), &tess3d);

	GL_DrawElements(GL_TRIANGLES, QUAD_INDICES, GL_UNSIGNED_BYTE, NULL);
}

//==================================================================================

/*
=============
R_DrawNullModel
=============
*/
void R_DrawNullModel(void)
{
	return;
}

//=======================================================================

/*
===============
R_SetupFrame
===============
*/
void R_SetupFrame(void)
{
	int i;
	mleaf_t *leaf;

	r_framecount++;
	occ_framecount++;

	// build the transformation matrix for the given view angles
	VectorCopy(r_newrefdef.vieworg, r_origin);

	AngleVectors(r_newrefdef.viewangles, vpn, vright, vup);
	
	// current viewcluster
	if (!(r_newrefdef.rdflags & RDF_NOWORLDMODEL)) {
		r_oldviewcluster = r_viewcluster;
		r_oldviewcluster2 = r_viewcluster2;
		leaf = Mod_PointInLeaf(r_origin, r_worldmodel);
		r_viewcluster = r_viewcluster2 = leaf->cluster;
		
		// check above and below so crossing solid water doesn't draw
		// wrong
		if (!leaf->contents) {	// look down a bit
			vec3_t temp;

			VectorCopy(r_origin, temp);
			temp[2] -= 16;
			leaf = Mod_PointInLeaf(temp, r_worldmodel);
			if (!(leaf->contents & CONTENTS_SOLID) &&
				(leaf->cluster != r_viewcluster2))
				r_viewcluster2 = leaf->cluster;
		} else {				// look up a bit
			vec3_t temp;

			VectorCopy(r_origin, temp);
			temp[2] += 16;
			leaf = Mod_PointInLeaf(temp, r_worldmodel);
			if (!(leaf->contents & CONTENTS_SOLID) &&
				(leaf->cluster != r_viewcluster2))
				r_viewcluster2 = leaf->cluster;
		}
	}

	if(CL_PMpointcontents(r_origin) & CONTENTS_SOLID)
		outMap = true;
	else
		outMap = false;

	for (i = 0; i < 4; i++)
		v_blend[i] = r_newrefdef.blend[i];	
}

/*
=============
R_SetupViewMatrices

=============
*/

// convert from Q2 coordinate system (screen depth is X)
// to OpenGL's coordinate system (screen depth is -Z)
mat4_t r_flipMatrix = {
	{  0.f, 0.f, -1.f, 0.f },	// world +X -> screen -Z
	{ -1.f, 0.f,  0.f, 0.f },	// world +Y -> screen -X
	{  0.f, 1.f,  0.f, 0.f },	// world +Z -> screen +Y (Y=0 being the bottom of the screen)
	{  0.f, 0.f,  0.f, 1.f }
};

static void R_SetupViewMatrices (void) {
	mat4_t	tmpMatrix;
	int		i;

	// setup perspective projection matrix
	float scale = 1.f / (r_zFar->value - r_zNear->value);

	r_newrefdef.projectionMatrix[0][0] = 1.0 / tan(DEG2RAD(r_newrefdef.fov_x) * 0.5);
	r_newrefdef.projectionMatrix[0][1] = 0.0;
	r_newrefdef.projectionMatrix[0][2] = 0.0;
	r_newrefdef.projectionMatrix[0][3] = 0.0;

	r_newrefdef.projectionMatrix[1][0] = 0.0;
	r_newrefdef.projectionMatrix[1][1] = 1.0 / tan(DEG2RAD(r_newrefdef.fov_y) * 0.5);
	r_newrefdef.projectionMatrix[1][2] = 0.0;
	r_newrefdef.projectionMatrix[1][3] = 0.0;

	r_newrefdef.projectionMatrix[2][0] = 0.0;
	r_newrefdef.projectionMatrix[2][1] = 0.0;
#ifdef INFITITY_VIEW
	r_newrefdef.projectionMatrix[2][2] = -0.999; // infinite
#else
	r_newrefdef.projectionMatrix[2][2] = -(r_zFar->value + r_zNear->value) * scale;
#endif
	r_newrefdef.projectionMatrix[2][3] = -1.0;

	r_newrefdef.projectionMatrix[3][0] = 0.0;
	r_newrefdef.projectionMatrix[3][1] = 0.0;
#ifdef INFITITY_VIEW
	r_newrefdef.projectionMatrix[3][2] = -2.0 * r_zNear->value; // infinite
#else
	r_newrefdef.projectionMatrix[3][2] = -2.0 * r_zFar->value * r_zNear->value * scale;
#endif
	r_newrefdef.projectionMatrix[3][3] = 0.0;

#ifdef INFITITY_VIEW
	r_newrefdef.depthParms[0] = r_zNear->value;
	r_newrefdef.depthParms[1] = 0.9995;
#else
	scale = 1.0 / (1.0 - r_zNear->value / r_zFar->value);
	r_newrefdef.depthParms[0] = r_zNear->value * scale;
	r_newrefdef.depthParms[1] = scale;
#endif

	// setup view matrix
	AnglesToMat3(r_newrefdef.viewangles, r_newrefdef.axis);
	Mat4_SetupTransform(r_newrefdef.modelViewMatrix, r_newrefdef.axis, r_newrefdef.vieworg);
	Mat4_AffineInvert(r_newrefdef.modelViewMatrix, tmpMatrix);
	Mat4_Multiply(tmpMatrix, r_flipMatrix, r_newrefdef.modelViewMatrix);

	// scissors transform
	Mat4_Multiply	(r_newrefdef.modelViewMatrix, r_newrefdef.projectionMatrix, r_newrefdef.modelViewProjectionMatrix);
	Mat4_Transpose	(r_newrefdef.modelViewProjectionMatrix, r_newrefdef.modelViewProjectionMatrixTranspose);

	// setup unprojection matrix
	Mat4_Invert(r_newrefdef.modelViewProjectionMatrix, r_newrefdef.unprojMatrix);
	Mat4_Translate(r_newrefdef.unprojMatrix, -(float)vid.width / (float)r_newrefdef.viewport[2], -(float)vid.height / (float)r_newrefdef.viewport[3], -1.0);
	Mat4_Scale(r_newrefdef.unprojMatrix, 2.0 / (float)r_newrefdef.viewport[2], 2.0 / (float)r_newrefdef.viewport[3], 2.0);

	// set sky matrix
	Mat4_Identity(tmpMatrix);
	if(skyrotate)
		Mat4_Rotate(tmpMatrix, r_newrefdef.time * skyrotate, skyaxis[0], skyaxis[1], skyaxis[2]);
	Mat4_Translate(tmpMatrix, -r_origin[0], -r_origin[1], -r_origin[2]);
	Mat4_Copy(tmpMatrix, r_newrefdef.skyMatrix);

	// compute the world-space rays to the far plane corners
	mat3_t axis;
	for (i = 0; i < 3; i++) {
		axis[0][i] = r_newrefdef.axis[0][i];
		axis[1][i] = r_newrefdef.axis[1][i] * tan(DEG2RAD(r_newrefdef.fov_x * 0.5f));
		axis[2][i] = r_newrefdef.axis[2][i] * tan(DEG2RAD(r_newrefdef.fov_y * 0.5f));

		// counter-clockwise order
		r_newrefdef.cornerRays[0][i] = axis[0][i] + axis[1][i] + axis[2][i];	// top left
		r_newrefdef.cornerRays[1][i] = axis[0][i] + axis[1][i] - axis[2][i];	// bottom left
		r_newrefdef.cornerRays[2][i] = axis[0][i] - axis[1][i] - axis[2][i];	// bottom right
		r_newrefdef.cornerRays[3][i] = axis[0][i] - axis[1][i] + axis[2][i];	// top right
	}
}

void R_SetupEntityMatrix(entity_t * e) {

	AnglesToMat3(e->angles, e->axis);
	Mat4_SetOrientation(e->matrix, e->axis, e->origin);
	Mat4_TransposeMultiply(e->matrix, r_newrefdef.modelViewProjectionMatrix, e->orMatrix);

	if ((e->flags & RF_WEAPONMODEL) && (r_leftHand->integer == 1)) { // Flip player weapon
		Mat4_Scale(e->orMatrix, 1.0, -1.0, 1.0);
		GL_CullFace(GL_FRONT);
	} else
		GL_CullFace(GL_BACK);
}

void R_SetupOrthoMatrix(void) {

	GL_Disable(GL_DEPTH_TEST);
	GL_Disable(GL_CULL_FACE);

	// set 2D virtual screen size
	GL_Viewport(0, 0, vid.width, vid.height);

	// setup orthographic projection
	r_newrefdef.orthoMatrix[0][0] = 2.f / (float)vid.width;
	r_newrefdef.orthoMatrix[0][1] = 0.f;
	r_newrefdef.orthoMatrix[0][2] = 0.f;
	r_newrefdef.orthoMatrix[0][3] = 0.f;
	r_newrefdef.orthoMatrix[1][0] = 0.f;
	r_newrefdef.orthoMatrix[1][1] = -2.f / (float)vid.height;
	r_newrefdef.orthoMatrix[1][2] = 0.f;
	r_newrefdef.orthoMatrix[1][3] = 0.f;
	r_newrefdef.orthoMatrix[2][0] = 0.f;
	r_newrefdef.orthoMatrix[2][1] = 0.f;
	r_newrefdef.orthoMatrix[2][2] = -1.f;
	r_newrefdef.orthoMatrix[2][3] = 0.f;
	r_newrefdef.orthoMatrix[3][0] = -1.f;
	r_newrefdef.orthoMatrix[3][1] = 1.f;
	r_newrefdef.orthoMatrix[3][2] = 0.f;
	r_newrefdef.orthoMatrix[3][3] = 1.f;
}

/*
=============
R_SetupGL
=============
*/

void R_SetupGL(void)
{
	// set drawing parms
	GL_Enable(GL_CULL_FACE);
	GL_Disable(GL_BLEND);
	GL_Enable(GL_DEPTH_TEST);
	GL_DepthMask(1);
	GL_DepthRange(0.0, 1.0);
}

/*
=============
R_Clear
=============
*/

void R_DrawPlayerWeaponLightPass(void)
{
	int i;

	if (!r_drawEntities->integer)
		return;

		for (i = 0; i < r_newrefdef.num_entities; i++)	// weapon model
		{
			currententity = &r_newrefdef.entities[i];
			currentmodel = currententity->model;
			if (currententity->flags & RF_TRANSLUCENT)
				continue;

			if (!currentmodel)
				continue;
			if (!(currententity->flags & RF_WEAPONMODEL))
				continue;

			if (currentmodel->type == mod_alias)
				R_DrawAliasModelLightPass(true);

			if (currentmodel->type == mod_alias_md3)
				R_DrawMD3MeshLight(true);	
		}

}
void R_DrawShadowMaps();

void R_DrawLightScene (void)
{
	int i;

	GL_Enable(GL_BLEND);
	GL_BlendFunc(GL_ONE, GL_ONE);

	if (!(r_newrefdef.rdflags & RDF_NOWORLDMODEL)) {

		if (r_lightScissors->integer)
			GL_Enable(GL_SCISSOR_TEST);

		if (gl_state.depthBoundsTest && r_depthBoundsTest->integer)
			GL_Enable(GL_DEPTH_BOUNDS_TEST_EXT);

		GL_Enable(GL_POLYGON_OFFSET_FILL);
	}

	R_PrepareShadowLightFrame(false);
	
	if(shadowLight_frame) {

	for(currentShadowLight = shadowLight_frame; currentShadowLight; currentShadowLight = currentShadowLight->next) {

	if (r_skipStaticLights->integer && currentShadowLight->isStatic)
		continue;
	
	if ((r_newrefdef.rdflags & RDF_NOWORLDMODEL) && !currentShadowLight->isNoWorldModel)
		continue;

	UpdateLightEditor();
	
	R_SetViewLightScreenBounds();

	if(r_lightScissors->integer)
		GL_Scissor(currentShadowLight->scissor[0], currentShadowLight->scissor[1], currentShadowLight->scissor[2], currentShadowLight->scissor[3]);
	
	if(gl_state.depthBoundsTest && r_depthBoundsTest->integer)
		GL_DepthBoundsTest(currentShadowLight->depthBounds[0], currentShadowLight->depthBounds[1]);

	if (!(r_newrefdef.rdflags & RDF_NOWORLDMODEL)) 
		c_numVisLights++;
	
	
	VectorCopy(currentShadowLight->origin, currentShadowLight->lsOrg);

	if (!currentShadowLight->isAmbient && currentShadowLight->isShadow)
			c_staticShadowTris += currentShadowLight->numStaticShadowTris;
	
	R_DrawShadowMaps();
	R_DrawLightWorld();	

	//brush models light pass
	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];

		if (currententity->flags & RF_WEAPONMODEL)
			continue;

		if (currententity->flags & RF_TRANSLUCENT)
			continue;

		if (currententity->flags & RF_DISTORT)
			continue;

		currentmodel = currententity->model;

		if (!currentmodel) {
			R_DrawNullModel();
			continue;
		}
		if (currentmodel->type == mod_brush)
			R_DrawLightBrushModel();
	}

	for (i = 0; i < r_newrefdef.num_entities; i++) { 
		currententity = &r_newrefdef.entities[i];

		if (currententity->flags & RF_WEAPONMODEL)
			continue;

		if (currententity->flags & RF_TRANSLUCENT)
			continue;

		if (currententity->flags & RF_DISTORT)
			continue;

		currentmodel = currententity->model;

		if (!currentmodel) {
			R_DrawNullModel();
			continue;
		}
		
		if (currentmodel->type == mod_alias)
			R_DrawAliasModelLightPass(false);
		
		if (currentmodel->type == mod_alias_md3)
			R_DrawMD3MeshLight(false);
	}
		R_DrawLightBounds();			// debug stuff
	}
	}
	GL_Disable(GL_SCISSOR_TEST);
	if(gl_state.depthBoundsTest && r_depthBoundsTest->integer)
		GL_Disable(GL_DEPTH_BOUNDS_TEST_EXT);

	GL_Disable(GL_BLEND);
	GL_Disable(GL_POLYGON_OFFSET_FILL);
	GL_PolygonOffset(0.0, 0.0);
}

void R_DrawPlayerWeaponAmbient(void)
{
	int i;

	if (!r_drawEntities->integer)
		return;

	// draw non-transparent first
	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];

		if (currententity->flags & RF_TRANSLUCENT)
			continue;

		currentmodel = currententity->model;

		if (!(currententity->flags & RF_WEAPONMODEL))
			continue;

		if (currentmodel->type == mod_alias)
			R_DrawAliasModel(currententity);

		if (currentmodel->type == mod_alias_md3)
			R_DrawMD3Mesh(true);
	}

	// draw transluscent shells
	GL_Enable(GL_BLEND);
	GL_BlendFunc(GL_ONE, GL_ONE);

	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];

		if (!(currententity->flags & RF_TRANSLUCENT))
			continue;

		currentmodel = currententity->model;

		if (!(currententity->flags & RF_WEAPONMODEL))
			continue;

		if (currentmodel->type == mod_alias)
			R_DrawAliasModel(currententity);

		if (currentmodel->type == mod_alias_md3) {
			if (currententity->flags & (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_GOD))
				R_DrawMD3ShellMesh(true);
		}
	}

	GL_Disable(GL_BLEND);
}

void R_DrawPlayerWeapon(void)
{
	if (!r_drawEntities->integer)
		return;
	
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;


	R_DrawPlayerWeaponAmbient();
	R_linearDepth();

	GL_Enable(GL_BLEND);
	GL_BlendFunc(GL_ONE, GL_ONE);

	if (r_lightScissors->integer)
		GL_Enable(GL_SCISSOR_TEST);

	if (gl_state.depthBoundsTest && r_depthBoundsTest->integer)
		GL_Enable(GL_DEPTH_BOUNDS_TEST_EXT);

	R_PrepareShadowLightFrame(true);

	if (shadowLight_frame) {

		for (currentShadowLight = shadowLight_frame; currentShadowLight; currentShadowLight = currentShadowLight->next) {

			if (r_skipStaticLights->integer && currentShadowLight->isStatic)
				continue;

			if (currentShadowLight->filter == 33)  // flashlight cut off
				continue;

			R_SetViewLightScreenBounds();

			if (r_lightScissors->integer)
				GL_Scissor(currentShadowLight->scissor[0], currentShadowLight->scissor[1], currentShadowLight->scissor[2], currentShadowLight->scissor[3]);

			if (gl_state.depthBoundsTest && r_depthBoundsTest->integer)
				GL_DepthBoundsTest(currentShadowLight->depthBounds[0], currentShadowLight->depthBounds[1]);
			
			R_DrawShadowMaps();
			R_DrawPlayerWeaponLightPass();
		}
	}
	GL_Disable(GL_SCISSOR_TEST);
	if (gl_state.depthBoundsTest && r_depthBoundsTest->integer)
		GL_Disable(GL_DEPTH_BOUNDS_TEST_EXT);
	GL_Disable(GL_BLEND);
}

void R_RenderSprites(void)
{
	int i;
	
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	// setup program
	GL_BindProgram(spriteProgram);

	GL_SetBindlessTexture(U_TMU0, gi.distort->handle);
	GL_SetBindlessTexture(U_TMU2, gi.hdrBaseInterim->handle);
	GL_SetBindlessTexture(U_TMU3, gi.linearDepth->handle);

	qglUniform1f(U_REFR_DEFORM_MUL, 4.5);
	qglUniformMatrix4fv(U_MVP_MATRIX, 1, false, (const float *)r_newrefdef.modelViewProjectionMatrix);
	qglUniformMatrix4fv(U_MODELVIEW_MATRIX, 1, false, (const float *)r_newrefdef.modelViewMatrix);
	qglUniformMatrix4fv(U_PROJ_MATRIX, 1, false, (const float *)r_newrefdef.projectionMatrix);

	qglUniform2f(U_SCREEN_SIZE, vid.width, vid.height);
	qglUniform2f(U_REFR_MASK, 0.0, 1.0);

	GL_BindVAO(vao.stream3d);
	GL_BindVBO(vbo.stream3d);
	GL_BindVBO(vbo.quadIbo);

	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];
		currentmodel = currententity->model;

		if (!currentmodel || currentmodel->type != mod_sprite)
			continue;
			R_DrawDistortSpriteModel(currententity);
	}
}

// draws ambient opaque entities
static void R_DrawOpaqueEntities(void) {
	int i;

	if (!r_drawEntities->integer)
		return;

	// draw non-transparent first
	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];

		if (currententity->flags & (RF_TRANSLUCENT | RF_WEAPONMODEL) || ((currententity->flags & RF_DISTORT) && !(r_newrefdef.rdflags & RDF_IRGOGGLES)))
			continue;

		if (currententity->flags & (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_GOD))
			continue;
		
		if (currententity->flags & RF_BEAM) 
			continue;

		currentmodel = currententity->model;

		if (!currentmodel) {
			R_DrawNullModel();
			continue;
		}

		switch (currentmodel->type) {
			case mod_alias:
				R_DrawAliasModel(currententity);
				break;
			case mod_brush:
				R_DrawBrushModel();
				break;
			case mod_sprite:
				R_DrawSpriteModel(currententity);
				break;
			case mod_alias_md3:
				R_DrawMD3Mesh(false);
				break;
			default:
				VID_Error(ERR_DROP, "Bad modeltype");
				break;
		}
	}
}

static void R_DrawTransEntities(void) {
	int i;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	if (!r_drawEntities->integer)
		return;

	GL_Enable(GL_BLEND);
	GL_BlendFunc(GL_ONE, GL_ONE);

	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];

		if (!(currententity->flags & RF_TRANSLUCENT))
			continue;

		if (currententity->flags & (RF_WEAPONMODEL))
			continue;

		if (currententity->flags & RF_BEAM)
			continue;

		currentmodel = currententity->model;

		if (!currentmodel) {
			R_DrawNullModel();
			continue;
		}

		if (currentmodel->type == mod_alias)
			R_DrawAliasModel(currententity);

		if (currentmodel->type == mod_alias_md3) {
			if (currententity->flags & (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM | RF_SHELL_GOD))
				R_DrawMD3ShellMesh(false);
		}
	}
	GL_Disable(GL_BLEND);
}

// draw all opaque, non-reflective stuff
void R_DrawAmbientScene (void) {

	R_DrawBSP();
	R_DrawOpaqueEntities();
}

// draws reflective & alpha chains from world model
// draws reflective surfaces from brush models
void R_DrawRAScene (void) {
	int i;
	
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	GL_PolygonOffset(-1.0, 1.0);

	RA_Frame = false;

	if (r_reflectiveSurfaces || r_alphaSurfaces)
		RA_Frame = true;

	R_DrawSurfacesRA(false);

	GL_PolygonOffset(0.0, 1.0);

	R_CaptureColorBuffer();

	if (!r_drawEntities->integer)
		return;

	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];
		currentmodel = currententity->model;
		
		if (currentmodel && currentmodel->type == mod_brush) {
			R_DrawBrushModelRA();
			continue;
		}
	}
}

void R_linearDepth(void)
{
	if (r_newrefdef.rdflags & (RDF_NOWORLDMODEL))
		return;

	R_SetupOrthoMatrix();

	qglBindFramebuffer(GL_FRAMEBUFFER, fb.linearDepth->id);

	GL_BindProgram(linearDepthProgram);
	GL_SetBindlessTexture(U_TMU0, gi.rboDepth->handle);

	qglUniform2f(U_DEPTH_PARAMS, r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);
	qglUniformMatrix4fv(U_ORTHO_MATRIX, 1, false, (const float*)r_newrefdef.orthoMatrix);

	R_DrawFullScreenQuad();

	GL_Enable(GL_CULL_FACE);
	GL_Enable(GL_DEPTH_TEST);

	GL_Viewport(r_newrefdef.viewport[0], r_newrefdef.viewport[1],
		r_newrefdef.viewport[2], r_newrefdef.viewport[3]);

	qglBindFramebuffer(GL_FRAMEBUFFER, fb.hdrBase->id);
}

/*
================
R_RenderView

r_newrefdef must be set before the first call.
================
*/
void R_SetViewport3D(refdef_t* fd) {
	
	r_newrefdef = *fd;

	r_newrefdef.viewport[0] = fd->x;
	r_newrefdef.viewport[1] = vid.height - fd->height - fd->y;
	r_newrefdef.viewport[2] = fd->width;
	r_newrefdef.viewport[3] = fd->height;

	GL_Viewport(r_newrefdef.viewport[0], r_newrefdef.viewport[1], r_newrefdef.viewport[2], r_newrefdef.viewport[3]);
}

void R_RenderView (refdef_t *fd) {
	
	if (r_noRefresh->integer)
		return;

	R_SetViewport3D(fd);

	if (!r_worldmodel && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
		VID_Error(ERR_DROP, "R_RenderView: NULL worldmodel.");

	R_SetupFrame();
	R_SetFrustum(true);
	R_SetupViewMatrices();
	R_SetupGL();
	R_MarkLeaves();				// done here so we know if we're in water
	
	// clear out the portion of the screen that the NOWORLDMODEL defines
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL) {

		GL_Enable(GL_SCISSOR_TEST);
		GL_Scissor(r_newrefdef.viewport[0], r_newrefdef.viewport[1], r_newrefdef.viewport[2], r_newrefdef.viewport[3]);

		if (!(r_newrefdef.rdflags & RDF_NOCLEAR)) {
			qglClearBufferfv(GL_COLOR, 0, clearColor);
			qglClearBufferfv(GL_DEPTH, 0, &clearDepth);
		}
		else {
			qglClearBufferfv(GL_DEPTH, 0, &clearDepth);
		}
	}
	else {
		GL_Disable(GL_SCISSOR_TEST);
		qglBindFramebuffer(GL_FRAMEBUFFER, fb.hdrBase->id);
		qglClearBufferfv(GL_COLOR, 0, clearColor);
		qglClearBufferfv(GL_DEPTH, 0, &clearDepth);
	}

	R_DrawDepthScene();
	R_SetFrustum(false);
	R_linearDepth();
	
	R_SSAO();
	R_DrawAmbientScene();
	R_DrawLightScene();
	R_RenderDecals(false);

	R_DrawParticles();
	R_CaptureColorBuffer();
	R_RenderSprites();
	R_DrawRAScene();
	R_DrawLightWorldRA();
	R_RenderDecals(true);

	if (RA_Frame && r_particlesOverdraw->integer) { // overdraw particles if we have trans or reflective surfaces in frame
		R_DrawParticles();
		RA_Frame = false;
	}
	R_DrawTransEntities();

	R_CaptureColorBuffer();
	R_MotionBlur();
	R_CaptureColorBuffer();
	
	if(!outMap)
		R_GlobalFog();

	R_DrawPlayerWeapon();
	R_CaptureColorBuffer();
}


/*
====================
R_SetLightLevel

====================
*/
void R_SetLightLevel (void) {
	vec3_t amb;
	float mid;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	// save off light value for server to look at (BIG HACK!)
	R_LightPoint(r_newrefdef.vieworg, amb);

	mid = max(max(amb[0], amb[1]), amb[2]);
	if (mid <= 0.1)
		mid = 0.15;

	mid *= 2.0;
	r_lightLevel->value = 150 * mid; // convert to byte
}

/*
@@@@@@@@@@@@@@@@@@@@@
R_RenderFrame

@@@@@@@@@@@@@@@@@@@@@
*/
extern char buff0[128];
extern char buff1[128];
extern char buff2[128];
extern char buff3[128];
extern char buff4[128];
extern char buff5[128];
extern char buff6[128];
extern char buff7[128];
extern char buff8[128];
extern char buff9[128];
extern char buff10[128];
extern char buff11[128];
extern char buff12[128];
extern char buff13[128];
extern char buff14[128];
extern char buff15[128];
extern char buff16[128];
extern char buff17[128];
extern char buff18[128];

extern worldShadowLight_t *selectedShadowLight;

void R_RenderFrame(refdef_t * fd) {

	R_RenderView(fd);
	R_SetupOrthoMatrix();
	R_SetLightLevel();
	// post processing - cut off if player camera is out of map bounds
	if (!outMap) {
		R_FixFov();
		R_RadialBlur();
		R_ThermalVision();
		R_DofBlur();
		R_LensFlares();
		R_Bloom();
		R_FilmFx();
		R_ScreenBlend();
	}

	R_ToneMaping();
	
	if (!outMap)
		R_ColorTemperatureCorrection();

	// set alpha blend for 2D mode
	GL_Enable(GL_BLEND); 
	GL_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL_DepthMask(1);
	
	if (selectedShadowLight && r_lightEditor->integer){
		RE_SetColor(colorCyan);
		CL_AddString(0, VID_CENTER_H,       3, buff0, gi.consFont);
		CL_AddString(0, VID_CENTER_H + 25,  3, buff1, gi.consFont);
		CL_AddString(0, VID_CENTER_H + 50,  3, buff2, gi.consFont);
		CL_AddString(0, VID_CENTER_H + 75,  3, buff3, gi.consFont);
		CL_AddString(0, VID_CENTER_H + 100, 3, buff4, gi.consFont);
		CL_AddString(0, VID_CENTER_H + 125, 3, buff5, gi.consFont);
		CL_AddString(0, VID_CENTER_H + 150, 3, buff6, gi.consFont);
		CL_AddString(0, VID_CENTER_H + 175, 3, buff7, gi.consFont);
		CL_AddString(0, VID_CENTER_H + 200, 3, buff8, gi.consFont);
		CL_AddString(0, VID_CENTER_H + 225, 3, buff9, gi.consFont);
		CL_AddString(0, VID_CENTER_H + 250, 3, buff12, gi.consFont);
		CL_AddString(0, VID_CENTER_H + 275, 3, buff13, gi.consFont);
		CL_AddString(0, VID_CENTER_H + 300, 3, buff10, gi.consFont);
		CL_AddString(0, VID_CENTER_H + 325, 3, buff11, gi.consFont);
		CL_AddString(0, VID_CENTER_H + 350, 3, buff14, gi.consFont);
		CL_AddString(0, VID_CENTER_H + 375, 3, buff15, gi.consFont);

//		Draw_StringScaled(0, VID_CENTER_H + 325, 2, 2, buff16, true);
	//	Draw_StringScaled(0, VID_CENTER_H + 345, 2, 2, buff17, true);
		CL_AddString(0, VID_CENTER_H + 400, 3, buff18, gi.consFont);
		RE_SetColor(colorWhite);
	}
}

void AutoLightsStatsList_f(void){

	Com_Printf("%i raw auto lights\n", r_numAutoLights);
	Com_Printf("%i clean auto lights\n", r_numIgnoreAutoLights);
	Com_Printf("%i total auto lights\n", r_numAutoLights - r_numIgnoreAutoLights);
}


void Dump_EntityString(void){

	char *buf;
	FILE *f;
	char name[MAX_OSPATH];
	
	if(!r_worldmodel->name[0]){
		Com_Printf(S_COLOR_RED"You must be in a game to dump entity string\n");
			return;
	}

	Com_sprintf(name, sizeof(name), "%s/%s", FS_Gamedir(), r_worldmodel->name);
	
	name[strlen(name) - 4] = 0;
	strcat(name, ".ent");
	
	Com_Printf("Dump entity string to "S_COLOR_GREEN"%s\n", name);
	FS_CreatePath(name);

	buf = CM_EntityString();
	
	f = fopen(name, "w");
	if (!f) {
		Com_Printf(S_COLOR_RED"ERROR: couldn't open.\n");
		return;
	}

	fputs(buf, f);
	fclose(f);


}
/*
=================
SkipRestOfLine
=================
*/
/*
void SkipRestOfLine(char **data) {
	char    *p;
	int     c;

	p = *data;
	while ((c = *p++) != 0) {
		if (c == '\n') {
			//	com_lines++;
			break;
		}
	}

	*data = p;
}

void Cube2Lut_f(void)
{
	char *buf, *buf0;
	FILE *out;
	char name[MAX_OSPATH], outName[MAX_OSPATH];
	int i, len, lutSize = 0;
	char *token, *titlePtr = NULL;
	float x, y, z;
	extern int lutCount;

	if (Cmd_Argc() != 2) {
		Com_Printf(S_COLOR_YELLOW"Usage: %s <adobe cube lut filename>\n", Cmd_Argv(0));
		return;
	}

	Com_sprintf(name, sizeof(name), "gfx/lut/%s.cube", Cmd_Argv(1));
	len = FS_LoadFile(name, (void**)&buf);
	if (!buf || len <= 0)
	{
		if (buf)
			FS_FreeFile(buf);
		Com_Printf(S_COLOR_RED"ERROR: couldn't open %s\n", name);
		return;
	}

	Com_sprintf(outName, sizeof(outName), "%s/gfx/lut/lut_%i.lut", FS_Gamedir(), lutCount);
	out = fopen(outName, "wb");
	if (!out) {
		FS_FreeFile(buf);
		Com_Printf(S_COLOR_RED"ERROR: couldn't open %s\n", outName);
		return;
	}

	buf0 = buf;
	while (buf)
	{
		if (sscanf(buf, "%f %f %f", &x, &y, &z) == 3) {

			if (lutSize <= 0) {
				FS_FreeFile(buf0);
				fclose(out);
				Com_Printf(S_COLOR_RED"ERROR: lut file %s is corrupted\n", name);
				return;
			}
			// save lut size
			fwrite(&lutSize, 1, sizeof(lutSize), out);
			// read lut block
			int lutBlockSize = pow(lutSize, 3);
			for (i = 0; i < lutBlockSize; i++)
			{
				if (sscanf(buf, "%f %f %f", &x, &y, &z) != 3) {
					FS_FreeFile(buf0);
					fclose(out);
					Com_Printf(S_COLOR_RED"ERROR: lut file %s is truncated\n", name);
					return;
				}
				fwrite(&x, 1, sizeof(x), out);
				fwrite(&y, 1, sizeof(y), out);
				fwrite(&z, 1, sizeof(z), out);
				SkipRestOfLine(&buf);
			}
			break;
		}

		token = COM_Parse(&buf);
		if (token[0] == '#')
		{   // comment
			SkipRestOfLine(&buf);
			continue;
		}
		if (!Q_strcasecmp(token, "LUT_3D_SIZE"))
		{   // lut size
			token = COM_Parse(&buf);
			lutSize = atoi(token);
			continue;
		}
		if (!Q_strcasecmp(token, "TITLE"))
		{   // name of lut
			titlePtr = buf;
			SkipRestOfLine(&buf);
			continue;
		}
		SkipRestOfLine(&buf);
	}

	if (titlePtr)
	{
		token = COM_Parse(&titlePtr);
		fwrite(token, 1, strlen(token), out);
	}
	FS_FreeFile(buf0);
	fclose(out);
	Com_Printf(S_COLOR_YELLOW"wrote: %s\n", outName);
}
*/
void R_GLSLinfo_f(void);
void GL_LevelShot_f(void);
void R_FogEditor_f(void);
void R_ScaleLightColor_f(void);

void R_RegisterCvars(void)
{
	r_leftHand =						Cvar_Get("hand", "0", CVAR_USERINFO | CVAR_ARCHIVE);
	r_noRefresh =						Cvar_Get("r_noRefresh", "0", 0);
	r_drawEntities =					Cvar_Get("r_drawEntities", "1", 0);
	r_drawWorld =						Cvar_Get("r_drawWorld", "1", 0);
	r_noVis =							Cvar_Get("r_noVis", "0", 0);
	r_noCull =							Cvar_Get("r_noCull", "0", 0);
	r_speeds =							Cvar_Get("r_speeds", "0", 0);

	r_lightLevel =						Cvar_Get("r_lightLevel", "0", 0);

	r_mode =							Cvar_Get("r_mode", "0", CVAR_ARCHIVE);
	r_noBind =							Cvar_Get("r_noBind", "0", 0);
	r_lockPvs =							Cvar_Get("r_lockPvs", "0", 0);

	r_vsync =							Cvar_Get("r_vsync", "0", CVAR_ARCHIVE);
	
	r_fullScreen =						Cvar_Get("r_fullScreen", "1", CVAR_ARCHIVE);

	r_hdrEVcomp =						Cvar_Get("r_hdrEVcomp", "0.0", CVAR_ARCHIVE);
	r_hdrLightScale =					Cvar_Get("r_hdrLightScale", "1.0", CVAR_ARCHIVE);
	r_hdrBloom =						Cvar_Get("r_hdrBloom", "1", CVAR_ARCHIVE);
//	r_hdrGlarePasses =					Cvar_Get("r_hdrGlarePasses", "8", CVAR_ARCHIVE);
//	r_hdrGlareIntens =					Cvar_Get("r_hdrGlareIntens", "1.6", CVAR_ARCHIVE);
	r_hdrLensFlaresIntens =				Cvar_Get("r_hdrLensFlaresIntens", "0.2", CVAR_ARCHIVE);
	r_hdrColorSpace =					Cvar_Get("r_hdrColorSpace", "0", CVAR_ARCHIVE);
	r_hdrColorSpace->help =				" 0 = sRGB/D65 \n| 1 = DCI-P3/D65b \n| 2 = Rec.2020/D65 \n| 3 = ACES AP0/D60 \n| 4 = ACES AP1/D60";
	r_hdrUiNits =						Cvar_Get("r_hdrUiNits", "300.0", CVAR_ARCHIVE);
	r_hdrMaxIso =						Cvar_Get("r_hdrMaxIso", "300.0", CVAR_ARCHIVE);

	r_brightness =						Cvar_Get("r_brightness", "1.0", CVAR_ARCHIVE);
	r_contrast =						Cvar_Get("r_contrast", "1.0", CVAR_ARCHIVE);
	r_saturation =						Cvar_Get("r_saturation", "1.0", CVAR_ARCHIVE);
	r_gamma =							Cvar_Get("r_gamma", "1.9", CVAR_ARCHIVE);
	r_colorVibrance =					Cvar_Get("r_colorVibrance", "0.0", CVAR_ARCHIVE);
	r_colorBalanceRed =					Cvar_Get("r_colorBalanceRed", "1.0", CVAR_ARCHIVE);
	r_colorBalanceGreen =				Cvar_Get("r_colorBalanceGreen", "1.0", CVAR_ARCHIVE);
	r_colorBalanceBlue =				Cvar_Get("r_colorBalanceBlue", "1.0", CVAR_ARCHIVE);

	r_displayRefresh =					Cvar_Get("r_displayRefresh", "0", CVAR_ARCHIVE);

	r_textureAnisotropy =				Cvar_Get("r_textureAnisotropy", "16", CVAR_ARCHIVE);
	r_textureLodBias =					Cvar_Get("r_textureLodBias", "0.0", CVAR_ARCHIVE);
	r_imageAutoBump	=					Cvar_Get("r_imageAutoBump", "1", CVAR_ARCHIVE);
	r_imageAutoBumpScale =				Cvar_Get("r_imageAutoBumpScale", "6.0", CVAR_ARCHIVE);
	r_imageAutoSpecularScale =			Cvar_Get("r_imageAutoSpecularScale", "1", CVAR_ARCHIVE);
	r_textureQuality =					Cvar_Get("r_textureQuality", "0", CVAR_ARCHIVE);
	r_textureQuality->help =			"textures quality: 0 - max quality\n";

	r_screenShot =						Cvar_Get("r_screenShot", "jpg", CVAR_ARCHIVE);
	r_screenShot->help =				"jpg, tga, png, bmp, hdr";

	r_multiSamples =					Cvar_Get("r_multiSamples", "0", CVAR_ARCHIVE);
	r_fxaa =							Cvar_Get("r_fxaa", "1", CVAR_ARCHIVE);

	deathmatch =						Cvar_Get("deathmatch", "0", CVAR_SERVERINFO);
	
	r_hdrLensFlares =					Cvar_Get("r_hdrLensFlares", "1", CVAR_ARCHIVE);
	r_scaleAutoLightColor =				Cvar_Get("r_scaleAutoLightColor", "3", CVAR_ARCHIVE);

	r_customWindowWidth =				Cvar_Get("r_customWindowWidth", "0", CVAR_ARCHIVE);
	r_customWindowWidth->help =			"Minimal value is 1024\n Minimal custom resolution 1024x768";
	r_customWindowHeight =				Cvar_Get("r_customWindowHeight", "0", CVAR_ARCHIVE);
	r_customWindowHeight->help =		"Minimal value is 768\n Minimal custom resolution 1024x768";

	r_parallaxMapping =					Cvar_Get("r_parallaxMapping", "1", CVAR_ARCHIVE);
	r_parallaxScale =					Cvar_Get("r_parallaxScale", "2.0", CVAR_ARCHIVE);
	r_selfShadowingParallax =			Cvar_Get("r_selfShadowingParallax", "1", CVAR_ARCHIVE);
	r_selfShadowOffset =				Cvar_Get("r_selfShadowOffset", "3.0", CVAR_ARCHIVE);
	r_selfShadowBlur =					Cvar_Get("r_selfShadowBlur", "0.7", CVAR_ARCHIVE);

	r_shadows =							Cvar_Get("r_shadows", "1", /*CVAR_VIDEO_DEBUG*/CVAR_ARCHIVE);
	r_playerShadow =					Cvar_Get("r_playerShadow", "1", CVAR_ARCHIVE);
	r_penumbraSize =					Cvar_Get("r_penumbraSize", "4.0", CVAR_ARCHIVE);

	r_blinnPhongLighting =				Cvar_Get("r_blinnPhongLighting", "0", CVAR_ARCHIVE);
	r_blinnPhongLighting->help =		"use old lighting model";

	r_skipStaticLights =				Cvar_Get("r_skipStaticLights", "0", CVAR_VIDEO_DEBUG);
	r_lightmapScale =					Cvar_Get("r_lightmapScale", "0.5", CVAR_ARCHIVE);
	r_lightScissors = 					Cvar_Get("r_lightScissors", "1", 0);
	r_depthBoundsTest =					Cvar_Get("r_depthBoundsTest", "1", 0);
	r_debugLights =						Cvar_Get("r_debugLights", "0", 0);
	r_transSurfShading =				Cvar_Get("r_transSurfShading", "1", CVAR_ARCHIVE);

	r_debugTbn =						Cvar_Get("r_debugTbn", "0", 0);
	r_debugTbnLen =						Cvar_Get("r_debugTbnLen", "1.0", 0);
	r_bspSmoothTbn =					Cvar_Get("r_bspSmoothTbn", "1", CVAR_ARCHIVE);

	r_showTris =						Cvar_Get("r_showTris", "0", 0);

	r_radiositySpecularScale =			Cvar_Get("r_radiositySpecularScale", "0.3", CVAR_ARCHIVE);
	r_radiosityNormalMapping =			Cvar_Get("r_radiosityNormalMapping", "1", CVAR_ARCHIVE);
	r_zNear =							Cvar_Get("r_zNear", "3", CVAR_ARCHIVE);
	r_zFar =							Cvar_Get("r_zFar", "4096", CVAR_ARCHIVE);

	r_ssao =							Cvar_Get ("r_ssao", "1", CVAR_ARCHIVE);
	r_ssaoIntensity =					Cvar_Get ("r_ssaoIntensity", "1.5", CVAR_ARCHIVE);
	r_ssaoScale =						Cvar_Get ("r_ssaoScale", "80.0", CVAR_ARCHIVE);
	r_ssaoBlur	=						Cvar_Get ("r_ssaoBlur", "2", CVAR_ARCHIVE);

	r_dof =								Cvar_Get("r_dof", "1", CVAR_ARCHIVE);
	r_dofBias =							Cvar_Get("r_dofBias", "0.002", CVAR_ARCHIVE);
	r_dofFocus =						Cvar_Get("r_dofFocus", "0.0", CVAR_ARCHIVE);

	r_motionBlur =						Cvar_Get("r_motionBlur", "1", CVAR_ARCHIVE);
	r_motionBlurSamples	=				Cvar_Get("r_motionBlurSamples", "32", CVAR_ARCHIVE);
	r_motionBlurFrameLerp =				Cvar_Get("r_motionBlurFrameLerp", "20", CVAR_ARCHIVE);

	r_radialBlur =						Cvar_Get("r_radialBlur", "1", CVAR_ARCHIVE);
	r_radialBlurFov =                   Cvar_Get("r_radialBlurFov", "30", CVAR_ARCHIVE);
	
	r_filmicFx = 						Cvar_Get("r_filmicFx", "0", CVAR_ARCHIVE);
	r_filmicFxVignetSize =				Cvar_Get("r_filmicFxVignetSize", "0.4", CVAR_ARCHIVE);

	r_glDebugOutput =					Cvar_Get("r_glDebugOutput", "0", 0);
	r_glMajorVersion =					Cvar_Get("r_glMajorVersion", "4", CVAR_ARCHIVE);
	r_glMinorVersion =					Cvar_Get("r_glMinorVersion", "6", CVAR_ARCHIVE);
	r_glCoreProfile =					Cvar_Get("r_glCoreProfile", "1", CVAR_ARCHIVE);
	r_contextNoError =					Cvar_Get("r_contextNoError", "0", CVAR_ARCHIVE);
	r_useHdrDisplay =					Cvar_Get("r_useHdrDisplay", "0", CVAR_ARCHIVE);

	r_debug =							Cvar_Get("r_debug", "0", 0);

	r_lightEditor =						Cvar_Get("r_lightEditor", "0", 0);
	r_cameraSpaceLightMove =			Cvar_Get("r_cameraSpaceLightMove", "0", CVAR_ARCHIVE);

	r_drawPicBump =						Cvar_Get("r_drawPicBump", "1", CVAR_ARCHIVE);
	r_drawPicBump->help =				"hud pics lighing";

	r_fixFovStrength =					Cvar_Get("r_fixFovStrength", "0", CVAR_ARCHIVE);
	r_fixFovStrength->help =			"0.0 no perspective correction";
	r_fixFovDistroctionRatio =			Cvar_Get("r_fixFovDistroctionRatio", "5", CVAR_ARCHIVE);
	r_fixFovDistroctionRatio->help =	"cylindrical distortion ratio";

	r_screenBlend =						Cvar_Get("r_screenBlend", "1", CVAR_ARCHIVE);
	r_screenBlendIntensity =			Cvar_Get("r_screenBlendIntensity", "0.75", CVAR_ARCHIVE);
	r_globalFog =						Cvar_Get("r_globalFog", "1", CVAR_ARCHIVE);
	r_fogEditor =						Cvar_Get("r_fogEditor", "0", 0);
	r_fogEditor->help =					 "type fogEdit ? for help list.";

	r_useShaderCache =					Cvar_Get("r_useShaderCache", "0", 0);
	r_particlesOverdraw =				Cvar_Get("r_particlesOverdraw", "1", CVAR_ARCHIVE);

	r_colorTempK =						Cvar_Get("r_colorTempK", "6500", CVAR_ARCHIVE);
	r_colorTempK->help =				"Color Temperature in Kelvins (from 1000K to 40000K)";
	r_colorTempK->integer =				ClampCvarInteger(1000, 40000, r_colorTempK->integer);

	r_useColorCorrection =				Cvar_Get("r_useColorCorrection", "1", CVAR_ARCHIVE);
	r_nsightDebug =						Cvar_Get("r_nsightDebug", "0", 0);
	r_nsightDebug->help =				"Enable Nvidia Nsight Graphics frame delimiter";

	Cmd_AddCommand("imagelist",			GL_ImageList_f);
	Cmd_AddCommand("screenshot",		GL_ScreenShot_f);
	Cmd_AddCommand("levelshot",			GL_LevelShot_f);
	Cmd_AddCommand("modellist",			Mod_Modellist_f);
	Cmd_AddCommand("openglInfo",		GL_Strings_f);
	Cmd_AddCommand("autoLightsStats",	AutoLightsStatsList_f);
	Cmd_AddCommand("dumpEntityString",	Dump_EntityString);
	Cmd_AddCommand("glslInfo",			R_ListPrograms_f);
	Cmd_AddCommand("glsl",				R_GLSLinfo_f);

	//Cmd_AddCommand("makeLut",			Cube2Lut_f);

	Cmd_AddCommand("fogEdit",			R_FogEditor_f);

#ifdef _WIN32
	Cmd_AddCommand("gpuInfo",			R_GpuInfo_f);
#endif

/*
bind INS		"spawnLight"
bind HOME		"spawnLightToCamera"
bind END		"saveLights"
bind DEL		"removeLight"
bind LEFTARROW	"moveLight_right   -1"
bind RIGHTARROW "moveLight_right    1"
bind UPARROW	"moveLight_forward  1"
bind DOWNARROW	"moveLight_forward -1"
bind PGUP		"moveLight_z        1"
bind PGDN		"moveLight_z       -1"
bind KP_MINUS	"changeLightRadius -5"
bind KP_PLUS	"changeLightRadius  5"
bind KP_INS		"copyLight"
bind KP_DEL		"unselectLight"
bind c			"copy"
bind v			"paste"
*/

	Cmd_AddCommand("saveLights",				R_SaveLights_f);
	Cmd_AddCommand("spawnLight",				R_Light_Spawn_f);
	Cmd_AddCommand("spawnLightToCamera",		R_Light_SpawnToCamera_f);
	Cmd_AddCommand("removeLight",				R_Light_Delete_f);
	Cmd_AddCommand("editLight",					R_EditSelectedLight_f);
	Cmd_AddCommand("moveLight_right",			R_MoveLightToRight_f);
	Cmd_AddCommand("moveLight_forward",			R_MoveLightForward_f);
	Cmd_AddCommand("moveLight_z",				R_MoveLightUpDown_f);
	Cmd_AddCommand("changeLightRadius",			R_ChangeLightRadius_f);
	Cmd_AddCommand("cloneLight",				R_Light_Clone_f);
	Cmd_AddCommand("clearWorldLights",          R_ClearWorldLights);
	Cmd_AddCommand("unselectLight",				R_Light_UnSelect_f);
	Cmd_AddCommand("editFlare",					R_FlareEdit_f);
	Cmd_AddCommand("resetFlarePos",				R_ResetFlarePos_f);
	Cmd_AddCommand("copy",						R_Copy_Light_Properties_f);
	Cmd_AddCommand("paste",						R_Paste_Light_Properties_f);
	Cmd_AddCommand("scaleLightColor",			R_ScaleLightColor_f);
	Cmd_AddCommand("vaoList",					R_VaoListing_f);
	Cmd_AddCommand("vboList",					R_VboListing_f);
	Cmd_AddCommand("fboList",					R_FboListing_f);
}

/*
==================
R_SetMode
==================
*/
bool R_SetMode(void)
{
	rserr_t err;
	const bool fullscreen = (bool)r_fullScreen->integer;

	r_fullScreen->modified = false;
	r_mode->modified = false;
 
    err = GLimp_SetMode(&vid.width, &vid.height, r_mode->integer, fullscreen);

    // success, update variables
	if (err == rserr_ok) {
        Cvar_SetValue("r_fullScreen", gl_state.fullscreen);
        r_fullScreen->modified = false;
		gl_state.prev_mode = r_mode->integer;
        return true;

    // try without fullscreen
	} else if (err == rserr_invalid_fullscreen) {
        Com_Printf(S_COLOR_RED "ref_xpgl::R_SetMode() - fullscreen unavailable in this mode\n");
        if ((err = GLimp_SetMode(&vid.width, &vid.height, r_mode->integer, false)) == rserr_ok) {
            Cvar_SetValue("r_fullScreen", 0);
            r_fullScreen->modified = false;
            gl_state.prev_mode = r_mode->integer;
            return true;
        }

    } else if (err == rserr_invalid_mode) {
        Com_Printf(S_COLOR_RED"ref_xpgl::R_SetMode() - invalid mode\n");
    }

    // revert to previous mode
    if (GLimp_SetMode(&vid.width, &vid.height, gl_state.prev_mode, false) == rserr_ok) {
        Cvar_SetValue("r_mode", gl_state.prev_mode);
        r_mode->modified = false;
        Cvar_SetValue("r_fullScreen", 0);
        r_fullScreen->modified = false;
        return true;
    } else {
        Com_Printf(S_COLOR_RED"ref_xpgl::R_SetMode() - could not revert to safe mode\n");
        return false;
    }
}

/*
===============
R_Init
===============
*/
bool IsExtensionSupported(const char *name)
{
	int			i;
	GLint		n = 0;
	const char	*extension;

	qglGetIntegerv(GL_NUM_EXTENSIONS, &n);
	
	for (i = 0; i<n; i++){
		extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
		if (!strcmp(name, extension))
			return true;		
	}
	return false;
}

int R_Init(void *hinstance, void *hWnd)
{
	int		max_aniso, max_texSize;
	float	aniso_level;

	Draw_GetPalette();
	R_RegisterCvars();

	// initialize our QGL dynamic bindings
	if (!QGL_Init()) {
		QGL_Shutdown();
		Com_Printf(S_COLOR_RED "quake2xp::R_Init() - could not load opengl32.dll");
		return -1;
	}
	// initialize OS-specific parts of OpenGL
	if (!GLimp_Init(hinstance, hWnd)) {
		QGL_Shutdown();
		return -1;
	}

	int flags;
	qglGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	gl_state.glDebugOutput = false;

	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {

		qglEnable(GL_DEBUG_OUTPUT);
		qglEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, NULL);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
		gl_state.glDebugOutput = true;
	}

	// set our "safe" modes
	gl_state.prev_mode = 0;

	Com_Printf("\n====" S_COLOR_YELLOW " Starting OpenGL Renderer" S_COLOR_WHITE " ====\n");

	// create the window and set up the context
	if (!R_SetMode()) {
		QGL_Shutdown();
		Com_Printf(S_COLOR_RED "ref_xpgl::R_Init() - could not R_SetMode()\n");
		return -1;
	}

	VID_MenuInit();

	// Get Extension string
	glGetStringi = (PFNGLGETSTRINGIPROC)qwglGetProcAddress("glGetStringi");

	// Separated Stencil 
	qglStencilFuncSeparate	= (PFNGLSTENCILFUNCSEPARATEPROC)	qwglGetProcAddress("glStencilFuncSeparate");
	qglStencilOpSeparate	= (PFNGLSTENCILOPSEPARATEPROC)		qwglGetProcAddress("glStencilOpSeparate");
	qglStencilMaskSeparate	= (PFNGLSTENCILMASKSEPARATEPROC)	qwglGetProcAddress("glStencilMaskSeparate");

	// debug context
	glDebugMessageControl	= (PFNGLDEBUGMESSAGECONTROLPROC)	qwglGetProcAddress("glDebugMessageControl");
	glDebugMessageInsert	= (PFNGLDEBUGMESSAGEINSERTPROC)		qwglGetProcAddress("glDebugMessageInsert");
	glDebugMessageCallback	= (PFNGLDEBUGMESSAGECALLBACKPROC)	qwglGetProcAddress("glDebugMessageCallback");
	glGetDebugMessageLog	= (PFNGLGETDEBUGMESSAGELOGPROC)		qwglGetProcAddress("glGetDebugMessageLog");
	
	// vao stuff
	glGenVertexArrays		= (PFNGLGENVERTEXARRAYSPROC)	qwglGetProcAddress("glGenVertexArrays");
	glDeleteVertexArrays	= (PFNGLDELETEVERTEXARRAYSPROC)	qwglGetProcAddress("glDeleteVertexArrays");
	glBindVertexArray		= (PFNGLBINDVERTEXARRAYPROC)	qwglGetProcAddress("glBindVertexArray");
	glIsVertexArray			= (PFNGLISVERTEXARRAYPROC)		qwglGetProcAddress("glIsVertexArray");

	glMultiDrawElements		= (PFNGLMULTIDRAWELEMENTSPROC)	qwglGetProcAddress("glMultiDrawElements");
	glMultiDrawArrays		= (PFNGLMULTIDRAWARRAYSPROC)	qwglGetProcAddress("glMultiDrawArrays");

	// vbo stuff
	qglBindBuffer				= (PFNGLBINDBUFFERPROC)				qwglGetProcAddress("glBindBuffer");
	qglDeleteBuffers			= (PFNGLDELETEBUFFERSPROC)			qwglGetProcAddress("glDeleteBuffers");
	qglGenBuffers				= (PFNGLGENBUFFERSPROC)				qwglGetProcAddress("glGenBuffers");
	qglBufferData				= (PFNGLBUFFERDATAPROC)				qwglGetProcAddress("glBufferData");
	qglBufferSubData			= (PFNGLBUFFERSUBDATAPROC)			qwglGetProcAddress("glBufferSubData");
	qglMapBuffer				= (PFNGLMAPBUFFERPROC)				qwglGetProcAddress("glMapBuffer");
	qglUnmapBuffer				= (PFNGLUNMAPBUFFERPROC)			qwglGetProcAddress("glUnmapBuffer");
	qglMapBufferRange			= (PFNGLMAPBUFFERRANGEPROC)			qwglGetProcAddress("glMapBufferRange");
	qglInvalidateBufferData		= (PFNGLINVALIDATEBUFFERDATAPROC)	qwglGetProcAddress("glInvalidateBufferData");
	qglInvalidateBufferSubData	= (PFNGLINVALIDATEBUFFERSUBDATAPROC)qwglGetProcAddress("glInvalidateBufferSubData");

	// fbo stuff
	qglIsRenderbuffer						= (PFNGLISRENDERBUFFERPROC)						qwglGetProcAddress("glIsRenderbuffer");
	qglBindRenderbuffer						= (PFNGLBINDRENDERBUFFERPROC)					qwglGetProcAddress("glBindRenderbuffer");
	qglDeleteRenderbuffers					= (PFNGLDELETERENDERBUFFERSPROC)				qwglGetProcAddress("glDeleteRenderbuffers");
	qglGenRenderbuffers						= (PFNGLGENRENDERBUFFERSPROC)					qwglGetProcAddress("glGenRenderbuffers");
	qglRenderbufferStorage					= (PFNGLRENDERBUFFERSTORAGEPROC)				qwglGetProcAddress("glRenderbufferStorage");
	qglGetRenderbufferParameteriv			= (PFNGLGETRENDERBUFFERPARAMETERIVPROC)			qwglGetProcAddress("glGetRenderbufferParameteriv");
	qglRenderbufferStorageMultisample		= (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)		qwglGetProcAddress("glRenderbufferStorageMultisample");

	qglIsFramebuffer						= (PFNGLISFRAMEBUFFERPROC)						qwglGetProcAddress("glIsFramebuffer");
	qglBindFramebuffer						= (PFNGLBINDFRAMEBUFFERPROC)					qwglGetProcAddress("glBindFramebuffer");
	qglDeleteFramebuffers					= (PFNGLDELETEFRAMEBUFFERSPROC)					qwglGetProcAddress("glDeleteFramebuffers");
	qglGenFramebuffers						= (PFNGLGENFRAMEBUFFERSPROC)					qwglGetProcAddress("glGenFramebuffers");
	qglCheckFramebufferStatus				= (PFNGLCHECKFRAMEBUFFERSTATUSPROC)				qwglGetProcAddress("glCheckFramebufferStatus");
	qglFramebufferTexture1D					= (PFNGLFRAMEBUFFERTEXTURE1DPROC)				qwglGetProcAddress("glFramebufferTexture1D");
	qglFramebufferTexture2D					= (PFNGLFRAMEBUFFERTEXTURE2DPROC)				qwglGetProcAddress("glFramebufferTexture2D");
	qglFramebufferTexture3D					= (PFNGLFRAMEBUFFERTEXTURE3DPROC)				qwglGetProcAddress("glFramebufferTexture3D");
	qglFramebufferRenderbuffer				= (PFNGLFRAMEBUFFERRENDERBUFFERPROC)			qwglGetProcAddress("glFramebufferRenderbuffer");
	qglGenerateMipmap						= (PFNGLGENERATEMIPMAPPROC)						qwglGetProcAddress("glGenerateMipmap");
	qglBlitFramebuffer						= (PFNGLBLITFRAMEBUFFERPROC)					qwglGetProcAddress("glBlitFramebuffer");
	qglGetFramebufferAttachmentParameteriv	= (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)qwglGetProcAddress("glGetFramebufferAttachmentParameteriv");
	qglDrawBuffers							= (PFNGLDRAWBUFFERSPROC)						qwglGetProcAddress("glDrawBuffers");
	glCopyImageSubData						= (PFNGLCOPYIMAGESUBDATAPROC)					qwglGetProcAddress("glCopyImageSubData");
	qglClearBufferfv						= (PFNGLCLEARBUFFERFVPROC)						qwglGetProcAddress("glClearBufferfv");
	qglClearBufferfi						= (PFNGLCLEARBUFFERFIPROC)						qwglGetProcAddress("glClearBufferfi");
	qglClearBufferiv						= (PFNGLCLEARBUFFERIVPROC)						qwglGetProcAddress("glClearBufferiv");
	
	// fbo dsa
	qglCreateRenderbuffers					= (PFNGLCREATERENDERBUFFERSPROC)				qwglGetProcAddress("glCreateRenderbuffers");
	qglNamedRenderbufferStorageMultisample	= (PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC)qwglGetProcAddress("glNamedRenderbufferStorageMultisample");
	qglNamedFramebufferRenderbuffer			= (PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC)		qwglGetProcAddress("glNamedFramebufferRenderbuffer");
	qglNamedRenderbufferStorage				= (PFNGLNAMEDRENDERBUFFERSTORAGEPROC)			qwglGetProcAddress("glNamedRenderbufferStorage");
		
	qglCreateFramebuffers					= (PFNGLCREATEFRAMEBUFFERSPROC)					qwglGetProcAddress("glCreateFramebuffers");
	qglNamedFramebufferTexture				= (PFNGLNAMEDFRAMEBUFFERTEXTUREPROC)			qwglGetProcAddress("glNamedFramebufferTexture");
	qglCheckNamedFramebufferStatus			= (PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC)		qwglGetProcAddress("glCheckNamedFramebufferStatus");
	qglNamedFramebufferDrawBuffer			= (PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC)			qwglGetProcAddress("glNamedFramebufferDrawBuffer");
	qglNamedFramebufferDrawBuffers			= (PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC)		qwglGetProcAddress("glNamedFramebufferDrawBuffers");
	qglBlitNamedFramebuffer					= (PFNGLBLITNAMEDFRAMEBUFFERPROC)				qwglGetProcAddress("glBlitNamedFramebuffer");
	
	qglClearNamedFramebufferiv				= (PFNGLCLEARNAMEDFRAMEBUFFERIVPROC)			qwglGetProcAddress("glClearNamedFramebufferiv");
	qglClearNamedFramebufferfv				= (PFNGLCLEARNAMEDFRAMEBUFFERFVPROC)			qwglGetProcAddress("glClearNamedFramebufferfv");
	qglClearNamedFramebufferfi				= (PFNGLCLEARNAMEDFRAMEBUFFERFIPROC)			qwglGetProcAddress("glClearNamedFramebufferfi");


	// bindless textures stuff
	glGetTextureHandleARB				= (PFNGLGETTEXTUREHANDLEARBPROC)			qwglGetProcAddress("glGetTextureHandleARB");
	glGetTextureSamplerHandleARB		= (PFNGLGETTEXTURESAMPLERHANDLEARBPROC)		qwglGetProcAddress("glGetTextureSamplerHandleARB");
	glMakeTextureHandleResidentARB		= (PFNGLMAKETEXTUREHANDLERESIDENTARBPROC)	qwglGetProcAddress("glMakeTextureHandleResidentARB");
	glMakeTextureHandleNonResidentARB	= (PFNGLMAKETEXTUREHANDLERESIDENTARBPROC)	qwglGetProcAddress("glMakeTextureHandleNonResidentARB");
	glGetImageHandleARB					= (PFNGLGETIMAGEHANDLEARBPROC)				qwglGetProcAddress("glGetImageHandleARB");
	glMakeImageHandleResidentARB		= (PFNGLMAKEIMAGEHANDLERESIDENTARBPROC)		qwglGetProcAddress("glMakeImageHandleResidentARB");
	glMakeImageHandleNonResidentARB		= (PFNGLMAKEIMAGEHANDLENONRESIDENTARBPROC)	qwglGetProcAddress("glMakeImageHandleNonResidentARB");
	glUniformHandleui64ARB				= (PFNGLUNIFORMHANDLEUI64ARBPROC)			qwglGetProcAddress("glUniformHandleui64ARB");
	glUniformHandleui64vARB				= (PFNGLUNIFORMHANDLEUI64VARBPROC)			qwglGetProcAddress("glUniformHandleui64vARB");
	glProgramUniformHandleui64ARB		= (PFNGLPROGRAMUNIFORMHANDLEUI64ARBPROC)	qwglGetProcAddress("glProgramUniformHandleui64ARB");
	glProgramUniformHandleui64vARB		= (PFNGLPROGRAMUNIFORMHANDLEUI64VARBPROC)	qwglGetProcAddress("glProgramUniformHandleui64vARB");
	glIsTextureHandleResidentARB		= (PFNGLISTEXTUREHANDLERESIDENTARBPROC)		qwglGetProcAddress("glIsTextureHandleResidentARB");
	glIsImageHandleResidentARB			= (PFNGLISIMAGEHANDLERESIDENTARBPROC)		qwglGetProcAddress("glIsImageHandleResidentARB");
	glVertexAttribL1ui64ARB				= (PFNGLVERTEXATTRIBL1UI64ARBPROC)			qwglGetProcAddress("glVertexAttribL1ui64ARB");
	glVertexAttribL1ui64vARB			= (PFNGLVERTEXATTRIBL1UI64VARBPROC)			qwglGetProcAddress("glVertexAttribL1ui64vARB");
	glGetVertexAttribLui64vARB			= (PFNGLGETVERTEXATTRIBLUI64VARBPROC)		qwglGetProcAddress("glGetVertexAttribLui64vARB");
	
	if (!glGetTextureHandleARB && !glMakeTextureHandleResidentARB && !glMakeTextureHandleNonResidentARB && !glUniformHandleui64ARB) {
		Com_Printf("Current video card/driver combination does not support GL_ARB_bindless_texture\n");
		VID_Error(ERR_FATAL, "Current video card/driver combination does not support GL_ARB_bindless_texture\n");
	}

	// glsl stuff
	qglCreateShader =				(PFNGLCREATESHADERPROC)				qwglGetProcAddress("glCreateShader");
	qglCreateProgram =				(PFNGLCREATEPROGRAMPROC)			qwglGetProcAddress("glCreateProgram");
	qglDeleteShader =				(PFNGLDELETESHADERPROC)				qwglGetProcAddress("glDeleteShader");
	qglDeleteProgram =				(PFNGLDELETEPROGRAMPROC)			qwglGetProcAddress("glDeleteProgram");
	qglGetShaderiv =				(PFNGLGETSHADERIVPROC)				qwglGetProcAddress("glGetShaderiv");
	qglGetProgramiv =				(PFNGLGETPROGRAMIVPROC)				qwglGetProcAddress("glGetProgramiv");
	qglGetShaderInfoLog =			(PFNGLGETSHADERINFOLOGPROC)			qwglGetProcAddress("glGetShaderInfoLog");
	qglGetProgramInfoLog =			(PFNGLGETPROGRAMINFOLOGPROC)		qwglGetProcAddress("glGetProgramInfoLog");
	qglShaderSource =				(PFNGLSHADERSOURCEPROC)				qwglGetProcAddress("glShaderSource");
	qglCompileShader =				(PFNGLCOMPILESHADERPROC)			qwglGetProcAddress("glCompileShader");
	qglAttachShader =				(PFNGLATTACHSHADERPROC)				qwglGetProcAddress("glAttachShader");
	qglDetachShader =				(PFNGLDETACHSHADERPROC)				qwglGetProcAddress("glDetachShader");
	qglLinkProgram =				(PFNGLLINKPROGRAMPROC)				qwglGetProcAddress("glLinkProgram");
	qglUseProgram =					(PFNGLUSEPROGRAMPROC)				qwglGetProcAddress("glUseProgram");
	qglVertexAttribPointer =		(PFNGLVERTEXATTRIBPOINTERPROC)		qwglGetProcAddress("glVertexAttribPointer");
	qglEnableVertexAttribArray =	(PFNGLENABLEVERTEXATTRIBARRAYPROC)	qwglGetProcAddress("glEnableVertexAttribArray");
	qglDisableVertexAttribArray =	(PFNGLDISABLEVERTEXATTRIBARRAYPROC)	qwglGetProcAddress("glDisableVertexAttribArray");
	qglBindAttribLocation =			(PFNGLBINDATTRIBLOCATIONPROC)		qwglGetProcAddress("glBindAttribLocation");
	qglGetAttribLocation =			(PFNGLGETATTRIBLOCATIONPROC)		qwglGetProcAddress("glGetAttribLocation");
	qglGetActiveUniform =			(PFNGLGETACTIVEUNIFORMPROC)			qwglGetProcAddress("glGetActiveUniform");
	qglGetUniformLocation =			(PFNGLGETUNIFORMLOCATIONPROC)		qwglGetProcAddress("glGetUniformLocation");
	qglUniform1f =					(PFNGLUNIFORM1FPROC)				qwglGetProcAddress("glUniform1f");
	qglUniform2f =					(PFNGLUNIFORM2FPROC)				qwglGetProcAddress("glUniform2f");
	qglUniform3f =					(PFNGLUNIFORM3FPROC)				qwglGetProcAddress("glUniform3f");
	qglUniform4f =					(PFNGLUNIFORM4FPROC)				qwglGetProcAddress("glUniform4f");
	qglUniform1i =					(PFNGLUNIFORM1IPROC)				qwglGetProcAddress("glUniform1i");
	qglUniform2i =					(PFNGLUNIFORM2IPROC)				qwglGetProcAddress("glUniform2i");
	qglUniform3i =					(PFNGLUNIFORM3IPROC)				qwglGetProcAddress("glUniform3i");
	qglUniform4i =					(PFNGLUNIFORM4IPROC)				qwglGetProcAddress("glUniform4i");
	qglUniform1fv =					(PFNGLUNIFORM1FVPROC)				qwglGetProcAddress("glUniform1fv");
	qglUniform2fv =					(PFNGLUNIFORM2FVPROC)				qwglGetProcAddress("glUniform2fv");
	qglUniform3fv =					(PFNGLUNIFORM3FVPROC)				qwglGetProcAddress("glUniform3fv");
	qglUniform4fv =					(PFNGLUNIFORM4FVPROC)				qwglGetProcAddress("glUniform4fv");
	qglUniform1iv =					(PFNGLUNIFORM1IVPROC)				qwglGetProcAddress("glUniform1iv");
	qglUniform2iv =					(PFNGLUNIFORM2IVPROC)				qwglGetProcAddress("glUniform2iv");
	qglUniform3iv =					(PFNGLUNIFORM3IVPROC)				qwglGetProcAddress("glUniform3iv");
	qglUniform4iv =					(PFNGLUNIFORM4IVPROC)				qwglGetProcAddress("glUniform4iv");
	qglUniformMatrix2fv =			(PFNGLUNIFORMMATRIX2FVPROC)			qwglGetProcAddress("glUniformMatrix2fv");
	qglUniformMatrix3fv =			(PFNGLUNIFORMMATRIX3FVPROC)         qwglGetProcAddress("glUniformMatrix3fv");
	qglUniformMatrix4fv =			(PFNGLUNIFORMMATRIX4FVPROC)			qwglGetProcAddress("glUniformMatrix4fv");

	// DSA stuff glsl block
	glProgramUniform1f = (PFNGLPROGRAMUNIFORM1FPROC)					qwglGetProcAddress("glProgramUniform1f");
	glProgramUniform2f = (PFNGLPROGRAMUNIFORM2FPROC)					qwglGetProcAddress("glProgramUniform2f");
	glProgramUniform3f = (PFNGLPROGRAMUNIFORM3FPROC)					qwglGetProcAddress("glProgramUniform3f");
	glProgramUniform4f = (PFNGLPROGRAMUNIFORM4FPROC)					qwglGetProcAddress("glProgramUniform4f");
	glProgramUniform1i = (PFNGLPROGRAMUNIFORM1IPROC)					qwglGetProcAddress("glProgramUniform1i");
	glProgramUniform2i = (PFNGLPROGRAMUNIFORM2IPROC)					qwglGetProcAddress("glProgramUniform2i");
	glProgramUniform3i = (PFNGLPROGRAMUNIFORM3IPROC)					qwglGetProcAddress("glProgramUniform3i");
	glProgramUniform4i = (PFNGLPROGRAMUNIFORM4IPROC)					qwglGetProcAddress("glProgramUniform4i");

	glProgramUniform1fv = (PFNGLPROGRAMUNIFORM1FVPROC)					qwglGetProcAddress("glProgramUniform1fv");
	glProgramUniform2fv = (PFNGLPROGRAMUNIFORM2FVPROC)					qwglGetProcAddress("glProgramUniform2fv");
	glProgramUniform3fv = (PFNGLPROGRAMUNIFORM3FVPROC)					qwglGetProcAddress("glProgramUniform3fv");
	glProgramUniform4fv = (PFNGLPROGRAMUNIFORM4FVPROC)					qwglGetProcAddress("glProgramUniform4fv");
	glProgramUniform1iv = (PFNGLPROGRAMUNIFORM1IVPROC)					qwglGetProcAddress("glProgramUniform1iv");
	glProgramUniform2iv = (PFNGLPROGRAMUNIFORM2IVPROC)					qwglGetProcAddress("glProgramUniform2iv");
	glProgramUniform3iv = (PFNGLPROGRAMUNIFORM3IVPROC)					qwglGetProcAddress("glProgramUniform3iv");
	glProgramUniform4iv = (PFNGLPROGRAMUNIFORM4IVPROC)					qwglGetProcAddress("glProgramUniform4iv");

	glProgramUniformMatrix2fv =		(PFNGLPROGRAMUNIFORMMATRIX2FVPROC)	qwglGetProcAddress("glProgramUniformMatrix2fv");
	glProgramUniformMatrix3fv =		(PFNGLPROGRAMUNIFORMMATRIX3FVPROC)	qwglGetProcAddress("glProgramUniformMatrix3fv");
	glProgramUniformMatrix4fv =		(PFNGLPROGRAMUNIFORMMATRIX4FVPROC)	qwglGetProcAddress("glProgramUniformMatrix4fv");

	glProgramUniformMatrix2x3fv =	(PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC)	qwglGetProcAddress("glProgramUniformMatrix2x3fv");
	glProgramUniformMatrix3x2fv =	(PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC)	qwglGetProcAddress("glProgramUniformMatrix3x2fv");
	glProgramUniformMatrix2x4fv =	(PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC)	qwglGetProcAddress("glProgramUniformMatrix2x4fv");
	glProgramUniformMatrix4x2fv =	(PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC)	qwglGetProcAddress("glProgramUniformMatrix4x2fv");
	glProgramUniformMatrix3x4fv =	(PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC)	qwglGetProcAddress("glProgramUniformMatrix3x4fv");
	glProgramUniformMatrix4x3fv =	(PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC)	qwglGetProcAddress("glProgramUniformMatrix4x3fv");

	qglTextureView	=	(PFNGLTEXTUREVIEWPROC)	qwglGetProcAddress("glTextureView");
	qglTexImage3D	=	(PFNGLTEXIMAGE3DPROC)	qwglGetProcAddress("glTexImage3D");

	// Textures DSA 
	glBindTextures			=		(PFNGLBINDTEXTURESPROC)			qwglGetProcAddress("glBindTextures");
	glBindTextureUnit		=		(PFNGLBINDTEXTUREUNITPROC)		qwglGetProcAddress("glBindTextureUnit");
	glCreateTextures		=		(PFNGLCREATETEXTURESPROC)		qwglGetProcAddress("glCreateTextures");
	glTextureStorage2D		=		(PFNGLTEXTURESTORAGE2DPROC)		qwglGetProcAddress("glTextureStorage2D");
	glTextureSubImage2D		=		(PFNGLTEXTURESUBIMAGE2DPROC)	qwglGetProcAddress("glTextureSubImage2D");
	glTextureParameteri		=		(PFNGLTEXTUREPARAMETERIPROC)	qwglGetProcAddress("glTextureParameteri");
	glTextureParameterf		=		(PFNGLTEXTUREPARAMETERFPROC)	qwglGetProcAddress("glTextureParameterf");
	glCopyTextureSubImage2D =		(PFNGLCOPYTEXTURESUBIMAGE2DPROC)qwglGetProcAddress("glCopyTextureSubImage2D");
	glGenerateTextureMipmap =		(PFNGLGENERATETEXTUREMIPMAPPROC)qwglGetProcAddress("glGenerateTextureMipmap");
	glTextureStorage3D		=		(PFNGLTEXTURESTORAGE3DPROC)		qwglGetProcAddress("glTextureStorage3D");
	glTextureSubImage3D		=		(PFNGLTEXTURESUBIMAGE3DPROC)	qwglGetProcAddress("glTextureSubImage3D");
	glGetTextureImage		=		(PFNGLGETTEXTUREIMAGEPROC)		qwglGetProcAddress("glGetTextureImage");

	glCompressedTextureSubImage2D =	(PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC)	qwglGetProcAddress("glCompressedTextureSubImage2D");
	glCompressedTextureSubImage3D = (PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC)	qwglGetProcAddress("glCompressedTextureSubImage3D");
	qglCompressedTexSubImage2D =	(PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC)		qwglGetProcAddress("glCompressedTexSubImage2D");
	qglTexSubImage2D =				(PFNGLTEXSUBIMAGE2DPROC)				qwglGetProcAddress("glTexSubImage2D");
	glGetTextureLevelParameteriv =	(PFNGLGETTEXTURELEVELPARAMETERIVPROC)	qwglGetProcAddress("glGetTextureLevelParameteriv");

	// texture storage
	glTexStorage2D		=		(PFNGLTEXSTORAGE2DPROC)			qwglGetProcAddress("glTexStorage2D");
	glTexStorage3D		=		(PFNGLTEXSTORAGE3DPROC)			qwglGetProcAddress("glTexStorage3D");
	qglTexSubImage3D	=		(PFNGLTEXSUBIMAGE3DPROC)		qwglGetProcAddress("glTexSubImage3D");

	glGenQueries		= (PFNGLGENQUERIESPROC)			qwglGetProcAddress("glGenQueries");
	glDeleteQueries		= (PFNGLDELETEQUERIESPROC)		qwglGetProcAddress("glDeleteQueries");
	glIsQuery			= (PFNGLISQUERYPROC)			qwglGetProcAddress("glIsQuery");
	glBeginQuery		= (PFNGLBEGINQUERYPROC)			qwglGetProcAddress("glBeginQuery");
	glEndQuery			= (PFNGLENDQUERYPROC)			qwglGetProcAddress("glEndQuery");
	glGetQueryiv		= (PFNGLGETQUERYIVPROC)			qwglGetProcAddress("glGetQueryiv");
	glGetQueryObjectiv	= (PFNGLGETQUERYOBJECTIVPROC)	qwglGetProcAddress("glGetQueryObjectiv");
	glGetQueryObjectuiv	= (PFNGLGETQUERYOBJECTUIVPROC)	qwglGetProcAddress("glGetQueryObjectuiv");

	glGetProgramBinary	=	(PFNGLGETPROGRAMBINARYPROC)		qwglGetProcAddress("glGetProgramBinary");
	glProgramBinary		=	(PFNGLPROGRAMBINARYPROC)		qwglGetProcAddress("glProgramBinary");
	glProgramParameteri =	(PFNGLPROGRAMPARAMETERIPROC)	qwglGetProcAddress("glProgramParameteri");

	qglClampColor		=	(PFNGLCLAMPCOLORPROC)		qwglGetProcAddress("glClampColor");
	qglClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);

	glFenceSync = (PFNGLFENCESYNCPROC)	qwglGetProcAddress("glFenceSync");
	glGetSynciv = (PFNGLGETSYNCIVPROC)	qwglGetProcAddress("glGetSynciv");

	qglObjectLabel =	(PFNGLOBJECTLABELPROC)		qwglGetProcAddress("glObjectLabel");
	qglGetObjectLabel = (PFNGLGETOBJECTLABELPROC)	qwglGetProcAddress("glGetObjectLabel");

	// vbo dsa
	qglCreateBuffers		= (PFNGLCREATEBUFFERSPROC)		qwglGetProcAddress("glCreateBuffers");
	qglNamedBufferStorage	= (PFNGLNAMEDBUFFERSTORAGEPROC)	qwglGetProcAddress("glNamedBufferStorage");
	qglNamedBufferData		= (PFNGLNAMEDBUFFERDATAPROC)	qwglGetProcAddress("glNamedBufferData");
	qglNamedBufferSubData	= (PFNGLNAMEDBUFFERSUBDATAPROC)	qwglGetProcAddress("glNamedBufferSubData");

	qglGetTextureSubImage	= (PFNGLGETTEXTURESUBIMAGEPROC)	qwglGetProcAddress("glGetTextureSubImage");

	qglGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS,	&gl_state.numFormats);
	qglGetIntegerv(GL_PROGRAM_BINARY_FORMATS,		&gl_state.binaryFormats);

	gl_config.vendor_string					= (const char*)qglGetString(GL_VENDOR);
	gl_config.renderer_string				= (const char*)qglGetString(GL_RENDERER);
	gl_config.version_string				= (const char*)qglGetString(GL_VERSION);
	gl_config.shadingLanguageVersionString	= (const char*)qglGetString(GL_SHADING_LANGUAGE_VERSION);

	qglGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK, GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE,		&gl_config.glFbCB[0]);
	qglGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK, GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE,	&gl_config.glFbCB[1]);
	qglGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK, GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE,	&gl_config.glFbCB[2]);
	qglGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK, GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE,	&gl_config.glFbCB[3]);

	Com_Printf("GL_VENDOR:" S_COLOR_GREEN "    %s\n", gl_config.vendor_string);
	Com_Printf("GL_RENDERER:" S_COLOR_GREEN "  %s\n", gl_config.renderer_string);
	Com_Printf("GL_VERSION:" S_COLOR_GREEN "   %s\n", gl_config.version_string);
	Com_Printf("GLSL_VERSION:" S_COLOR_GREEN " %s\n", gl_config.shadingLanguageVersionString);
	Com_Printf("GL_FRAMEBUFFER Color Bits: " S_COLOR_YELLOW "R:" S_COLOR_GREEN "%i " S_COLOR_YELLOW "G:" S_COLOR_GREEN "%i " S_COLOR_YELLOW "B:" S_COLOR_GREEN "%i " S_COLOR_YELLOW "A:" S_COLOR_GREEN "%i\n", gl_config.glFbCB[0], gl_config.glFbCB[1], gl_config.glFbCB[2], gl_config.glFbCB[3]);
	
	qglGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,	&gl_config.maxFragmentUniformComponents);
	qglGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS,	&gl_config.maxVertexUniformComponents);
	qglGetIntegerv(GL_MAX_VARYING_FLOATS,				&gl_config.maxVaryingFloats);
	qglGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,	&gl_config.maxVertexTextureImageUnits);
	qglGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &gl_config.maxCombinedTextureImageUnits);
	qglGetIntegerv(GL_MAX_VERTEX_ATTRIBS,				&gl_config.maxVertexAttribs);
	qglGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,			&gl_config.maxTextureImageUnits);
	qglGetIntegerv(GL_MAX_UNIFORM_LOCATIONS,			&gl_config.maxUniformLocations);
	qglGetIntegerv(GL_MAX_RENDERBUFFER_SIZE,			&gl_state.maxRenderBufferSize);
	qglGetIntegerv(GL_MAX_DRAW_BUFFERS,					&gl_state.maxDrawBuffers);
	qglGetIntegerv(GL_MAX_COLOR_ATTACHMENTS,			&gl_state.maxColorAttachments);
	qglGetIntegerv(GL_MAX_SAMPLES,						&gl_state.maxSamples);
	qglGetIntegerv(GL_MAX_PATCH_VERTICES,				&gl_config.maxPatchVertices);

	Com_Printf("\n");
	Com_Printf(S_COLOR_YELLOW"Max Fragment Uniform Components:" S_COLOR_GREEN " %i\n", gl_config.maxFragmentUniformComponents);
	Com_Printf(S_COLOR_YELLOW"Max Vertex Uniform Components:  " S_COLOR_GREEN " %i\n", gl_config.maxVertexUniformComponents);
	Com_Printf(S_COLOR_YELLOW"Max Uniform Locations:          " S_COLOR_GREEN " %i\n", gl_config.maxUniformLocations);
	Com_Printf(S_COLOR_YELLOW"Max Vertex Attribs:             " S_COLOR_GREEN " %i\n", gl_config.maxVertexAttribs);
	Com_Printf(S_COLOR_YELLOW"Max Varying Floats:             " S_COLOR_GREEN " %i\n", gl_config.maxVaryingFloats);
	Com_Printf(S_COLOR_YELLOW"Max Vertex TextureImageUnits:   " S_COLOR_GREEN " %i\n", gl_config.maxVertexTextureImageUnits);
	Com_Printf(S_COLOR_YELLOW"Max Texture ImageUnits:         " S_COLOR_GREEN " %i\n", gl_config.maxTextureImageUnits);
	Com_Printf(S_COLOR_YELLOW"Max Combined TextureImageUnits: " S_COLOR_GREEN " %i\n", gl_config.maxCombinedTextureImageUnits);
	Com_Printf(S_COLOR_YELLOW"Max Patch Vertices:             " S_COLOR_GREEN " %i\n", gl_config.maxPatchVertices);
	Com_Printf("\n");
	Com_Printf(S_COLOR_YELLOW"Max Render Buffer Size:   " S_COLOR_GREEN "       %i\n", gl_state.maxRenderBufferSize);
	Com_Printf(S_COLOR_YELLOW"Max Draw Buffers:         " S_COLOR_GREEN "       %i\n", gl_state.maxDrawBuffers);
	Com_Printf(S_COLOR_YELLOW"Max Color Attachments:    " S_COLOR_GREEN "       %i\n", gl_state.maxColorAttachments);
	Com_Printf(S_COLOR_YELLOW"Max Buffer Samples:       " S_COLOR_GREEN "       %i\n", gl_state.maxSamples);


	qglGetIntegeri_v =		(PFNGLGETINTEGERI_VPROC)	qwglGetProcAddress("glGetIntegeri_v");
	qglMemoryBarrier =		(PFNGLMEMORYBARRIERPROC)	qwglGetProcAddress("glMemoryBarrier");
	qglBindImageTexture =	(PFNGLBINDIMAGETEXTUREPROC)	qwglGetProcAddress("glBindImageTexture");
	qglDispatchCompute =	(PFNGLDISPATCHCOMPUTEPROC)	qwglGetProcAddress("glDispatchCompute");

	int maxX, maxY, maxZ, maxItemsPerGroup;
	qglGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &maxX);
	qglGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &maxY);
	qglGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &maxZ);
	qglGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxItemsPerGroup);
	
	int maxGroupX, maxGroupY, maxGroupZ;
	qglGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxGroupX);
	qglGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &maxGroupY);
	qglGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &maxGroupZ);

	int maxSharedSize;
	qglGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &maxSharedSize);

	Com_Printf("\n");
	Com_Printf(S_COLOR_YELLOW"Max Compute Work Group Size:" S_COLOR_GREEN "        %i %i %i\n", maxX, maxY, maxZ);
	Com_Printf(S_COLOR_YELLOW"Max Compute Work Group Invocations:" S_COLOR_GREEN " %i\n", maxItemsPerGroup);
	Com_Printf(S_COLOR_YELLOW"Max Compute Work Group Count:" S_COLOR_GREEN "       %i %i %i\n", maxGroupX, maxGroupY, maxGroupZ);
	Com_Printf(S_COLOR_YELLOW"Max Compute Shared Memory Size:" S_COLOR_GREEN "     %i\n", maxSharedSize);
	
	GL_SetDefaultState();

	R_InitPrograms();
	R_InitFboBuffers();
	R_InitVertexBuffers();
	GL_InitImages();
	Mod_Init();
	R_InitEngineTextures();
	R_Init2D();

	qglGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texSize);
	Com_Printf("...Max Texture Size is ["S_COLOR_GREEN"%i"S_COLOR_WHITE"]\n", max_texSize);

	qglGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &max_aniso);

	aniso_level = r_textureAnisotropy->value;
	if (r_textureAnisotropy->value <= 1.0) {
		r_textureAnisotropy = Cvar_Set("r_anisotropic", "1.0");
		Com_Printf(S_COLOR_YELLOW"...ignoring GL_ARB_texture_filter_anisotropic\n");
	}
	else {
		Com_Printf("...using GL_ARB_texture_filter_anisotropic ["S_COLOR_GREEN"%i"S_COLOR_WHITE" max] ["S_COLOR_GREEN"%i" S_COLOR_WHITE" selected]\n",
			max_aniso, (int)aniso_level);
	}

	if (IsExtensionSupported("GL_ARB_seamless_cube_map")) {
		Com_Printf("...using GL_ARB_seamless_cube_map\n");
		qglEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	}

	// ===========================================================================================================================

	gl_state.depthBoundsTest = false;
	if (IsExtensionSupported("GL_EXT_depth_bounds_test")) {
		Com_Printf("...using GL_EXT_depth_bounds_test\n");

		glDepthBoundsEXT = (PFNGLDEPTHBOUNDSEXTPROC)qwglGetProcAddress("glDepthBoundsEXT");
		gl_state.depthBoundsTest = true;
	}
	else {
		Com_Printf(S_COLOR_RED"...GL_EXT_depth_bounds_test not found\n");
		gl_state.depthBoundsTest = false;
	}


	Com_Printf("=====================================\n");

	flareEdit = (bool)false;
	return 0;
}



/*
===============
R_Shutdown
===============
*/

void R_Shutdown(void)
{
	Cmd_RemoveCommand("modellist");
	Cmd_RemoveCommand("screenshot");
	Cmd_RemoveCommand("levelshot");
	Cmd_RemoveCommand("imagelist");
	Cmd_RemoveCommand("autoLightsStats");
	Cmd_RemoveCommand("dumpEntityString");
	Cmd_RemoveCommand("gpuInfo");
	
	Cmd_RemoveCommand("saveLights");
	Cmd_RemoveCommand("spawnLight");
	Cmd_RemoveCommand("removeLight");
	Cmd_RemoveCommand("editLight");
	Cmd_RemoveCommand("spawnLightToCamera");
	Cmd_RemoveCommand("changeLightRadius");
	Cmd_RemoveCommand("cloneLight");
	Cmd_RemoveCommand("clearWorldLights");
	Cmd_RemoveCommand("unselectLight");
	Cmd_RemoveCommand("editFlare");
	Cmd_RemoveCommand("resetFlarePos");
	Cmd_RemoveCommand("copy");
	Cmd_RemoveCommand("paste");
	Cmd_RemoveCommand("moveLight_right");
	Cmd_RemoveCommand("moveLight_forward");
	Cmd_RemoveCommand("moveLight_z");
	Cmd_RemoveCommand("scaleLightColor");

	Cmd_RemoveCommand("glsl");
	Cmd_RemoveCommand("glslInfo");
	Cmd_RemoveCommand("openglInfo");

//	Cmd_RemoveCommand("makeLut");

	Cmd_RemoveCommand("fogEdit");
#ifdef _WIN32
	Cmd_RemoveCommand("gpuInfo");
#endif
	
	Cmd_RemoveCommand("vaoList");
	Cmd_RemoveCommand("vboList");
	Cmd_RemoveCommand("fboList");

	// free pbo's
	qglDeleteBuffers(1, &pbo._fullScreen);
	qglDeleteBuffers(1, &pbo._fullScreenF);

	R_ShotdownFBO();
	R_ShutDownVertexBuffers();

	Mod_FreeAll();
	GL_ShutdownImages();

	R_ClearWorldLights();
	R_ShutdownPrograms();

	GLimp_Shutdown();
	QGL_Shutdown();
}


/*
@@@@@@@@@@@@@@@@@@@@@
R_BeginFrame
@@@@@@@@@@@@@@@@@@@@@
*/
void R_BeginFrame()
{
	
#ifndef _WIN32
    // there is no need to restart video mode with SDL
    if (r_fullScreen->modified) {
        R_SetMode();
		r_fullScreen->modified = false;
    }
#endif

	/* 
	 ** change modes if necessary
	 */
	r_lightmapScale->value			= ClampCvar(0.0, 1.0,			r_lightmapScale->value);
	r_parallaxMapping->integer		= ClampCvarInteger(0, 3,		r_parallaxMapping->integer);
	r_parallaxScale->integer		= ClampCvarInteger(0, 6,		r_parallaxScale->integer);
	r_colorTempK->integer			= ClampCvarInteger(1000, 40000, r_colorTempK->integer);
	r_hdrUiNits->value				= ClampCvar(100.0, 1000.0,		r_hdrUiNits->value);
	r_hdrLensFlaresIntens->value	= ClampCvar(0.1, 1.0,			r_hdrLensFlaresIntens->value);
	r_penumbraSize->value			= ClampCvar(2.0, 16.0,			r_penumbraSize->value);

	if (r_mode->modified || r_fullScreen->modified)
        vid_ref->modified = true;

	if (r_selfShadowingParallax->modified)
		r_selfShadowingParallax->modified = false;

	if(r_dof->modified)
		r_dof->modified = false;

	if(r_lightmapScale->modified)
		r_lightmapScale->modified = false;

	if (r_ssao->modified)
		r_ssao->modified = false;
	
	if (r_parallaxMapping->modified)
		r_parallaxMapping->modified = false;

	if (r_textureAnisotropy->modified)
		r_textureAnisotropy->modified = false;

	if (r_textureLodBias->modified)
		r_textureLodBias->modified = false;

	//go into 2D mode
	R_SetupOrthoMatrix();

	GL_Enable(GL_BLEND); // alpha blend for chars
	GL_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if(r_nsightDebug->integer)
		qglFlush();

	GL_UpdateSwapInterval();

	gldepthmin = 0.0;
	gldepthmax = 1.0;
	GL_DepthFunc(GL_LEQUAL);
	GL_DepthRange(gldepthmin, gldepthmax);
	qglClearBufferfv(GL_COLOR, 0, clearColor);
}

/*
=============
R_SetPalette
=============
*/
unsigned r_rawpalette[256];

void R_SetPalette(const unsigned char *palette)
{
	int i;

	byte *rp = (byte *) r_rawpalette;
	
	if (palette) {
		for (i = 0; i < 256; i++) {
			rp[i * 4 + 0] = palette[i * 3 + 0];
			rp[i * 4 + 1] = palette[i * 3 + 1];
			rp[i * 4 + 2] = palette[i * 3 + 2];
			rp[i * 4 + 3] = 0xff;
		}
	} else {
		for (i = 0; i < 256; i++) {
			rp[i * 4 + 0] = d_8to24table[i] & 0xff;
			rp[i * 4 + 1] = (d_8to24table[i] >> 8) & 0xff;
			rp[i * 4 + 2] = (d_8to24table[i] >> 16) & 0xff;
			rp[i * 4 + 3] = 0xff;
		}
	}
}