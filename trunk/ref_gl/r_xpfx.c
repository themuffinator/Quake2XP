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

cvar_t	*r_radarSize;			// GLOOM radar
cvar_t	*r_radarZoom;
cvar_t	*r_radar;

cvar_t	*r_arbSamples;
cvar_t	*r_useCSAA;
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
cvar_t	*r_bumpMapping;
cvar_t	*r_ambientLevel;

cvar_t	*hunk_bsp;
cvar_t	*hunk_model;
cvar_t	*hunk_sprite;

cvar_t	*r_vbo;
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
	GL_BindRect(ScreenMap->texnum);
	qglCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
	qglDisable(GL_TEXTURE_RECTANGLE_ARB);
}

void R_CaptureDepthBuffer()
{
		
	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
		
	qglEnable(GL_TEXTURE_RECTANGLE_ARB);
	GL_BindRect(depthMap->texnum);
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


/*=======================================
VBO management based on Q2XP2 VBO-manager
=========================================
*/

qboolean	vc_initialised = false;
qboolean	need_free_vbo;

void GL_BindVBO(vertCache_t *cache)
{
	if (cache)
		qglBindBuffer(GL_ARRAY_BUFFER_ARB, cache->id);
	else
		qglBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
}

vertCache_t *R_VCFindCache(vertStoreMode_t store, entity_t *ent, int mesh)
{
	model_t		*mod;
	int			frame;
	float		backlerp;
	vec3_t		angles, orgs;
	vertCache_t	*cache, *next;

	if (store == VBO_STORE_XYZ)
	{
		mod = ent->model;
		frame = ent->frame;
		backlerp = ent->backlerp;

		angles[0] = ent->angles[0];
		angles[1] = ent->angles[1];
		angles[2] = ent->angles[2];
		orgs[0] = ent->origin[0];
		orgs[1] = ent->origin[1];
		orgs[2] = ent->origin[2];
		for (cache = vcm.activeVertCache.next; cache != &vcm.activeVertCache; cache = next)
		{
			next = cache->next;
			if (backlerp)
			{	// анимированным моделям ОПАСНО шарить один и тот же VBO-кэш, т.к. они могут иметь и разные oldangles, oldorigin и oldframe. Отсюда следует, что геометрия будет немного отличаться. Поэтому данное кэширование будет действовать в пределах одного entity в пределах одного кадра. Выигрыш заметнее в местах с многими источниками света.
				if (cache->store == store && cache->mod == mod && cache->frame == frame && cache->mesh == mesh && cache->backlerp == backlerp && cache->angles[0] == angles[0] && cache->angles[1] == angles[1] && cache->angles[2] == angles[2] && cache->origin[0] == orgs[0] && cache->origin[1] == orgs[1] && cache->origin[2] == orgs[2])
				{	// already cached!
					GL_BindVBO(cache);
					return cache;
				}
			}
			else
			{	// для статичных моделей всё проще...
				if (cache->store == store && cache->mod == mod && cache->frame == frame && cache->mesh == mesh && cache->angles[0] == angles[0] && cache->angles[1] == angles[1] && cache->angles[2] == angles[2])
				{	// already cached!
					GL_BindVBO(cache);
					return cache;
				}
			}
		}
	}
	else if (store == VBO_STORE_NORMAL || store == VBO_STORE_BINORMAL || store == VBO_STORE_TANGENT)
	{
		mod = ent->model;
		frame = ent->frame;

		backlerp = ent->backlerp;

		for (cache = vcm.activeVertCache.next; cache != &vcm.activeVertCache; cache = next)
		{
			next = cache->next;
			if (cache->store == store && cache->mod == mod && cache->frame == frame && cache->mesh == mesh && cache->backlerp == backlerp)
			{	// already cached!
				GL_BindVBO(cache);
				return cache;
			}
		}
	}

	return NULL;
}


vertCache_t *R_VCLoadData(vertCacheMode_t mode, int size, void *buffer, vertStoreMode_t store, entity_t *ent, int mesh)
{
	vertCache_t *cache;

	if (!vcm.freeVertCache)
		Com_Error(ERR_FATAL, "VBO cache overflow\n");

	cache = vcm.freeVertCache;
	cache->mode = mode;
	cache->size = size;
	cache->pointer = buffer;
	cache->store = store;
	if (store != VBO_STORE_ANY)
	{
		cache->mod = ent->model;
		cache->mesh = mesh;
		cache->frame = ent->frame;
		cache->backlerp = ent->backlerp;
		cache->angles[0] = ent->angles[0];
		cache->angles[1] = ent->angles[1];
		cache->angles[2] = ent->angles[2];
		cache->origin[0] = ent->origin[0];
		cache->origin[1] = ent->origin[1];
		cache->origin[2] = ent->origin[2];
	}

	// link
	vcm.freeVertCache = vcm.freeVertCache->next;

	cache->next = vcm.activeVertCache.next;
	cache->prev = &vcm.activeVertCache;

	vcm.activeVertCache.next->prev = cache;
	vcm.activeVertCache.next = cache;

	GL_BindVBO(cache);

	switch (cache->mode)
	{
		case VBO_STATIC:
			qglBufferData(GL_ARRAY_BUFFER_ARB, cache->size, cache->pointer, GL_STATIC_DRAW_ARB);
		case VBO_DYNAMIC:
			qglBufferData(GL_ARRAY_BUFFER_ARB, cache->size, cache->pointer, GL_DYNAMIC_DRAW_ARB);
	}

	return cache;
}


vertCache_t *R_VCCreate(vertCacheMode_t mode, int size, void *buffer, vertStoreMode_t store, entity_t *ent, int mesh)
{
	vertCache_t	*cache;

	cache = R_VCFindCache(store, ent, mesh);
	if (cache)
		return cache;

	return R_VCLoadData(mode, size, buffer, store, ent, mesh);
}


void R_VCFree(vertCache_t *cache)
{
	if (!cache)
		return;

	// unlink
	cache->prev->next = cache->next;
	cache->next->prev = cache->prev;

	cache->next = vcm.freeVertCache;
	vcm.freeVertCache = cache;
}


void R_VCInit()
{
	int	i;

	if (!gl_state.vbo)
		return;

	Com_Printf("   Initializing VBO cache\n");
	vc_initialised = true;

	memset(&vcm, 0, sizeof(vcm));

	// setup the linked lists
	vcm.activeVertCache.next = &vcm.activeVertCache;
	vcm.activeVertCache.prev = &vcm.activeVertCache;

	vcm.freeVertCache = vcm.vertCacheList;

	for (i=0; i<MAX_VERTEX_CACHES-1; i++)
		vcm.vertCacheList[i].next = &vcm.vertCacheList[i+1];

	// Создадим хэндлы для всех VBO
	for (i=0; i<MAX_VERTEX_CACHES; i++)
		qglGenBuffers(1, &vcm.vertCacheList[i].id);

}

/*
===============
R_VCFreeFrame

Вызывается ПЕРЕД рисованием кадра со всеми зеркалами и ПОСЛЕ определения всех entity.
Deletes all non-STATIC buffers from the previous frame.
PS: VBO_STATIC using for ST texture/skin coordinates and static shadow volumes
===============
*/
void R_VCFreeFrame()
{
	vertCache_t	*cache, *next;

	if (!gl_state.vbo)
		return;

	for (cache = vcm.activeVertCache.next; cache != &vcm.activeVertCache; cache = next)
	{
		next = cache->next;
		if (cache->mode != VBO_STATIC)
			R_VCFree(cache);
	}
}

void R_VCShutdown()
{
	int			i;
	vertCache_t	*cache, *next;

	if (!gl_state.vbo)
		return;

	if (!vc_initialised)
		return;
	vc_initialised = false;

	for (cache = vcm.activeVertCache.next; cache != &vcm.activeVertCache; cache = next)
	{
		next = cache->next;
		R_VCFree(cache);
	}

	// Освободим хэндлы всех VBO
	for (i=0; i<MAX_VERTEX_CACHES; i++)
		qglDeleteBuffers(1, &vcm.vertCacheList[i].id);
}


//======================================================================================
void R_Register2(void)
{
	
	r_shadows =							Cvar_Get("r_shadows", "1", CVAR_ARCHIVE);
	r_shadowWorldLightScale =			Cvar_Get("r_shadowWorldLightScale", "10", CVAR_ARCHIVE);
	r_shadowVolumesDebug =				Cvar_Get("r_shadowVolumesDebug", "0", 0);
	r_playerShadow =					Cvar_Get("r_playerShadow", "1", CVAR_ARCHIVE);
	

	r_anisotropic =						Cvar_Get("r_anisotropic", "1", CVAR_ARCHIVE);
	r_maxAnisotropy =					Cvar_Get("r_maxAnisotropy", "0", 0);
	
	r_causticIntens =					Cvar_Get("r_causticIntens", "1.5", CVAR_ARCHIVE);
	r_displayRefresh =					Cvar_Get("r_displayRefresh", "0", CVAR_ARCHIVE);
	
	
	r_screenShot =						Cvar_Get("r_screenShot", "jpg", CVAR_ARCHIVE);
	r_screenShotJpegQuality =			Cvar_Get("r_screenShotJpegQuality", "99", CVAR_ARCHIVE);
	
	r_overBrightBits =					Cvar_Get("r_overBrightBits", "2", CVAR_ARCHIVE);
	r_textureCompression =				Cvar_Get("r_textureCompression", "0", CVAR_ARCHIVE);														
		
	r_radarSize =						Cvar_Get("r_radarSize", "256", CVAR_ARCHIVE);
	r_radarZoom =						Cvar_Get("r_radarZoom", "1", CVAR_ARCHIVE);
	r_radar =							Cvar_Get("r_radar", "0", CVAR_ARCHIVE);
	
	r_arbSamples =						Cvar_Get("r_arbSamples", "1", CVAR_ARCHIVE);
	r_nvMultisampleFilterHint =			Cvar_Get ("r_nvMultisampleFilterHint", "fastest", CVAR_ARCHIVE);
	r_useCSAA =							Cvar_Get("r_useCSAA", "0", CVAR_ARCHIVE);
	r_nvSamplesCoverange =				Cvar_Get("r_nvSamplesCoverange", "8", CVAR_ARCHIVE);

	deathmatch =						Cvar_Get("deathmatch", "0", CVAR_SERVERINFO);
	
	r_drawFlares =						Cvar_Get("r_drawFlares", "1", CVAR_ARCHIVE);
	r_flaresIntens =					Cvar_Get("r_flaresIntens", "3", CVAR_ARCHIVE);
	r_flareWeldThreshold =				Cvar_Get("r_flareWeldThreshold", "32", CVAR_ARCHIVE);
	
	r_customWidth =						Cvar_Get("r_customWidth", "1024", CVAR_ARCHIVE);
	r_customHeight =					Cvar_Get("r_customHeight", "500", CVAR_ARCHIVE);
	
	r_bloom =							Cvar_Get("r_bloom", "0", CVAR_ARCHIVE);
	r_bloomThreshold=					Cvar_Get("r_bloomThreshold", "0.5", CVAR_ARCHIVE);
	r_bloomIntens=						Cvar_Get("r_bloomIntens", "0.35", CVAR_ARCHIVE);

	sys_priority =						Cvar_Get("sys_priority", "0", CVAR_ARCHIVE);
	sys_affinity =						Cvar_Get("sys_affinity", "0", CVAR_ARCHIVE);
		
	r_DrawRangeElements	=				Cvar_Get("r_DrawRangeElements","1",CVAR_ARCHIVE);

	r_pplWorldAmbient = 				Cvar_Get("r_pplWorldAmbient", "0.15", CVAR_ARCHIVE);
	r_bumpMapping =						Cvar_Get("r_bumpMapping", "1", CVAR_ARCHIVE);
	r_ambientLevel =					Cvar_Get("r_ambientLevel", "0.75", CVAR_ARCHIVE);
	
	hunk_bsp=							Cvar_Get("hunk_bsp", "20", CVAR_ARCHIVE);
	hunk_model=							Cvar_Get("hunk_model", "8", CVAR_ARCHIVE);
	hunk_sprite=						Cvar_Get("hunk_sprite", "1", CVAR_ARCHIVE);
	
	r_vbo=								Cvar_Get("r_vbo", "1", CVAR_ARCHIVE);

	r_maxTextureSize=					Cvar_Get("r_maxTextureSize", "0", CVAR_ARCHIVE);

	r_parallax=							Cvar_Get("r_parallax", "1", CVAR_ARCHIVE);
	r_parallaxScale=					Cvar_Get("r_parallaxScale", "0.5", CVAR_ARCHIVE);
	r_parallaxSteps=					Cvar_Get("r_parallaxSteps", "10", CVAR_ARCHIVE);

	r_dof=								Cvar_Get("r_dof", "1", CVAR_ARCHIVE);
	r_dofBias=							Cvar_Get("r_dofBias", "0.002", CVAR_ARCHIVE);
	r_dofFocus=							Cvar_Get("r_dofFocus", "256.0", CVAR_ARCHIVE);

	r_radialBlur=						Cvar_Get("r_radialBlur", "1", CVAR_ARCHIVE);
	r_radialBlurFov=                    Cvar_Get("r_radialBlurFov", "30", CVAR_ARCHIVE);
	r_radialBlurSamples=                Cvar_Get("r_radialBlurSamples", "8", CVAR_ARCHIVE);
}

