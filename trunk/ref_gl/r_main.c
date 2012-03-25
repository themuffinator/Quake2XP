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

viddef_t vid;


model_t *r_worldmodel;

float gldepthmin, gldepthmax;

glconfig_t gl_config;
glstate_t gl_state;

image_t *r_notexture;			// use for bad textures

image_t *r_radarmap;			// wall texture for radar texgen
image_t *r_around;


entity_t *currententity;
model_t *currentmodel;

cplane_t frustum[4];

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

mat4x4_t r_world_matrix;
mat4x4_t r_base_world_matrix;
mat4x4_t r_project_matrix;

//
// screen size info
//
refdef_t r_newrefdef;
glwstate_t glw_state;
int r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;

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
			sprintf(S, "UNKNOWN GL ERROR\n");
			break;
	}

	Con_Printf(PRINT_ALL, S);

	return n;
}


/*
=================
R_CullBox

Returns true if the box is completely outside the frustom
=================
*/
qboolean R_CullBox(vec3_t mins, vec3_t maxs)
{
	int i;

	if (r_noCull->value)
		return false;

	for (i = 0; i < 4; i++)
		if (BOX_ON_PLANE_SIDE(mins, maxs, &frustum[i]) == 2)
			return true;
	return false;
}

/*
=================
R_CullOrigin

Returns true if the origin is completely outside the frustom
=================
*/
qboolean R_CullOrigin(vec3_t origin)
{
	int i;
	
	if (r_noCull->value)
		return false;

	for (i = 0; i < 4; i++)
		if (BOX_ON_PLANE_SIDE(origin, origin, &frustum[i]) == 2)
			return true;
	return false;
}


qboolean R_CullPoint(vec3_t org)
{
	int i;

	for (i = 0; i < 4; i++)
		if (DotProduct(org, frustum[i].normal) > frustum[i].dist)
			return true;

	return false;
}

void R_RotateForEntity(entity_t * e)
{
	qglTranslatef(e->origin[0], e->origin[1], e->origin[2]);

	qglRotatef( e->angles[1], 0, 0, 1);
	qglRotatef(-e->angles[0], 0, 1, 0);
	qglRotatef(-e->angles[2], 1, 0, 0);
}

/*
=============================================================

  SPRITE MODELS

=============================================================
*/


void R_DrawSpriteModel(entity_t * e)
{

	vec3_t point;
	dsprframe_t *frame;
	float *up, *right;
	dsprite_t *psprite;

	if (e->flags & RF_DISTORT)
		return;

	// don't even bother culling, because it's just a single
	// polygon without a surface cache

	psprite = (dsprite_t *) currentmodel->extradata;
	e->frame %= psprite->numframes;
	frame = &psprite->frames[e->frame];

	// normal sprite
	up = vup;
	right = vright;


	qglDepthMask(false);
	qglDepthFunc(GL_LEQUAL);
	
	qglEnable(GL_BLEND);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	qglColor4f(1, 1, 1, 1);

	GL_Bind(currentmodel->skins[e->frame]->texnum);

	GL_PicsColorScaleARB(true);

	qglBegin(GL_QUADS);

	qglTexCoord2f(0, 1);
	VectorMA(e->origin, -frame->origin_y, up, point);
	VectorMA(point, -frame->origin_x, right, point);
	qglVertex3fv(point);

	qglTexCoord2f(0, 0);
	VectorMA(e->origin, frame->height - frame->origin_y, up, point);
	VectorMA(point, -frame->origin_x, right, point);
	qglVertex3fv(point);

	qglTexCoord2f(1, 0);
	VectorMA(e->origin, frame->height - frame->origin_y, up, point);
	VectorMA(point, frame->width - frame->origin_x, right, point);
	qglVertex3fv(point);

	qglTexCoord2f(1, 1);
	VectorMA(e->origin, -frame->origin_y, up, point);
	VectorMA(point, frame->width - frame->origin_x, right, point);
	qglVertex3fv(point);

	qglEnd();

	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	qglDisable(GL_BLEND);
	qglColor4f(1, 1, 1, 1);
	qglDepthMask(true);
}


static void R_DrawDistortSpriteModel(entity_t * e)
{
	dsprframe_t *frame;
	float		*up, *right;
	dsprite_t	*psprite;
	int			id;
	unsigned	defBits = 0;
	unsigned	Index[MAX_INDICES];
	int			vert=0, index=0;
	vec3_t		dist;
	float		len;
	
	psprite = (dsprite_t *) currentmodel->extradata;
	e->frame %= psprite->numframes;
	frame = &psprite->frames[e->frame];

	VectorSubtract(e->origin, r_origin, dist);
	len = VectorLength(dist);

	// normal sprite
	up = vup;
	right = vright;

	qglEnable(GL_BLEND);
	defBits = worldDefs.AlphaMaskBits;

	// setup program
	GL_BindProgram(refractProgram, defBits);
	id = refractProgram->id[defBits];
		
	qglEnableVertexAttribArray(ATRB_POSITION);
	qglEnableVertexAttribArray(ATRB_TEX0);

	qglVertexAttribPointer(ATRB_POSITION, 3, GL_FLOAT, false, 0, wVertexArray);	
	qglVertexAttribPointer(ATRB_TEX0, 2, GL_FLOAT, false, 0, wTexArray);
	
	GL_MBind(GL_TEXTURE0_ARB, r_distort->texnum);
	qglUniform1i(qglGetUniformLocation(id, "u_deformMap"), 0);
	GL_MBind(GL_TEXTURE1_ARB, currentmodel->skins[e->frame]->texnum);
	qglUniform1i(qglGetUniformLocation(id, "u_colorMap"), 1);
	GL_MBindRect(GL_TEXTURE2_ARB, ScreenMap->texnum);
	qglUniform1i(qglGetUniformLocation(id, "g_colorBufferMap"), 2);
	GL_MBindRect(GL_TEXTURE3_ARB, depthMap->texnum);
	qglUniform1i(qglGetUniformLocation(id, "g_depthBufferMap"), 3);
			
	qglUniform1f(qglGetUniformLocation(id, "u_deformMul"),	11.0);
	qglUniform1f(qglGetUniformLocation(id, "u_alpha"),	e->alpha);
	qglUniform1f(qglGetUniformLocation(id, "u_thickness"),	len*0.5);
	qglUniform1f(qglGetUniformLocation(id, "u_thickness2"),	frame->height * 0.2);
	qglUniform2f(qglGetUniformLocation(id, "u_viewport"),	vid.width, vid.height);
	qglUniform2f(qglGetUniformLocation(id, "u_depthParms"), r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);
	qglUniform2f(qglGetUniformLocation(id, "u_mask"),	0.0, 1.0);

	VectorMA (e->origin, -frame->origin_y, up, wVertexArray[vert+0]);
	VectorMA (wVertexArray[vert+0], -frame->origin_x, right, wVertexArray[vert+0]);
	VA_SetElem2(wTexArray[vert+0], 0, 1);
	
	VectorMA (e->origin, frame->height - frame->origin_y, up, wVertexArray[vert+1]);
	VectorMA (wVertexArray[vert+1], -frame->origin_x, right, wVertexArray[vert+1]);
    VA_SetElem2(wTexArray[vert+1], 0, 0);

	VectorMA (e->origin, frame->height - frame->origin_y, up, wVertexArray[vert+2]);
	VectorMA (wVertexArray[vert+2], frame->width - frame->origin_x, right, wVertexArray[vert+2]);
    VA_SetElem2(wTexArray[vert+2], 1, 0);

	VectorMA (e->origin, -frame->origin_y, up, wVertexArray[vert+3]);
	VectorMA (wVertexArray[vert+3],frame->width - frame->origin_x, right, wVertexArray[vert+3]);
    VA_SetElem2(wTexArray[vert+3], 1, 1);

	Index[index++] = vert+0;
	Index[index++] = vert+1;
	Index[index++] = vert+3;
	Index[index++] = vert+3;
	Index[index++] = vert+1;
	Index[index++] = vert+2;
			
	vert+=4;

	
	if(vert)
	{
		if(gl_state.DrawRangeElements && r_DrawRangeElements->value)
			qglDrawRangeElementsEXT(GL_TRIANGLES, 0, vert, index, GL_UNSIGNED_INT, Index);
		else
			qglDrawElements(GL_TRIANGLES, index, GL_UNSIGNED_INT, Index);
	}

	qglDisableVertexAttribArray(ATRB_POSITION);
	qglDisableVertexAttribArray(ATRB_TEX0);
	GL_SelectTexture(GL_TEXTURE0_ARB);
	GL_BindNullProgram();
	qglDisable(GL_BLEND);
}

//==================================================================================

/*
=============
R_DrawNullModel
=============
*/
void R_DrawNullModel(void)
{
	int i;

	if (currententity->flags & (RF_VIEWERMODEL))
		return;

	qglPushMatrix();
	R_RotateForEntity(currententity);

	qglDisable(GL_TEXTURE_2D);
	qglColor3f(1, 0, 1);

	qglBegin(GL_TRIANGLE_FAN);
	qglVertex3f(0, 0, -8);
	for (i = 0; i <= 4; i++)
		qglVertex3f(8 * cos(i * M_PI * 0.5f), 8 * sin(i * M_PI * 0.5f),
					0);
	qglEnd();

	qglBegin(GL_TRIANGLE_FAN);
	qglVertex3f(0, 0, 8);
	for (i = 4; i >= 0; i--)
		qglVertex3f(8 * cos(i * M_PI * 0.5f), 8 * sin(i * M_PI * 0.5f),
					0);
	qglEnd();

	qglColor3f(1, 1, 1);
	qglPopMatrix();
	qglEnable(GL_TEXTURE_2D);
}




//=======================================================================

int SignbitsForPlane(cplane_t * out)
{
	int bits, j;

	// for fast box on planeside test

	bits = 0;
	for (j = 0; j < 3; j++) {
		if (out->normal[j] < 0)
			bits |= 1 << j;
	}
	return bits;
}


void R_SetFrustum(void)
{
	int i;

	if (r_newrefdef.fov_x == 90) {
		// front side is visible

		VectorAdd(vpn, vright, frustum[0].normal);
		VectorSubtract(vpn, vright, frustum[1].normal);

		VectorAdd(vpn, vup, frustum[2].normal);
		VectorSubtract(vpn, vup, frustum[3].normal);
	} else {
		// Speedup Small Calculations - Eradicator
		RotatePointAroundVector(frustum[0].normal, vup, vpn,
								-(90 - r_newrefdef.fov_x * 0.5));
		RotatePointAroundVector(frustum[1].normal, vup, vpn,
								90 - r_newrefdef.fov_x * 0.5);
		RotatePointAroundVector(frustum[2].normal, vright, vpn,
								90 - r_newrefdef.fov_y * 0.5);
		RotatePointAroundVector(frustum[3].normal, vright, vpn,
								-(90 - r_newrefdef.fov_y * 0.5));
	}

	for (i = 0; i < 4; i++) {
		frustum[i].type = PLANE_ANYZ;
		frustum[i].dist = DotProduct(r_origin, frustum[i].normal);
		frustum[i].signbits = SignbitsForPlane(&frustum[i]);
	}
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
		
	// build the transformation matrix for the given view angles
	VectorCopy(r_newrefdef.vieworg, r_origin);

	AngleVectors(r_newrefdef.viewangles, vpn, vright, vup);
	
	// current viewcluster
	if (!(r_newrefdef.rdflags & RDF_NOWORLDMODEL)) {
		r_oldviewcluster = r_viewcluster;
		r_oldviewcluster2 = r_viewcluster2;
		leaf = Mod_PointInLeaf(r_origin, r_worldmodel);
		r_viewcluster = r_viewcluster2 = leaf->cluster;
		
		if (r_viewcluster == -1)
			qglClear(GL_COLOR_BUFFER_BIT);
		
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

	// clear out the portion of the screen that the NOWORLDMODEL defines
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL) {
		qglEnable(GL_SCISSOR_TEST);
		qglScissor(r_newrefdef.x,
				   vid.height - r_newrefdef.height - r_newrefdef.y,
				   r_newrefdef.width, 
				   r_newrefdef.height);

		if (!(r_newrefdef.rdflags & RDF_NOCLEAR)) {
			qglClearColor(0.35, 0.35, 0.35, 1);
			qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			qglClearColor(0.35, 0.35, 0.35, 1);
		} else
			qglClear(GL_DEPTH_BUFFER_BIT);
		qglDisable(GL_SCISSOR_TEST);
	}

	
}



/*==================
INFINITY PERSPECTIVE
==================*/

GLdouble nudge = 1.0 - 1.0 / ((GLdouble) (1 << 23));

GLdouble p[4][4] = {
	{0.0, 0.0, 0.0, 0.0}
	,
	{0.0, 0.0, 0.0, 0.0}
	,
	{0.0, 0.0, 0.0, -1.0}
	,
	{0.0, 0.0, 0.0, 0.0}
};

void MyGlPerspective(GLdouble fov, GLdouble aspectr, GLdouble zNear)
{
	GLdouble fov1, fov2;
	fov1 = fov2 = fov * M_PI / 360.0;
	
	r_newrefdef.depthParms[0] = zNear;
	r_newrefdef.depthParms[1] = 0.9995f;

	p[0][0] = 1.0 / (aspectr * tan(fov1));
	p[1][1] = 1.0 / tan(fov2);

	p[2][2] = -0.999f;
	p[3][2] = -2.f * zNear;

	qglLoadMatrixd(&p[0][0]);
}

/*
PENTA:
from http://www.markmorley.com/opengl/frustumculling.html
Should clean it up by using procedures.
*/
float frustumPlanes[6][4];

void ExtractFrustum()			// <AWE> added return type.
{
	float proj[16];
	float modl[16];
	float clip[16];
	float t;

	/* Get the current PROJECTION matrix from OpenGL */
	qglGetFloatv(GL_PROJECTION_MATRIX, proj);

	/* Get the current MODELVIEW matrix from OpenGL */
	qglGetFloatv(GL_MODELVIEW_MATRIX, modl);

	/* Combine the two matrices (multiply projection by modelview) */
	clip[0] =
		modl[0] * proj[0] + modl[1] * proj[4] + modl[2] * proj[8] +
		modl[3] * proj[12];
	clip[1] =
		modl[0] * proj[1] + modl[1] * proj[5] + modl[2] * proj[9] +
		modl[3] * proj[13];
	clip[2] =
		modl[0] * proj[2] + modl[1] * proj[6] + modl[2] * proj[10] +
		modl[3] * proj[14];
	clip[3] =
		modl[0] * proj[3] + modl[1] * proj[7] + modl[2] * proj[11] +
		modl[3] * proj[15];

	clip[4] =
		modl[4] * proj[0] + modl[5] * proj[4] + modl[6] * proj[8] +
		modl[7] * proj[12];
	clip[5] =
		modl[4] * proj[1] + modl[5] * proj[5] + modl[6] * proj[9] +
		modl[7] * proj[13];
	clip[6] =
		modl[4] * proj[2] + modl[5] * proj[6] + modl[6] * proj[10] +
		modl[7] * proj[14];
	clip[7] =
		modl[4] * proj[3] + modl[5] * proj[7] + modl[6] * proj[11] +
		modl[7] * proj[15];

	clip[8] =
		modl[8] * proj[0] + modl[9] * proj[4] + modl[10] * proj[8] +
		modl[11] * proj[12];
	clip[9] =
		modl[8] * proj[1] + modl[9] * proj[5] + modl[10] * proj[9] +
		modl[11] * proj[13];
	clip[10] =
		modl[8] * proj[2] + modl[9] * proj[6] + modl[10] * proj[10] +
		modl[11] * proj[14];
	clip[11] =
		modl[8] * proj[3] + modl[9] * proj[7] + modl[10] * proj[11] +
		modl[11] * proj[15];

	clip[12] =
		modl[12] * proj[0] + modl[13] * proj[4] + modl[14] * proj[8] +
		modl[15] * proj[12];
	clip[13] =
		modl[12] * proj[1] + modl[13] * proj[5] + modl[14] * proj[9] +
		modl[15] * proj[13];
	clip[14] =
		modl[12] * proj[2] + modl[13] * proj[6] + modl[14] * proj[10] +
		modl[15] * proj[14];
	clip[15] =
		modl[12] * proj[3] + modl[13] * proj[7] + modl[14] * proj[11] +
		modl[15] * proj[15];

	/* Extract the numbers for the RIGHT plane */
	frustumPlanes[0][0] = clip[3] - clip[0];
	frustumPlanes[0][1] = clip[7] - clip[4];
	frustumPlanes[0][2] = clip[11] - clip[8];
	frustumPlanes[0][3] = clip[15] - clip[12];

	/* Normalize the result */
	t = sqrt(frustumPlanes[0][0] * frustumPlanes[0][0] +
			 frustumPlanes[0][1] * frustumPlanes[0][1] +
			 frustumPlanes[0][2] * frustumPlanes[0][2]);
	frustumPlanes[0][0] /= t;
	frustumPlanes[0][1] /= t;
	frustumPlanes[0][2] /= t;
	frustumPlanes[0][3] /= t;

	/* Extract the numbers for the LEFT plane */
	frustumPlanes[1][0] = clip[3] + clip[0];
	frustumPlanes[1][1] = clip[7] + clip[4];
	frustumPlanes[1][2] = clip[11] + clip[8];
	frustumPlanes[1][3] = clip[15] + clip[12];

	/* Normalize the result */
	t = sqrt(frustumPlanes[1][0] * frustumPlanes[1][0] +
			 frustumPlanes[1][1] * frustumPlanes[1][1] +
			 frustumPlanes[1][2] * frustumPlanes[1][2]);
	frustumPlanes[1][0] /= t;
	frustumPlanes[1][1] /= t;
	frustumPlanes[1][2] /= t;
	frustumPlanes[1][3] /= t;

	/* Extract the BOTTOM plane */
	frustumPlanes[2][0] = clip[3] + clip[1];
	frustumPlanes[2][1] = clip[7] + clip[5];
	frustumPlanes[2][2] = clip[11] + clip[9];
	frustumPlanes[2][3] = clip[15] + clip[13];

	/* Normalize the result */
	t = sqrt(frustumPlanes[2][0] * frustumPlanes[2][0] +
			 frustumPlanes[2][1] * frustumPlanes[2][1] +
			 frustumPlanes[2][2] * frustumPlanes[2][2]);
	frustumPlanes[2][0] /= t;
	frustumPlanes[2][1] /= t;
	frustumPlanes[2][2] /= t;
	frustumPlanes[2][3] /= t;

	/* Extract the TOP plane */
	frustumPlanes[3][0] = clip[3] - clip[1];
	frustumPlanes[3][1] = clip[7] - clip[5];
	frustumPlanes[3][2] = clip[11] - clip[9];
	frustumPlanes[3][3] = clip[15] - clip[13];

	/* Normalize the result */
	t = sqrt(frustumPlanes[3][0] * frustumPlanes[3][0] +
			 frustumPlanes[3][1] * frustumPlanes[3][1] +
			 frustumPlanes[3][2] * frustumPlanes[3][2]);
	frustumPlanes[3][0] /= t;
	frustumPlanes[3][1] /= t;
	frustumPlanes[3][2] /= t;
	frustumPlanes[3][3] /= t;

	/* Extract the FAR plane */
	frustumPlanes[4][0] = clip[3] - clip[2];
	frustumPlanes[4][1] = clip[7] - clip[6];
	frustumPlanes[4][2] = clip[11] - clip[10];
	frustumPlanes[4][3] = clip[15] - clip[14];

	/* Normalize the result */
	t = sqrt(frustumPlanes[4][0] * frustumPlanes[4][0] +
			 frustumPlanes[4][1] * frustumPlanes[4][1] +
			 frustumPlanes[4][2] * frustumPlanes[4][2]);
	frustumPlanes[4][0] /= t;
	frustumPlanes[4][1] /= t;
	frustumPlanes[4][2] /= t;
	frustumPlanes[4][3] /= t;

	/* Extract the NEAR plane */
	frustumPlanes[5][0] = clip[3] + clip[2];
	frustumPlanes[5][1] = clip[7] + clip[6];
	frustumPlanes[5][2] = clip[11] + clip[10];
	frustumPlanes[5][3] = clip[15] + clip[14];

	/* Normalize the result */
	t = sqrt(frustumPlanes[5][0] * frustumPlanes[5][0] +
			 frustumPlanes[5][1] * frustumPlanes[5][1] +
			 frustumPlanes[5][2] * frustumPlanes[5][2]);
	frustumPlanes[5][0] /= t;
	frustumPlanes[5][1] /= t;
	frustumPlanes[5][2] /= t;
	frustumPlanes[5][3] /= t;
}

float SphereInFrustum( vec3_t o, float radius )
{
	int		p;
	float	d;

	for( p = 0; p < 6; p++ )
	{
		d = frustumPlanes[p][0] * o[0] + frustumPlanes[p][1] * o[1] + frustumPlanes[p][2] * o[2] + frustumPlanes[p][3];
		if( d <= -radius )
			return 0;
	}
	return d + radius;
}


/*
=============
R_SetupGL
=============
*/
int r_viewport[4];


void R_SetupGL(void)
{
	float screenaspect;
//  float   yfov;
	int x, x2, y2, y, w, h;

	// 
	// set up viewport
	// 
	x = floor (r_newrefdef.x * vid.width / vid.width);
	x2 = ceil ((r_newrefdef.x + r_newrefdef.width) * vid.width / vid.width);
	y = floor (vid.height - r_newrefdef.y * vid.height / vid.height);
	y2 = ceil (vid.height - (r_newrefdef.y + r_newrefdef.height) * vid.height / vid.height);

	w = x2 - x;
	h = y - y2;

	qglViewport(x, y2, w, h);

	// 
	// set up projection matrix
	// 
	screenaspect = (float) r_newrefdef.width / r_newrefdef.height;
//  yfov = 2*atan((float)r_newrefdef.height/r_newrefdef.width)*180/M_PI;
	qglMatrixMode(GL_PROJECTION);
	qglLoadIdentity();
	MyGlPerspective(r_newrefdef.fov_y, (float) r_newrefdef.width / r_newrefdef.height, 1);

	qglCullFace(GL_FRONT);

	qglMatrixMode(GL_MODELVIEW);
	qglLoadIdentity();

	qglRotatef(-90, 1, 0, 0);	// put Z going up
	qglRotatef(90, 0, 0, 1);	// put Z going up
	qglRotatef(-r_newrefdef.viewangles[2], 1, 0, 0);
	qglRotatef(-r_newrefdef.viewangles[0], 0, 1, 0);
	qglRotatef(-r_newrefdef.viewangles[1], 0, 0, 1);
	qglTranslatef(-r_newrefdef.vieworg[0], -r_newrefdef.vieworg[1],
				  -r_newrefdef.vieworg[2]);


	qglGetFloatv(GL_MODELVIEW_MATRIX, r_world_matrix);
	qglGetFloatv(GL_PROJECTION_MATRIX, r_project_matrix);	

	qglGetIntegerv(GL_VIEWPORT, (int *) r_viewport);
	// 
	// set drawing parms
	// 
	if (r_cull->value)
		qglEnable(GL_CULL_FACE);
	else
		qglDisable(GL_CULL_FACE);

	GLSTATE_DISABLE_BLEND
	GLSTATE_DISABLE_ALPHATEST 
	qglEnable(GL_DEPTH_TEST);

}




void R_CastShadowVolumes(void)
{
	int i;
	
	if (r_shadows->value < 2)
		return;
	
	if (r_newrefdef.rdflags & RDF_IRGOGGLES)
		return;
			
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	qglEnableVertexAttribArray(ATRB_POSITION);
	qglVertexAttribPointer(ATRB_POSITION, 3, GL_FLOAT, false, 0, ShadowArray);

	qglEnable(GL_CULL_FACE);
	qglDepthMask(0);
	qglDepthFunc(GL_LESS);
	
	if (r_shadowVolumesDebug->value){
		qglColor4f(0.8, 0.3, 0, 0.2);
		qglEnable(GL_BLEND);
	}
	else
		qglColorMask(0, 0, 0, 0);
	
	qglEnable(GL_STENCIL_TEST);
	qglStencilMask(255);
	qglStencilFuncSeparate(GL_FRONT_AND_BACK, GL_ALWAYS, 128, 255);
		
	for (i = 0; i < r_newrefdef.num_entities; i++) 
	{
		currententity = &r_newrefdef.entities[i];
		currentmodel = currententity->model;
		
		if (!currentmodel)
			continue;
		
		if (currentmodel->type != mod_alias)
			continue;
		
		if (currententity->
		flags & (RF_SHELL_HALF_DAM | RF_SHELL_GREEN | RF_SHELL_RED |
				 RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_GOD |
				 RF_TRANSLUCENT | RF_BEAM | RF_WEAPONMODEL | RF_NOSHADOW | RF_DISTORT))
				 continue;
		
		if (!r_playerShadow->value && (currententity->flags & RF_VIEWERMODEL))
			continue;
		
		if (r_shadowVolumesDebug->value && (currententity->flags & RF_VIEWERMODEL))
			continue;
		
		R_DrawShadowVolume(currententity);
	}
	
	qglDisable(GL_STENCIL_TEST);
	qglDisableVertexAttribArray(ATRB_POSITION);
	qglDepthMask(true);
	if (r_shadowVolumesDebug->value)
	qglDisable(GL_BLEND);
	qglDepthFunc(GL_LEQUAL);
	qglColor4f(1,1,1,1);
	qglColorMask(1, 1, 1, 1);
	
}

/*
=============
R_Clear
=============
*/

void R_Clear(void)
{
	qglClearStencil(128);
	qglStencilMask(255);
	qglClear(GL_STENCIL_BUFFER_BIT);

	qglClear(GL_DEPTH_BUFFER_BIT);
	gldepthmin = 0;
	gldepthmax = 1;
	qglDepthFunc(GL_LEQUAL);
	qglDepthRange(gldepthmin, gldepthmax);
}




void R_DrawEntitiesOnList(void)
{
	int i;

	if (!r_drawEntities->value)
		return;

	// draw non-transparent first
	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];

		if (currententity->flags & RF_WEAPONMODEL)
			continue;

		if (currententity->flags & RF_TRANSLUCENT)
			continue;			// solid

		if ( r_newrefdef.rdflags & RDF_IRGOGGLES) 
				goto jump;

			if (currententity->flags & RF_DISTORT)
				continue;
jump:

		if (currententity->flags & RF_BEAM) {
			R_DrawBeam();
		} else {
			currentmodel = currententity->model;
			if (!currentmodel) {
				R_DrawNullModel();
				continue;
			}
			switch (currentmodel->type) {
			case mod_alias:
				R_DrawAliasModel(currententity, false);
				break;
			case mod_brush:
				R_DrawBrushModel(currententity);
				break;
			case mod_sprite:
				R_DrawSpriteModel(currententity);
				break;
			default:
				VID_Error(ERR_DROP, "Bad modeltype");
				break;
			}
		}
	}

	// draw transparent entities
	// we could sort these if it ever becomes a problem...
	qglDepthMask(0);			// no z writes

	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];
		if (currententity->flags & RF_WEAPONMODEL)
			continue;
		if (!(currententity->flags & RF_TRANSLUCENT))
			continue;			// solid

		if ( r_newrefdef.rdflags & RDF_IRGOGGLES) 
				goto next;

			if (currententity->flags & RF_DISTORT)
				continue;
next:

		if (currententity->flags & RF_BEAM) {
			R_DrawBeam();
		} else {
			currentmodel = currententity->model;

			if (!currentmodel) {
				R_DrawNullModel();
				continue;
			}
			switch (currentmodel->type) {
			case mod_alias:
				R_DrawAliasModel(currententity, false);
				break;
			case mod_brush:
				R_DrawBrushModel(currententity);
				break;
			case mod_sprite:
				R_DrawSpriteModel(currententity);
				break;
			default:
				VID_Error(ERR_DROP, "Bad modeltype");
				break;
			}
		}
	}
	qglDepthMask(1);			// back to writing
	
}

/*
=========================
Draw player weapon AFTER 
distort texture capture
=========================
*/
void R_DrawPlayerWeapon(void)
{
	int i;

	if (!r_drawEntities->value)
		return;

	for (i = 0; i < r_newrefdef.num_entities; i++)	// weapon model
	{
		currententity = &r_newrefdef.entities[i];
		currentmodel = currententity->model;
		if (currententity->flags & RF_TRANSLUCENT)
			continue;

		if (!currentmodel)
			continue;
		if (currentmodel->type != mod_alias)
			continue;
		if (!(currententity->flags & RF_WEAPONMODEL))
			continue;
		R_DrawAliasModel(currententity, true);
	}

	qglDepthMask(0);
	for (i = 0; i < r_newrefdef.num_entities; i++)	// weapon shells
	{
		currententity = &r_newrefdef.entities[i];
		currentmodel = currententity->model;
		if (!(currententity->flags & RF_TRANSLUCENT))
			continue;

		if (!currentmodel)
			continue;
		if (currentmodel->type != mod_alias)
			continue;
		if (!(currententity->flags & RF_WEAPONMODEL))
			continue;
		R_DrawAliasModel(currententity, true);
	}
	qglDepthMask(1);
}

void R_DrawAliasModelLightPass (qboolean weapon_model);

void R_DrawPlayerWeaponLightPass(void)
{
	int i;

	if (!r_drawEntities->value)
		return;

	if(!r_bumpAlias->value)
		return;
	
	qglDepthMask(0);
	qglEnable(GL_BLEND);
	qglBlendFunc(GL_ONE, GL_ONE);
	
	for (i = 0; i < r_newrefdef.num_entities; i++)	// weapon model
	{
		currententity = &r_newrefdef.entities[i];
		currentmodel = currententity->model;
		if (currententity->flags & RF_TRANSLUCENT)
			continue;

		if (!currentmodel)
			continue;
		if (currentmodel->type != mod_alias)
			continue;
		if (!(currententity->flags & RF_WEAPONMODEL))
			continue;
		R_DrawAliasModelLightPass(true);
	}
	qglDepthMask(1);
	qglDisable(GL_BLEND);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void R_DrawEntitiesLightPass(void)
{
	int i;

	if (!r_drawEntities->value)
		return;

	if(!r_bumpAlias->value)
		return;

	qglDepthMask(0);
	qglEnable(GL_BLEND);
	qglBlendFunc(GL_ONE, GL_ONE);

	for (i = 0; i < r_newrefdef.num_entities; i++)	
	{
		currententity = &r_newrefdef.entities[i];
		currentmodel = currententity->model;
		if (currententity->flags & RF_TRANSLUCENT)
			continue;

		if (!currentmodel)
			continue;
		if (currentmodel->type != mod_alias)
			continue;
		if (currententity->flags & RF_WEAPONMODEL)
			continue;

		R_DrawAliasModelLightPass(false);
	}
	qglDisable(GL_BLEND);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	qglDepthMask(1);

}

void R_RenderDistortModels(void)
{
	int i;
	
	qglDepthMask(0);
	
	for (i = 0; i < r_newrefdef.num_entities; i++) {
		currententity = &r_newrefdef.entities[i];
		currentmodel = currententity->model;

		if (!currentmodel)
			continue;

		if ( r_newrefdef.rdflags & RDF_IRGOGGLES) 
				continue;

		if (!(currententity->flags & RF_DISTORT))
			continue;

		if (currentmodel->type == mod_alias)
			R_DrawAliasDistortModel(currententity);
		else if (currentmodel->type == mod_sprite)
			R_DrawDistortSpriteModel(currententity);


	}
	qglDepthMask(1);

}


/*
================
R_RenderView

r_newrefdef must be set before the first call
================
*/

void R_RenderView(refdef_t * fd)
{
if (r_noRefresh->value)
		return;

	r_newrefdef = *fd;

	if (!r_worldmodel && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
		VID_Error(ERR_DROP, "R_RenderView: NULL worldmodel");
		
	R_PushDlights();

	if (r_finish->value)
		qglFinish();
	
	R_SetupFrame();
	R_SetFrustum();
	R_SetupGL();
	R_MarkLeaves();				// done here so we know if we're in water
	R_DrawBSP();
	R_RenderDecals();
	R_DrawEntitiesOnList();
	R_CaptureDepthBuffer();
	
	R_BlobShadow();
	R_CastShadowVolumes();	
	R_DrawEntitiesLightPass();
	R_ShadowBlend();

	R_RenderFlares();
	R_DrawParticles(true); //underwater particles
	R_CaptureColorBuffer();
	R_DrawAlphaPoly();
	R_DrawParticles(false); // air particles
	R_CaptureColorBuffer();
	R_RenderDistortModels();
	R_DrawPlayerWeapon();
	R_DrawPlayerWeaponLightPass();
}


void R_SetGL2D(void)
{
	// set 2D virtual screen size

	qglViewport(0, 0, vid.width, vid.height);
	qglMatrixMode(GL_PROJECTION);

	qglLoadIdentity();
	qglOrtho(0, vid.width, vid.height, 0, -99999, 99999);
	qglMatrixMode(GL_MODELVIEW);
	qglLoadIdentity();
	qglDisable(GL_DEPTH_TEST);
	qglDisable(GL_CULL_FACE);
	
	qglColor4f(1, 1, 1, 1);
	
}


/*
====================
R_SetLightLevel

====================
*/
void R_SetLightLevel(void)
{
	vec3_t shadelight;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	// save off light value for server to look at (BIG HACK!)
	R_LightPoint(r_newrefdef.vieworg, shadelight, false);

	// pick the greatest component, which should be the same
	// as the mono value returned by software
	if (shadelight[0] > shadelight[1]) {
		if (shadelight[0] > shadelight[2])
			r_lightLevel->value = 150 * shadelight[0];
		else
			r_lightLevel->value = 150 * shadelight[2];
	} else {
		if (shadelight[1] > shadelight[2])
			r_lightLevel->value = 150 * shadelight[1];
		else
			r_lightLevel->value = 150 * shadelight[2];
	}

}

/*
@@@@@@@@@@@@@@@@@@@@@
R_RenderFrame

@@@@@@@@@@@@@@@@@@@@@
*/

void R_RenderFrame(refdef_t * fd, qboolean client)
{
	
	R_SetLightLevel();
	R_RenderView(fd);
	R_SetGL2D();

	// post processing - cut of if player camera out map bounds
	if(!outMap){
	R_FXAA();
	R_RadialBlur();
	R_ThermalVision();
	R_DofBlur();
	R_Bloom();
	}

	if (v_blend[3] && r_polyBlend->value) {
		
		GL_Blend(true, 0, 0);
		qglDisable(GL_TEXTURE_2D);
		qglColor4f(v_blend[0],v_blend[1],v_blend[2], 0.15);

		qglBegin(GL_QUADS);
		qglVertex2f(0, 0);
		qglVertex2f(vid.width, 0);
		qglVertex2f(vid.width, vid.height);
		qglVertex2f(0, vid.height);
		qglEnd();

		qglColor4f(1, 1, 1, 1);
		qglEnable(GL_TEXTURE_2D);
		GL_Blend(false, 0, 0);
		}
	
	GL_DrawRadar();	// GLOOM RADAR !!!
	numRadarEnts = 0;

	GL_MsgGLError("R_RenderFrame: ");
}

void FlareStatsList_f(void){

	Com_Printf("%i raw flares\n", r_numflares);
	Com_Printf("%i clean flares\n", r_numIgnoreflares);
	Com_Printf("%i total flares\n", r_numflares - r_numIgnoreflares);
}

void Dump_EntityString(void){

	char *buf;
	FILE *f;
	char name[MAX_OSPATH];
	
	if(!r_worldmodel->name){
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

	fprintf(f, buf);

	fclose(f);


}


void R_VideoInfo_f(void){
	
	int mem[4];
	
	if (strstr(gl_config.extensions_string, "GL_NVX_gpu_memory_info")) {
						
		Com_Printf("\nNvidia specific memory info:\n");
		Com_Printf("\n");
		qglGetIntegerv ( GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX , mem);
		Com_Printf("dedicated video memory %i MB\n", mem[0] >>10);

		qglGetIntegerv ( GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX , mem);
		Com_Printf("total available memory %i MB\n", mem[0] >>10);

		qglGetIntegerv ( GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX , mem);
		Com_Printf("currently unused GPU memory %i MB\n", mem[0] >>10);

		qglGetIntegerv ( GPU_MEMORY_INFO_EVICTION_COUNT_NVX , mem);
		Com_Printf("count of total evictions seen by system %i MB\n", mem[0] >>10);

		qglGetIntegerv ( GPU_MEMORY_INFO_EVICTED_MEMORY_NVX , mem);
		Com_Printf("total video memory evicted %i MB\n", mem[0] >>10);

	} else 
		if (strstr(gl_config.extensions_string, "GL_ATI_meminfo")) {
		
        Com_Printf("\nATI/AMD specific memory info:\n");
		Com_Printf("\n");
        qglGetIntegerv (VBO_FREE_MEMORY_ATI, mem);
        Com_Printf("VBO: total memory free in the pool %i MB\n", mem[0] >> 10);
        Com_Printf("VBO: largest available free block in the pool %i MB\n", mem[1] >> 10);
        Com_Printf("VBO: total auxiliary memory free %i MB\n", mem[2] >> 10);
        Com_Printf("VBO: largest auxiliary free block %i MB\n", mem[3] >> 10);

        qglGetIntegerv (TEXTURE_FREE_MEMORY_ATI, mem);
        Com_Printf("Texture: total memory free in the pool %i MB\n", mem[0] >> 10);
        Com_Printf("Texture: largest available free block in the pool %i MB\n", mem[1] >> 10);
        Com_Printf("Texture: total auxiliary memory free %i MB\n", mem[2] >> 10);
        Com_Printf("Texture: largest auxiliary free block %i MB\n", mem[3] >> 10);

        qglGetIntegerv (RENDERBUFFER_FREE_MEMORY_ATI, mem);
        Com_Printf("RenderBuffer: total memory free in the pool %i MB\n", mem[0] >> 10);
        Com_Printf("RenderBuffer: largest available free block in the pool %i MB\n", mem[1] >> 10);
        Com_Printf("RenderBuffer: total auxiliary memory free %i MB\n", mem[2] >> 10);
        Com_Printf("RenderBuffer: largest auxiliary free block %i MB\n", mem[3] >> 10);
    }
	else
		Com_Printf("MemInfo not availabled for your video card or driver!\n");



}

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
	r_dynamic =							Cvar_Get("r_dynamic", "1", 0);
	r_noBind =							Cvar_Get("r_noBind", "0", 0);
	r_cull =							Cvar_Get("r_cull", "1", 0);
	r_polyBlend =						Cvar_Get("r_polyBlend", "1", CVAR_ARCHIVE);
	r_lockPvs =							Cvar_Get("r_lockPvs", "0", 0);

	r_vsync =							Cvar_Get("r_vsync", "0", CVAR_ARCHIVE);
	r_finish =							Cvar_Get("r_finish", "0", 0);
	
	r_fullScreen =						Cvar_Get("r_fullScreen", "1", CVAR_ARCHIVE);
	r_hardwareGamma =					Cvar_Get("r_hardwareGamma", "1", CVAR_ARCHIVE);	
	r_gamma =							Cvar_Get("r_gamma", "0.7", CVAR_ARCHIVE);
	vid_ref =							Cvar_Get("vid_ref", "xpgl", CVAR_ARCHIVE);
	r_displayRefresh =					Cvar_Get("r_displayRefresh", "0", CVAR_ARCHIVE);

	r_shadows =							Cvar_Get("r_shadows", "1", CVAR_ARCHIVE);
	r_shadowWorldLightScale =			Cvar_Get("r_shadowWorldLightScale", "10", CVAR_ARCHIVE);
	r_shadowVolumesDebug =				Cvar_Get("r_shadowVolumesDebug", "0", 0);
	r_playerShadow =					Cvar_Get("r_playerShadow", "1", CVAR_ARCHIVE);
	r_shadowCapOffset =					Cvar_Get("r_shadowCapOffset", "0.1", CVAR_ARCHIVE);

	r_anisotropic =						Cvar_Get("r_anisotropic", "16", CVAR_ARCHIVE);
	r_maxAnisotropy =					Cvar_Get("r_maxAnisotropy", "0", 0);
	r_maxTextureSize=					Cvar_Get("r_maxTextureSize", "0", CVAR_ARCHIVE);
	r_worldColorScale =					Cvar_Get("r_worldColorScale", "2", CVAR_ARCHIVE);
	r_picsColorScale =					Cvar_Get("r_picsColorScale", "2", CVAR_ARCHIVE);
	r_textureCompression =				Cvar_Get("r_textureCompression", "0", CVAR_ARCHIVE);			
	r_causticIntens =					Cvar_Get("r_causticIntens", "2.0", CVAR_ARCHIVE);
	r_textureMode =						Cvar_Get("r_textureMode", "GL_LINEAR_MIPMAP_LINEAR", CVAR_ARCHIVE);

	r_screenShot =						Cvar_Get("r_screenShot", "jpg", CVAR_ARCHIVE);
	r_screenShotJpegQuality =			Cvar_Get("r_screenShotJpegQuality", "99", CVAR_ARCHIVE);
		
	r_radarSize =						Cvar_Get("r_radarSize", "256", CVAR_ARCHIVE);
	r_radarZoom =						Cvar_Get("r_radarZoom", "1", CVAR_ARCHIVE);
	r_radar =							Cvar_Get("r_radar", "0", CVAR_ARCHIVE);
	
	r_arbSamples =						Cvar_Get("r_arbSamples", "1", CVAR_ARCHIVE);
	r_nvSamplesCoverange =				Cvar_Get("r_nvSamplesCoverange", "8", CVAR_ARCHIVE);
	r_fxaa =							Cvar_Get("r_fxaa", "0", CVAR_ARCHIVE);

	deathmatch =						Cvar_Get("deathmatch", "0", CVAR_SERVERINFO);
	
	r_drawFlares =						Cvar_Get("r_drawFlares", "1", CVAR_ARCHIVE);
	r_flaresIntens =					Cvar_Get("r_flaresIntens", "3", CVAR_ARCHIVE);
	r_flareWeldThreshold =				Cvar_Get("r_flareWeldThreshold", "32", CVAR_ARCHIVE);
	
	r_customWidth =						Cvar_Get("r_customWidth", "1024", CVAR_ARCHIVE);
	r_customHeight =					Cvar_Get("r_customHeight", "500", CVAR_ARCHIVE);

	sys_priority =						Cvar_Get("sys_priority", "0", CVAR_ARCHIVE);
		
	r_DrawRangeElements	=				Cvar_Get("r_DrawRangeElements","1",CVAR_ARCHIVE);
			
	hunk_bsp=							Cvar_Get("hunk_bsp", "20", CVAR_ARCHIVE);
	hunk_model=							Cvar_Get("hunk_model", "2.4", CVAR_ARCHIVE);
	hunk_sprite=						Cvar_Get("hunk_sprite", "0.08", CVAR_ARCHIVE);
	
//	r_vbo=								Cvar_Get("r_vbo", "1", CVAR_ARCHIVE);

	r_parallax=							Cvar_Get("r_parallax", "2", CVAR_ARCHIVE);
	r_parallaxScale=					Cvar_Get("r_parallaxScale", "2.0", CVAR_ARCHIVE);

	r_bumpAlias =						Cvar_Get("r_bumpAlias", "1", CVAR_ARCHIVE);
	r_bumpWorld =						Cvar_Get("r_bumpWorld", "1", CVAR_ARCHIVE);
	r_pplWorldAmbient =					Cvar_Get("r_pplWorldAmbient", "0.5", CVAR_ARCHIVE);
	r_tbnSmoothAngle =					Cvar_Get("r_tbnSmoothAngle", "45", CVAR_ARCHIVE);
	r_pplMaxDlights =					Cvar_Get("r_pplMaxDlights", "13", CVAR_ARCHIVE);

	r_bloom =							Cvar_Get("r_bloom", "1", CVAR_ARCHIVE);
	r_bloomThreshold =					Cvar_Get("r_bloomThreshold", "0.75", CVAR_ARCHIVE);
	r_bloomIntens =						Cvar_Get("r_bloomIntens", "2.0", CVAR_ARCHIVE);

	r_dof =								Cvar_Get("r_dof", "1", CVAR_ARCHIVE);
	r_dofBias =							Cvar_Get("r_dofBias", "0.002", CVAR_ARCHIVE);
	r_dofFocus =						Cvar_Get("r_dofFocus", "512.0", CVAR_ARCHIVE);

	r_radialBlur =						Cvar_Get("r_radialBlur", "1", CVAR_ARCHIVE);
	r_radialBlurFov =                   Cvar_Get("r_radialBlurFov", "30", CVAR_ARCHIVE);
	r_softParticles =					Cvar_Get("r_softParticles", "1", CVAR_ARCHIVE);

	r_ignoreGlErrors =					Cvar_Get("r_ignoreGlErrors", "1", 0);

	Cmd_AddCommand("imagelist", GL_ImageList_f);
	Cmd_AddCommand("screenshot", GL_ScreenShot_f);
	Cmd_AddCommand("modellist", Mod_Modellist_f);
	Cmd_AddCommand("openglInfo", GL_Strings_f);
	Cmd_AddCommand("flaresStats", FlareStatsList_f);
	Cmd_AddCommand("dumpEntityString", Dump_EntityString);
	Cmd_AddCommand("glslInfo", R_ListPrograms_f);
	Cmd_AddCommand("r_meminfo", R_VideoInfo_f);
	
	
	
}

/*
==================
R_SetMode
==================
*/
qboolean R_SetMode(void)
{
	rserr_t err;
	qboolean fullscreen;

	fullscreen = r_fullScreen->value;

	r_fullScreen->modified = false;
	r_mode->modified = false;

	if ((err = GLimp_SetMode(&vid.width, &vid.height, r_mode->value, fullscreen)) == rserr_ok) {
		gl_state.prev_mode = r_mode->value;
	} else {
		if (err == rserr_invalid_fullscreen) {
			Cvar_SetValue("r_fullScreen", 0);
			r_fullScreen->modified = false;
			Com_Printf(S_COLOR_RED "ref_xpgl::R_SetMode() - fullscreen unavailable in this mode\n");
			if ((err = GLimp_SetMode(&vid.width, &vid.height, r_mode->value, false)) == rserr_ok)
				return true;
		} else if (err == rserr_invalid_mode) {
			Cvar_SetValue("r_mode", gl_state.prev_mode);
			r_mode->modified = false;
			Com_Printf(S_COLOR_RED"ref_xpgl::R_SetMode() - invalid mode\n");
		}
		// try setting it back to something safe
		if ((err =
			 GLimp_SetMode(&vid.width, &vid.height, gl_state.prev_mode,
						   false)) != rserr_ok) {
			Com_Printf(S_COLOR_RED"ref_xpgl::R_SetMode() - could not revert to safe mode\n");
			return false;
		}
	}
	return true;
}

/*
===============
R_Init
===============
*/
int GL_QueryBits;
int ocQueries[MAX_FLARES];

int R_Init(void *hinstance, void *hWnd)
{
	char			vendor_buffer[1000];
	int				aniso_level, max_aniso;

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
	// set our "safe" modes
	gl_state.prev_mode = 0;

	// create the window and set up the context
	if (!R_SetMode()) {
		QGL_Shutdown();
		Com_Printf(S_COLOR_RED "ref_xpgl::R_Init() - could not R_SetMode()\n");
		return -1;
	}
	
	VID_MenuInit();

	// Get Windows Version
	/* 
	 ** get our various GL strings
	 */
	Com_Printf( "\n");
	gl_config.vendor_string = (const char*)qglGetString(GL_VENDOR);
	Com_Printf(S_COLOR_WHITE "GL_VENDOR:" S_COLOR_GREEN "   %s\n", gl_config.vendor_string);
	gl_config.renderer_string = (const char*)qglGetString(GL_RENDERER);
	Com_Printf(S_COLOR_WHITE "GL_RENDERER:" S_COLOR_GREEN " %s\n", gl_config.renderer_string);
	gl_config.version_string = (const char*)qglGetString(GL_VERSION);
	Com_Printf(S_COLOR_WHITE "GL_VERSION:" S_COLOR_GREEN "  %s\n", gl_config.version_string);
	gl_config.extensions_string = (const char*)qglGetString(GL_EXTENSIONS);
	
	
	strcpy(vendor_buffer, gl_config.vendor_string);
	strlwr(vendor_buffer);

	if (strstr(vendor_buffer, "intel")) // fuck the intel lol
		{
		Com_Printf(S_COLOR_RED"Intel graphics card is unsupported.\n");
		VID_Error(ERR_FATAL,  "Intel graphics card is unsupported.\n");
		}
	
	if (strstr(vendor_buffer, "sis")) // fuck the SiS too
		{
		Com_Printf(S_COLOR_RED"SiS graphics card is unsupported.\n");
		VID_Error(ERR_FATAL,  "SiS graphics card is unsupported.\n");
		}
	
	
	{
	// check GL version /:-#)
	float version = atof(gl_config.version_string);

	if (version < 2.0){
		Com_Printf(S_COLOR_RED"Quake2xp requires OpenGL version 2.0 or higher.\nProbably your graphics card is unsupported or the drivers are not up-to-date.\nCurrent GL version is %3.1f\n", version);
		VID_Error(ERR_FATAL,  "Quake2xp requires OpenGL version 2.0 or higher.\nProbably your graphics card is unsupported or the drivers are not up-to-date.\nCurrent GL version is %3.1f\n", version);
		}
	}
	
	Com_DPrintf(S_COLOR_WHITE "GL_EXTENSIONS:\n"); 
	Com_DPrintf(S_COLOR_YELLOW"%s\n", gl_config.extensions_string);
	
	Cvar_Set("scr_drawall", "0");

#ifdef __linux__
	Cvar_SetValue("r_vsync", 1);
#endif

	
	Com_Printf("\n");
	Com_Printf("=====================================\n");
	Com_Printf(S_COLOR_GREEN"Checking Basic Quake II XP Extensions\n");
	Com_Printf("=====================================\n");
	Com_Printf("\n");
		
	qglGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
	
#ifdef _WIN32
	if (strstr(gl_config.extensions_string, "WGL_EXT_swap_control")) {
		qwglSwapIntervalEXT = (BOOL(WINAPI *) (int)) qwglGetProcAddress("wglSwapIntervalEXT");
		Com_Printf("...using WGL_EXT_swap_control\n");
	} else {
		Com_Printf(S_COLOR_RED"...WGL_EXT_swap_control not found\n");
	}
#endif

if (strstr(gl_config.extensions_string, "GL_ARB_multitexture")) {
	Com_Printf("...using GL_ARB_multitexture\n");
		
		qglMultiTexCoord2fARB =		(PFNGLMULTITEXCOORD2FARBPROC)		qwglGetProcAddress("glMultiTexCoord2fARB");
		qglActiveTextureARB =		(PFNGLACTIVETEXTUREARBPROC)			qwglGetProcAddress("glActiveTextureARB");
		qglClientActiveTextureARB =	(PFNGLCLIENTACTIVETEXTUREARBPROC)	qwglGetProcAddress("glClientActiveTextureARB");
		qglMultiTexCoord3fvARB =	(PFNGLMULTITEXCOORD3FVARBPROC)		qwglGetProcAddress("glMultiTexCoord3fvARB");

	
	} else {
		Com_Printf(S_COLOR_RED"...GL_ARB_multitexture not found\n");
		VID_Error(ERR_FATAL, "GL_ARB_multitexture not found!");
	}



	Cvar_SetValue("r_maxAnisotropy", max_aniso);
	if (r_anisotropic->value >= r_maxAnisotropy->value)
		Cvar_SetValue("r_anisotropic", r_maxAnisotropy->value);

	aniso_level = r_anisotropic->value;
	if (strstr
		(gl_config.extensions_string,
		 "GL_EXT_texture_filter_anisotropic")) {
		if (r_anisotropic->value == 1) {
			Com_Printf(S_COLOR_YELLOW"...ignoring GL_EXT_texture_filter_anisotropic\n");

		} else {

			Com_Printf("...using GL_EXT_texture_filter_anisotropic\n   ["S_COLOR_GREEN"%i"S_COLOR_WHITE" max] ["S_COLOR_GREEN"%i" S_COLOR_WHITE" selected]\n",
					   max_aniso, aniso_level);

		}
	} else {
		Com_Printf(S_COLOR_RED "...GL_EXT_texture_filter_anisotropic not found\n");
		r_anisotropic = Cvar_Set("r_anisotropic", "1");
		r_maxAnisotropy = Cvar_Set("r_maxAnisotropy", "0");
	}


	gl_state.texture_compression_arb = false;
	if (strstr(gl_config.extensions_string, "GL_ARB_texture_compression"))
		if (!r_textureCompression->value) {
			Com_Printf(S_COLOR_YELLOW"...ignoring GL_ARB_texture_compression\n");
			gl_state.texture_compression_arb = false;
		} else {
			Com_Printf("...using GL_ARB_texture_compression\n");
			gl_state.texture_compression_arb = true;

	} else {
		Com_Printf(S_COLOR_RED"...GL_ARB_texture_compression not found\n");
		gl_state.texture_compression_arb = false;
		r_textureCompression = Cvar_Set("r_textureCompression", "0");
	}


	// ===========================================================================================================================

		gl_state.DrawRangeElements = false;
	if (strstr(gl_config.extensions_string, "GL_EXT_draw_range_elements")) {
		gl_state.DrawRangeElements = true;
		qglDrawRangeElementsEXT = (PFNGLDRAWRANGEELEMENTSEXTPROC) qwglGetProcAddress("glDrawRangeElementsEXT");
		
		if(!r_DrawRangeElements->value)
		Com_Printf(S_COLOR_YELLOW"...ignoring GL_EXT_draw_range_elements\n");
		else
		Com_Printf("...using GL_EXT_draw_range_elements\n");
				
		} else {
		Com_Printf(S_COLOR_RED"...GL_EXT_draw_range_elements not found\n");
		gl_state.DrawRangeElements = false;
		r_DrawRangeElements = Cvar_Set("r_DrawRangeElements", "0");
		}

	// openGL 2.0 Unified Separate Stencil
	qglStencilFuncSeparate		= (PFNGLSTENCILFUNCSEPARATEPROC)	qwglGetProcAddress("glStencilFuncSeparate");
	qglStencilOpSeparate		= (PFNGLSTENCILOPSEPARATEPROC)		qwglGetProcAddress("glStencilOpSeparate");
	qglStencilMaskSeparate		= (PFNGLSTENCILMASKSEPARATEPROC)	qwglGetProcAddress("glStencilMaskSeparate");
	
	gl_state.separateStencil = false;
	if(qglStencilFuncSeparate && qglStencilOpSeparate && qglStencilMaskSeparate){
			Com_Printf("...using GL_EXT_stencil_two_side\n");
			gl_state.separateStencil = true;
	
	}else
		Com_Printf(S_COLOR_RED"...GL_EXT_stencil_two_side not found\n");




	gl_state.arb_occlusion = false;
	if (strstr(gl_config.extensions_string, "GL_ARB_occlusion_query")) {
		Com_Printf("...using GL_ARB_occlusion_query\n");
		gl_state.arb_occlusion = true;

		qglGenQueriesARB		=	(PFNGLGENQUERIESARBPROC) qwglGetProcAddress("glGenQueriesARB");
		qglDeleteQueriesARB		=	(PFNGLDELETEQUERIESARBPROC) qwglGetProcAddress("glDeleteQueriesARB");
		qglIsQueryARB			=	(PFNGLISQUERYARBPROC) qwglGetProcAddress("glIsQueryARB");
		qglBeginQueryARB		=	(PFNGLBEGINQUERYARBPROC) qwglGetProcAddress("glBeginQueryARB");
		qglEndQueryARB			=	(PFNGLENDQUERYARBPROC) qwglGetProcAddress("glEndQueryARB");
		qglGetQueryivARB		=	(PFNGLGETQUERYIVARBPROC) qwglGetProcAddress("glGetQueryivARB");
		qglGetQueryObjectivARB	=	(PFNGLGETQUERYOBJECTIVARBPROC) qwglGetProcAddress("glGetQueryObjectivARB");
		qglGetQueryObjectuivARB =	(PFNGLGETQUERYOBJECTUIVARBPROC) qwglGetProcAddress("glGetQueryObjectuivARB");

		qglGetQueryivARB(GL_SAMPLES_PASSED_ARB, GL_QUERY_COUNTER_BITS_ARB, &GL_QueryBits);
		
		if (GL_QueryBits) {
			qglGenQueriesARB(MAX_FLARES, (GLuint*)ocQueries);
			Com_Printf("   Found "S_COLOR_GREEN "%i" S_COLOR_WHITE " occlusion query bits\n", GL_QueryBits);
		}
		
		} else {
		Com_Printf(S_COLOR_RED"...GL_ARB_occlusion_query not found\n");
		gl_state.arb_occlusion = false;
	}

	gl_state.nPot = false;
	if (strstr
		(gl_config.extensions_string, "GL_ARB_texture_non_power_of_two")) {
		Com_Printf("...using GL_ARB_texture_non_power_of_two\n");
		gl_state.nPot = true;
	} else {
		Com_Printf(S_COLOR_RED "...GL_ARB_texture_non_power_of_two not found\n");
		gl_state.nPot = false;
	}
	
	
	if (strstr (gl_config.extensions_string, "GL_ARB_texture_rectangle")) {
		Com_Printf("...using GL_ARB_texture_rectangle\n");
		
		} else {
		Com_Printf(S_COLOR_RED "...GL_ARB_texture_rectangle not found\n");
		
	}
	
/*		gl_state.vbo = false;

	if (strstr(gl_config.extensions_string, "GL_ARB_vertex_buffer_object")) {
			
		qglBindBuffer =		(PFNGLBINDBUFFERPROC)		qwglGetProcAddress("glBindBuffer");
		qglDeleteBuffers =	(PFNGLDELETEBUFFERSPROC)	qwglGetProcAddress("glDeleteBuffers");
		qglGenBuffers =		(PFNGLGENBUFFERSPROC)		qwglGetProcAddress("glGenBuffers");
		qglBufferData =		(PFNGLBUFFERDATAPROC)		qwglGetProcAddress("glBufferData");
		qglBufferSubData =	(PFNGLBUFFERSUBDATAPROC)	qwglGetProcAddress("glBufferSubData");
		qglMapBuffer =		(PFNGLMAPBUFFERPROC)		qwglGetProcAddress("glMapBuffer");
		qglUnmapBuffer =	(PFNGLUNMAPBUFFERPROC)		qwglGetProcAddress("glUnmapBuffer");
		qglBufferSubData =	(PFNGLBUFFERSUBDATAPROC)	qwglGetProcAddress("glBufferSubData");

		if (qglGenBuffers && qglBindBuffer && qglBufferData && qglDeleteBuffers && qglBufferSubData){
				
			Com_Printf("...using GL_ARB_vertex_buffer_object\n");
			gl_state.vbo = true;
		}
	} else {
		Com_Printf(S_COLOR_RED "...GL_ARB_vertex_buffer_object not found\n");
		gl_state.vbo = false;
	}
*/
	gl_state.nv_conditional_render = false;
	if ( strstr( gl_config.extensions_string, "GL_NV_conditional_render" ) )
	{		
		glBeginConditionalRenderNV	= (PFNGLBEGINCONDITIONALRENDERNVPROC)	qwglGetProcAddress("glBeginConditionalRenderNV");
		glEndConditionalRenderNV	= (PFNGLENDCONDITIONALRENDERNVPROC)		qwglGetProcAddress("glEndConditionalRenderNV");
		
		if(glBeginConditionalRenderNV && glEndConditionalRenderNV)
			Com_Printf("...using GL_NV_conditional_render\n");
				gl_state.nv_conditional_render = true;

	} else {
		Com_Printf(S_COLOR_RED"...GL_NV_conditional_render not found\n");
		gl_state.nv_conditional_render = false;		
	}
	
	gl_state.glsl = false;	
	if ( strstr( gl_config.extensions_string, "GL_ARB_shading_language_100" ) )
	{
	if ( strstr( gl_config.extensions_string, "GL_ARB_fragment_shader" ) )
		{
		Com_Printf("...using GL_ARB_fragment_shader\n");

		if ( strstr( gl_config.extensions_string, "GL_ARB_vertex_shader" ) ){
			Com_Printf("...using GL_ARB_vertex_shader\n");
	gl_state.glsl = true;	

	gl_config.shadingLanguageVersionString = (const char*)qglGetString(GL_SHADING_LANGUAGE_VERSION_ARB);
	qglGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &gl_config.maxFragmentUniformComponents);
	qglGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &gl_config.maxVertexUniformComponents);
	qglGetIntegerv(GL_MAX_VARYING_FLOATS, &gl_config.maxVaryingFloats);
	qglGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &gl_config.maxVertexTextureImageUnits);
	qglGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &gl_config.maxCombinedTextureImageUnits);
	qglGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &gl_config.maxFragmentUniformComponents);
	qglGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &gl_config.maxVertexAttribs);
	qglGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &gl_config.maxTextureImageUnits);

	Com_Printf("\n");
	Com_Printf(S_COLOR_YELLOW"   GLSL Version:               "S_COLOR_GREEN"   %s\n", gl_config.shadingLanguageVersionString);
	Com_Printf(S_COLOR_YELLOW"   maxFragmentUniformComponents:"S_COLOR_GREEN"  %i\n", gl_config.maxFragmentUniformComponents);
	Com_Printf(S_COLOR_YELLOW"   maxVertexUniformComponents: "S_COLOR_GREEN"   %i\n", gl_config.maxVertexUniformComponents);
	Com_Printf(S_COLOR_YELLOW"   maxVertexAttribs:           "S_COLOR_GREEN"   %i\n", gl_config.maxVertexAttribs);
	Com_Printf(S_COLOR_YELLOW"   maxVaryingFloats:           "S_COLOR_GREEN"   %i\n", gl_config.maxVaryingFloats);
	Com_Printf(S_COLOR_YELLOW"   maxVertexTextureImageUnits: "S_COLOR_GREEN"   %i\n", gl_config.maxVertexTextureImageUnits);
	Com_Printf(S_COLOR_YELLOW"   maxTextureImageUnits:       "S_COLOR_GREEN"   %i\n", gl_config.maxTextureImageUnits);
	Com_Printf(S_COLOR_YELLOW"   maxCombinedTextureImageUnits: "S_COLOR_GREEN" %i\n", gl_config.maxCombinedTextureImageUnits);
	Com_Printf(S_COLOR_YELLOW"   maxFragmentUniformComponents: "S_COLOR_GREEN" %i\n", gl_config.maxFragmentUniformComponents);

	qglCreateShader =				(PFNGLCREATESHADERPROC)				qwglGetProcAddress("glCreateShader");
	qglCreateProgram =				(PFNGLCREATEPROGRAMOBJECTARBPROC)	qwglGetProcAddress("glCreateProgram");
	qglDeleteShader =				(PFNGLDELETESHADERPROC)				qwglGetProcAddress("glDeleteShader");
	qglDeleteProgram =				(PFNGLDELETEPROGRAMPROC)			qwglGetProcAddress("glDeleteProgram");
	qglGetShaderiv =				(PFNGLGETSHADERIVPROC)				qwglGetProcAddress("glGetShaderiv");
	qglGetProgramiv =				(PFNGLGETPROGRAMIVARBPROC)			qwglGetProcAddress("glGetProgramiv");
	qglGetShaderInfoLog =			(PFNGLGETSHADERINFOLOGPROC)			qwglGetProcAddress("glGetShaderInfoLog");
	qglGetProgramInfoLog =			(PFNGLGETPROGRAMINFOLOGPROC)		qwglGetProcAddress("glGetProgramInfoLog");
	qglShaderSource =				(PFNGLSHADERSOURCEPROC)				qwglGetProcAddress("glShaderSource");
	qglCompileShader =				(PFNGLCOMPILESHADERARBPROC)			qwglGetProcAddress("glCompileShader");
	qglAttachShader =				(PFNGLATTACHSHADERPROC)				qwglGetProcAddress("glAttachShader");
	qglDetachShader =				(PFNGLDETACHSHADERPROC)				qwglGetProcAddress("glDetachShader");
	qglLinkProgram =				(PFNGLLINKPROGRAMPROC)				qwglGetProcAddress("glLinkProgram");
	qglUseProgram =					(PFNGLUSEPROGRAMPROC)				qwglGetProcAddress("glUseProgram");
	qglVertexAttribPointer =		(PFNGLVERTEXATTRIBPOINTERPROC)				qwglGetProcAddress("glVertexAttribPointer");
	qglEnableVertexAttribArray =	(PFNGLENABLEVERTEXATTRIBARRAYARBPROC)		qwglGetProcAddress("glEnableVertexAttribArray");
	qglDisableVertexAttribArray =	(PFNGLDISABLEVERTEXATTRIBARRAYARBPROC)		qwglGetProcAddress("glDisableVertexAttribArray");
	qglBindAttribLocation =			(PFNGLBINDATTRIBLOCATIONARBPROC)			qwglGetProcAddress("glBindAttribLocation");
	qglGetAttribLocation =			(PFNGLGETATTRIBLOCATIONARBPROC)				qwglGetProcAddress("glGetAttribLocation");
	qglGetActiveUniform =			(PFNGLGETACTIVEUNIFORMPROC)					qwglGetProcAddress("glGetActiveUniform");
	qglGetUniformLocation =			(PFNGLGETUNIFORMLOCATIONPROC)				qwglGetProcAddress("glGetUniformLocation");
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
	
	R_InitPrograms();
	}

	}
		
	} else {
		Com_Printf(S_COLOR_RED"...GLSL render not supported\n");
		gl_state.glsl = false;
		VID_Error(ERR_FATAL, "...GLSL render not supported!");
	}
			
	Com_Printf("\n");
	Com_Printf("=====================================\n");

	GL_SetDefaultState();
	GL_InitImages();
	Mod_Init();
	R_InitEngineTextures();
	R_LoadFont();

	GL_MsgGLError("Init GL Errors: ");
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
	Cmd_RemoveCommand("imagelist");
	Cmd_RemoveCommand("glstrings");
	Cmd_RemoveCommand("flaresStats");
	Cmd_RemoveCommand("dumpEntityString");
	Cmd_RemoveCommand("r_meminfo");	

	Mod_FreeAll();
	GL_ShutdownImages();
	qglDeleteQueriesARB(MAX_FLARES, (GLuint*)ocQueries);
	GLimp_Shutdown();
	QGL_Shutdown();
	R_ShutdownPrograms();
}



/*
@@@@@@@@@@@@@@@@@@@@@
R_BeginFrame
@@@@@@@@@@@@@@@@@@@@@
*/
void UpdateGammaRamp();

void R_BeginFrame()
{

	/* 
	 ** change modes if necessary
	 */
	if (r_mode->modified || r_fullScreen->modified) {
		cvar_t *ref;

		ref = Cvar_Get("vid_ref", "gl", 0);
		ref->modified = true;
	}
	
	
	if (r_gamma->modified) {
		r_gamma->modified = false;

		if (gl_state.gammaramp) {
			UpdateGammaRamp();
		}
	}


	// realtime update
	if(r_softParticles->modified)
		r_softParticles->modified = false;
	
	if(r_dof->modified)
		r_dof->modified = false;


	if(r_pplMaxDlights->value < 8)
		Cvar_SetValue("r_pplMaxDlights", 8);
	
	if(r_pplMaxDlights->value > 13)
		Cvar_SetValue("r_pplMaxDlights", 13);

	qglDrawBuffer( GL_BACK );
	
	/* 
	 ** go into 2D mode
	 */
	qglViewport(0, 0, vid.width, vid.height);
	qglMatrixMode(GL_PROJECTION);
	qglLoadIdentity();
	qglOrtho(0, vid.width, vid.height, 0, -99999, 99999);
	qglMatrixMode(GL_MODELVIEW);
	qglLoadIdentity();
	qglDisable(GL_DEPTH_TEST);
	qglDisable(GL_CULL_FACE);
	GLSTATE_DISABLE_BLEND 
	qglColor4f(1, 1, 1, 1);

	/* 
	 ** texturemode stuff
	 */
	// Realtime set level of anisotropy filtering
	if (r_textureMode->modified || r_anisotropic->modified) {
		GL_TextureMode(r_textureMode->string);

		if (r_textureMode->modified)
			r_textureMode->modified = false;

		if (r_anisotropic->modified)
			r_anisotropic->modified = false;
	}

	
	/* 
	 ** swapinterval stuff
	 */
	GL_UpdateSwapInterval();

	// 
	// clear screen if desired
	// 
	R_Clear();
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
	qglClearColor(0.35, 0.35, 0.35, 1);
	qglClear(GL_COLOR_BUFFER_BIT);
	qglClearColor(0.35, 0.35, 0.35, 1);
	
}

/*
** R_DrawBeam
*/
void R_DrawBeam()
{
	return;
}
