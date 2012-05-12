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
// GL_RSURF.C: surface-related refresh code
//#include <assert.h>

#include "r_local.h"

vec4_t shadelight_surface;
void R_LightColor(vec3_t org, vec3_t color);

static vec3_t modelorg;			// relative to viewpoint

msurface_t *r_alpha_surfaces;

float color_black[4] = {0.0, 0.0, 0.0, 0.0};

extern void R_SetCacheState(msurface_t * surf);
extern void R_BuildLightMap(msurface_t * surf, byte * dest, int stride, qboolean loadModel);

vec3_t  wVertexArray[MAX_BATCH_SURFS];
float   wTexArray[MAX_BATCH_SURFS][2];
float   wLMArray[MAX_BATCH_SURFS][2];

float   wTmu0Array[MAX_BATCH_SURFS][2];
float   wTmu1Array[MAX_BATCH_SURFS][2];
float   wTmu2Array[MAX_BATCH_SURFS][2];
float   wTmu3Array[MAX_BATCH_SURFS][2];
float   wTmu4Array[MAX_BATCH_SURFS][2];
float   wTmu5Array[MAX_BATCH_SURFS][2];
float   wTmu6Array[MAX_BATCH_SURFS][2];
float	SurfColorArray[MAX_BATCH_SURFS][4];

int		indexArray[MAX_BATCH_SURFS*3];

vec3_t	nTexArray[MAX_BATCH_SURFS];
vec3_t	tTexArray[MAX_BATCH_SURFS];
vec3_t	bTexArray[MAX_BATCH_SURFS];

/*
=============================================================

	BRUSH MODELS

=============================================================
*/

/*
===============
R_TextureAnimation

Returns the proper texture for a given time and base texture
===============
*/
image_t *R_TextureAnimation(mtexinfo_t * tex)
{
	int c;

	if (!tex->next)
		return tex->image;

	c = currententity->frame % tex->numframes;
	while (c) {
		tex = tex->next;
		c--;
	}

	return tex->image;
}

image_t *R_TextureAnimationHm(mtexinfo_t * tex)
{
	int c;

	if (!tex->next)
		return tex->hiMap;

	c = currententity->frame % tex->numframes;
	while (c) {
		tex = tex->next;
		c--;
	}

	return tex->hiMap;
}

image_t *R_TextureAnimationFx(mtexinfo_t * tex)
{
	int c;

	if (!tex->next)
		return tex->addTexture;

	c = currententity->frame % tex->numframes;
	while (c) {
		tex = tex->next;
		c--;
	}

	return tex->addTexture;
}

image_t *R_TextureAnimationNormal(mtexinfo_t * tex)
{
	int c;

	if (!tex->next)
		return tex->normalmap;

	c = currententity->frame % tex->numframes;
	while (c) {
		tex = tex->next;
		c--;
	}

	return tex->normalmap;
}


/*
================
DrawGLPoly
================
*/

void DrawGLPolyGLSL(msurface_t * fa)
{
	int i;
	float *v;
	float alpha;
	glpoly_t *p;
	int	id, nv = fa->polys->numverts;
	unsigned	defBits = 0;
	
	if (fa->texinfo->flags & SURF_TRANS33)
		alpha = 0.33;
	else 
		alpha = 0.66;
	
	// setup program
	GL_BindProgram(refractProgram, defBits);
	id = refractProgram->id[defBits];

	qglEnableVertexAttribArray(ATRB_POSITION);
	qglEnableVertexAttribArray(ATRB_TEX0);
	qglEnableVertexAttribArray(ATRB_COLOR);

	qglVertexAttribPointer(ATRB_POSITION, 3, GL_FLOAT, false, 0, wVertexArray);	
	qglVertexAttribPointer(ATRB_TEX0, 2, GL_FLOAT, false, 0, wTexArray);
	qglVertexAttribPointer(ATRB_COLOR, 4, GL_FLOAT, false, 0, SurfColorArray);

	GL_MBind(GL_TEXTURE0_ARB, fa->texinfo->normalmap->texnum);
	qglUniform1i(qglGetUniformLocation(id, "u_deformMap"), 0);
	GL_MBind(GL_TEXTURE1_ARB, fa->texinfo->image->texnum);
	qglUniform1i(qglGetUniformLocation(id, "u_colorMap"), 1);
	GL_MBindRect(GL_TEXTURE2_ARB, ScreenMap->texnum);
	qglUniform1i(qglGetUniformLocation(id, "g_colorBufferMap"), 2);
	GL_MBindRect(GL_TEXTURE3_ARB, depthMap->texnum);
	qglUniform1i(qglGetUniformLocation(id, "g_depthBufferMap"), 3);

	qglUniform1f(qglGetUniformLocation(id, "u_deformMul"),	1.0);
	qglUniform1f(qglGetUniformLocation(id, "u_alpha"),	alpha);
	qglUniform1f(qglGetUniformLocation(id, "u_thickness"),	150.0);
	qglUniform2f(qglGetUniformLocation(id, "u_viewport"),	vid.width, vid.height);
	qglUniform2f(qglGetUniformLocation(id, "u_depthParms"), r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);
	
	p = fa->polys;
	v = p->verts[0];

	c_brush_polys += (nv-2);

	for (i = 0; i < p->numverts; i++, v += VERTEXSIZE) {
		
		VectorCopy(v, wVertexArray[i]);
			
		wTexArray[i][0] = v[3];
		wTexArray[i][1] = v[4];

		R_LightColor	(v, shadelight_surface);
		VA_SetElem4		(SurfColorArray[i],	shadelight_surface[0], 
											shadelight_surface[1], 
											shadelight_surface[2], 
											alpha);	

	}
	qglDrawElements(GL_TRIANGLES, fa->numIndices, GL_UNSIGNED_SHORT, fa->indices);	
			
	qglDisableVertexAttribArray(ATRB_POSITION);
	qglDisableVertexAttribArray(ATRB_TEX0);
	qglDisableVertexAttribArray(ATRB_COLOR);
	GL_SelectTexture(GL_TEXTURE0_ARB);
	GL_BindNullProgram();
	
}




//============
//PGM
/*
================
DrawGLFlowingPoly -- version of DrawGLPoly that handles scrolling texture
================
*/
void DrawGLFlowingPolyGLSL(msurface_t * fa)
{
	int i;
	float *v;
	float alpha, scroll;
	glpoly_t *p;
	int	id, nv = fa->polys->numverts;
	unsigned	defBits = 0;

	if (fa->texinfo->flags & SURF_TRANS33)
		alpha = 0.33;
	else 
		alpha = 0.66;

	// setup program
	GL_BindProgram(refractProgram, defBits);
	id = refractProgram->id[defBits];

	qglEnableVertexAttribArray(ATRB_POSITION);
	qglEnableVertexAttribArray(ATRB_TEX0);
	qglEnableVertexAttribArray(ATRB_COLOR);

	qglVertexAttribPointer(ATRB_POSITION, 3, GL_FLOAT, false, 0, wVertexArray);	
	qglVertexAttribPointer(ATRB_TEX0, 2, GL_FLOAT, false, 0, wTexArray);
	qglVertexAttribPointer(ATRB_COLOR, 4, GL_FLOAT, false, 0, SurfColorArray);

	GL_MBind(GL_TEXTURE0_ARB, fa->texinfo->normalmap->texnum);
	qglUniform1i(qglGetUniformLocation(id, "u_deformMap"), 0);
	GL_MBind(GL_TEXTURE1_ARB, fa->texinfo->image->texnum);
	qglUniform1i(qglGetUniformLocation(id, "u_colorMap"), 1);
	GL_MBindRect(GL_TEXTURE2_ARB, ScreenMap->texnum);
	qglUniform1i(qglGetUniformLocation(id, "g_colorBufferMap"), 2);
	GL_MBindRect(GL_TEXTURE3_ARB, depthMap->texnum);
	qglUniform1i(qglGetUniformLocation(id, "g_depthBufferMap"), 3);

	qglUniform1f(qglGetUniformLocation(id, "u_deformMul"),	1.0);
	qglUniform1f(qglGetUniformLocation(id, "u_alpha"),	alpha);
	qglUniform1f(qglGetUniformLocation(id, "u_thickness"),	300.000);
	qglUniform2f(qglGetUniformLocation(id, "u_viewport"),	vid.width, vid.height);
	qglUniform2f(qglGetUniformLocation(id, "u_depthParms"), r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);
		

	p = fa->polys;
	v = p->verts[0];

	c_brush_polys += (nv-2);
	
	scroll =-64 * ((r_newrefdef.time / 40.0) - (int) (r_newrefdef.time / 40.0));

	if (scroll == 0.0)
		scroll = -64.0;

	for (i = 0; i < p->numverts; i++, v += VERTEXSIZE) {
		
		VectorCopy(v, wVertexArray[i]);
			
		wTmu0Array[i][0] = v[3]+scroll;
		wTmu0Array[i][1] = v[4];

		wTmu1Array[i][0] = v[3];
		wTmu1Array[i][1] = v[4];

		wTmu2Array[i][0] = v[3]+scroll;
		wTmu2Array[i][1] = v[4];

		R_LightColor	(v, shadelight_surface);
		VA_SetElem4		(SurfColorArray[i],	shadelight_surface[0], 
											shadelight_surface[1], 
											shadelight_surface[2], 
											alpha);	
	}
	qglDrawElements(GL_TRIANGLES, fa->numIndices, GL_UNSIGNED_SHORT, fa->indices);
		
	qglDisableVertexAttribArray(ATRB_POSITION);
	qglDisableVertexAttribArray(ATRB_TEX0);
	qglDisableVertexAttribArray(ATRB_COLOR);
	GL_SelectTexture(GL_TEXTURE0_ARB);
	GL_BindNullProgram();
}




/*
================
R_RenderBrushPoly
================
*/

void R_RenderBrushPoly (msurface_t *fa)
{
	image_t *image;
	char *purename;
	char noext[MAX_QPATH];
	
	image = R_TextureAnimation(fa->texinfo);
	purename = COM_SkipPath(image->name);
	COM_StripExtension(purename, noext);

	if (fa->flags & SURF_DRAWTURB)
	{	

		if (!strcmp(noext, "brlava")|| !strcmp(noext, "lava") || !strcmp(noext, "tlava1_3"))
			RenderLavaSurfaces(fa);
		else
			R_DrawWaterPolygons(fa);
		return;
	}

	if(fa->texinfo->flags & SURF_FLOWING)
		DrawGLFlowingPolyGLSL(fa);
	else
		DrawGLPolyGLSL(fa);

}


void R_DrawAlphaPoly(void)
{
	msurface_t *s;

	//
	// go back to the world matrix
	//

	qglLoadMatrixf(r_world_matrix);
	qglDepthMask(0);
	
	for (s = r_alpha_surfaces; s; s = s->texturechain) {
//		GL_Bind(s->texinfo->image->texnum);
//		c_brush_polys++;

		// moving trans brushes - spaz
		if (s->ent) {
			s->ent->angles[0] = -s->ent->angles[0];	// stupid quake bug
			s->ent->angles[2] = -s->ent->angles[2];	// stupid quake bug
			R_RotateForEntity(s->ent);
			s->ent->angles[0] = -s->ent->angles[0];	// stupid quake bug
			s->ent->angles[2] = -s->ent->angles[2];	// stupid quake bug
		}

		if (s->texinfo->flags & SURF_TRANS33) 
			shadelight_surface[3] = 0.33;
		else	
		if (s->texinfo->flags & SURF_TRANS66) 
			shadelight_surface[3] = 0.66;
		else
			shadelight_surface[3] = 1.0;
		

		if (s->flags & SURF_DRAWTURB)
			R_DrawWaterPolygons(s);
		else if (s->texinfo->flags & SURF_FLOWING)
			DrawGLFlowingPolyGLSL(s);
		else
			DrawGLPolyGLSL(s);

	}

	qglColor4f(1, 1, 1, 1);
	qglDepthMask(1);
	r_alpha_surfaces = NULL;
}





/*
================
DrawTextureChains
================
*/
void DrawTextureChains(void)
{
	int i;
	msurface_t *s;
	image_t *image;

	c_visible_textures = 0;

	for (i = 0, image = gltextures; i < numgltextures; i++, image++) {
			if (!image->registration_sequence)
				continue;
			s = image->texturechain;
			if (!s)
				continue;

			for (; s; s = s->texturechain) {
					R_RenderBrushPoly(s);
			}
		
			image->texturechain = NULL;
		}


}


/*
===============
SORT AND BATCH 
BSP SURFACES
===============
*/

void GL_CreateParallaxLmPoly(msurface_t * surf)
	{
	int i, nv = surf->polys->numverts;
	float *v;
	glpoly_t *p = surf->polys;
	float scroll;
	
	scroll = -64 * ((r_newrefdef.time / 40.0) - (int) (r_newrefdef.time / 40.0));
		
	if (scroll == 0.0)
		scroll = -64.0;

			v = p->verts[0];
			c_brush_polys += (nv-2);

			for (i = 0; i < nv; i++, v += VERTEXSIZE) 
				{
				VectorCopy(v, wVertexArray[i]);
				//baseTex
				if (surf->texinfo->flags & SURF_FLOWING){
				wTexArray[i][0] = v[3]+scroll;
				}else{
				wTexArray[i][0] = v[3];
				}
				wTexArray[i][1] = v[4];
				//lightMap
				wLMArray[i][0]  = v[5];
				wLMArray[i][1]  = v[6];

				if(r_bumpWorld->value || r_parallax->value){
				//normals
				nTexArray[i][0] = v[7];
				nTexArray[i][1] = v[8];
				nTexArray[i][2] = v[9];
				//tangents
				tTexArray[i][0] = v[10];
				tTexArray[i][1] = v[11];
				tTexArray[i][2] = v[12];
				//binormals
				bTexArray[i][0] = v[13];
				bTexArray[i][1] = v[14];
				bTexArray[i][2] = v[15];
				}
			}
	
}

vec3_t BmodelViewOrg;

 int SurfSort( const msurface_t **a, const msurface_t **b )
{
	return	(((*a)->lightmaptexturenum<<26)+((*a)->texinfo->image->texnum<<13) + (*a)->texinfo->normalmap->texnum) - 
			(((*b)->lightmaptexturenum<<26)+((*b)->texinfo->image->texnum<<13) + (*b)->texinfo->normalmap->texnum);
}


int	num_scene_surfaces;
msurface_t	*scene_surfaces[MAX_MAP_FACES];
vec3_t	lightOrg, lightColor;
float lightRad;
int r_currTex = -9999;

static void GL_BatchLightmappedPoly(qboolean bmodel, qboolean caustics)
{
	msurface_t	*s;
	image_t		*image, *fx, *nm;
	unsigned	lmtex;
	unsigned	defBits = 0;
	int		id, i, map, j;
	float		scale[2];
	qboolean	is_dynamic = false;
	dlight_t	*dl;
	unsigned	temp[128 * 128];
	int		smax, tmax, counter=0;

	defBits = worldDefs.LightmapBits;

	if (r_parallax->value)
		defBits |= worldDefs.ParallaxBit;
	
	if (r_bumpWorld->value)
		defBits |= worldDefs.BumpBits;

	// setup program
	GL_BindProgram(diffuseProgram, defBits);
	id = diffuseProgram->id[defBits];

	qglUniform1f(qglGetUniformLocation(id, "u_ColorModulate"), r_worldColorScale->value);

	if(r_bumpWorld->value || r_parallax->value){
	if(bmodel)
		qglUniform3fv(qglGetUniformLocation(id, "u_viewOriginES"), 1 , BmodelViewOrg);
	else
		qglUniform3fv(qglGetUniformLocation(id, "u_viewOriginES"), 1 , r_origin);
	}

	if(r_parallax->value){
	qglUniform1i(qglGetUniformLocation(id, "u_parallaxType"), (int)r_parallax->value);
	}

	if(r_bumpWorld->value){

	qglUniform1f(qglGetUniformLocation(id, "u_ambientScale"), r_pplWorldAmbient->value);
	
	// cleanup lights
	for (j=0; j < r_pplMaxDlights->value; j++) {
		char uname[32];
		
		if(counter < 0){
		qglUniform1i(qglGetUniformLocation(id, "u_activeLights"), 0);
		break;
		}
		Com_sprintf(uname, sizeof(uname), "u_LightRadius[%i]", j);
		qglUniform1f(qglGetUniformLocation(id, uname), -1.0);
		counter--;
	}
	// setup dlights
	dl = r_newrefdef.dlights;
	for (j=0, dl = r_newrefdef.dlights; j < r_newrefdef.num_dlights; j++, dl++ ) 
	{
		char uname[32];
		int k;
		vec3_t mins, maxs;
		vec3_t locLight;

		for (k=0 ; k<3 ; k++)
		{
		mins[k] = dl->origin[k] - dl->intensity * 0.666;
		maxs[k] = dl->origin[k] + dl->intensity * 0.666;
		}

		if(R_CullBox(mins, maxs))
			continue;
			
		if(j >= r_pplMaxDlights->value )
			break;
				
	// put dlight into model space for bmodels
	if(bmodel){
	if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2])
		{
		mat3_t	entityAxis;
		vec3_t	tmp;
		VectorSubtract(r_origin, currententity->origin, tmp);
		AnglesToMat3(currententity->angles, entityAxis);
		Mat3_TransposeMultiplyVector(entityAxis, tmp, locLight);
		}
	else
		VectorSubtract(dl->origin, currententity->origin, locLight);
	
		Com_sprintf(uname, sizeof(uname), "u_LightOrg[%i]", j);
		qglUniform3f(qglGetUniformLocation(id, uname), locLight[0], locLight[1], locLight[2]);

	}else{
		Com_sprintf(uname, sizeof(uname), "u_LightOrg[%i]", j);
		qglUniform3f(qglGetUniformLocation(id, uname), dl->origin[0], dl->origin[1], dl->origin[2]);
	}
		Com_sprintf(uname, sizeof(uname), "u_LightColor[%i]", j);
		qglUniform3f(qglGetUniformLocation(id, uname), dl->color[0], dl->color[1], dl->color[2]);
		Com_sprintf(uname, sizeof(uname), "u_LightRadius[%i]", j);
		qglUniform1f(qglGetUniformLocation(id, uname), dl->intensity);
		qglUniform1i(qglGetUniformLocation(id, "u_numLights"), r_newrefdef.num_dlights);
		qglUniform1i(qglGetUniformLocation(id, "u_activeLights"), 1);
		counter ++;
		}
	}

	qsort(scene_surfaces, num_scene_surfaces, sizeof(msurface_t*), (int (*)(const void *, const void *))SurfSort);

	for (i=0; i<num_scene_surfaces; i++)
	{
		s=scene_surfaces[i];
		image = R_TextureAnimation(s->texinfo);
		fx    = R_TextureAnimationFx(s->texinfo);
		nm    = R_TextureAnimationNormal(s->texinfo);
		lmtex = s->lightmaptexturenum;

		if(r_parallax->value){
			scale[0] = r_parallaxScale->value / image->width;
			scale[1] = r_parallaxScale->value / image->height;
			qglUniform2f(qglGetUniformLocation(id, "u_parallaxScale"), scale[0], scale[1]);
			qglUniform2f(qglGetUniformLocation(id, "u_texSize"), image->upload_width, image->upload_height);
		}

		if(caustics || (s->flags & SURF_WATER)){
			qglUniform1f(qglGetUniformLocation(id, "u_CausticsModulate"), r_causticIntens->value);
			qglUniform1i(qglGetUniformLocation(id, "u_isCaustics"), 1);
		}else
			qglUniform1i(qglGetUniformLocation(id, "u_isCaustics"), 0);

		GL_CreateParallaxLmPoly(s);

		for (map = 0; map < MAXLIGHTMAPS && s->styles[map] != 255; map++) {
			if (r_newrefdef.lightstyles[s->styles[map]].white != s->cached_light[map])
				goto dynamic;
		}

		// dynamic this frame or dynamic previously
		if ((s->dlightframe == r_framecount)) {
        
		dynamic:
			if (r_dynamic->value) {
                
                if (!(s->texinfo-> flags &
				  (SURF_SKY | SURF_TRANS33 | SURF_TRANS66 | SURF_WARP))) {
					is_dynamic = true;
				}
			}
		}

		if (is_dynamic) {
			if ((s->styles[map] >= 32 || s->styles[map] == 0)
				&& (s->dlightframe != r_framecount)) {
				smax = (s->extents[0] / r_worldmodel->lightmap_scale) + 1;
				tmax = (s->extents[1] / r_worldmodel->lightmap_scale) + 1;

				R_BuildLightMap(s, (byte *) temp, smax * 4, false);
				R_SetCacheState(s);

				GL_MBind(GL_TEXTURE1_ARB, gl_state.lightmap_textures + s->lightmaptexturenum);

				lmtex = s->lightmaptexturenum;

				qglTexSubImage2D(GL_TEXTURE_2D, 0,
								s->light_s, s->light_t,
								smax, tmax,
								GL_LIGHTMAP_FORMAT, GL_UNSIGNED_INT_8_8_8_8_REV, temp); 

			} else {
				smax = (s->extents[0] / r_worldmodel->lightmap_scale) + 1;
				tmax = (s->extents[1] / r_worldmodel->lightmap_scale) + 1;

				R_BuildLightMap(s, (byte *) temp, smax * 4, false);

				GL_MBind(GL_TEXTURE1_ARB, gl_state.lightmap_textures + 0);

				lmtex = 0;

				qglTexSubImage2D(GL_TEXTURE_2D, 0, s->light_s, s->light_t, smax, tmax, GL_LIGHTMAP_FORMAT, GL_UNSIGNED_INT_8_8_8_8_REV, temp); 
			
			}
			
			if(s->texinfo->image->texnum != r_currTex) 
			{
				GL_MBind(GL_TEXTURE0_ARB, image->texnum);
				qglUniform1i(qglGetUniformLocation(id, "u_Diffuse"), 0);
				GL_MBind(GL_TEXTURE2_ARB, fx->texnum);
				qglUniform1i(qglGetUniformLocation(id, "u_Add"), 2);

				if(caustics || (s->flags & SURF_WATER)){
					GL_MBind(GL_TEXTURE3_ARB, r_caustic[((int) (r_newrefdef.time * 15)) & (MAX_CAUSTICS - 1)]->texnum);
					qglUniform1i(qglGetUniformLocation(id, "u_Caustics"), 3);
				}
				if(r_bumpWorld->value){
					GL_MBind(GL_TEXTURE4_ARB, nm->texnum);
					qglUniform1i(qglGetUniformLocation(id, "u_NormalMap"), 4);
				}
			}
			GL_MBind(GL_TEXTURE1_ARB, gl_state.lightmap_textures + lmtex);
			qglUniform1i(qglGetUniformLocation(id, "u_LightMap"), 1);
			
			qglDrawElements(GL_TRIANGLES, s->numIndices, GL_UNSIGNED_SHORT, s->indices);
		} else {
		
			if(s->texinfo->image->texnum != r_currTex) 
			{
				GL_MBind(GL_TEXTURE0_ARB, image->texnum);
				qglUniform1i(qglGetUniformLocation(id, "u_Diffuse"), 0);
				GL_MBind(GL_TEXTURE1_ARB, gl_state.lightmap_textures + lmtex);
				qglUniform1i(qglGetUniformLocation(id, "u_LightMap"), 1);
				GL_MBind(GL_TEXTURE2_ARB, fx->texnum);
				qglUniform1i(qglGetUniformLocation(id, "u_Add"), 2);
				
				if(caustics || (s->flags & SURF_WATER)){
					GL_MBind(GL_TEXTURE3_ARB, r_caustic[((int) (r_newrefdef.time * 15)) & (MAX_CAUSTICS - 1)]->texnum);
					qglUniform1i(qglGetUniformLocation(id, "u_Caustics"), 3);
				}
				if(r_bumpWorld->value){
					GL_MBind(GL_TEXTURE4_ARB, nm->texnum);
					qglUniform1i(qglGetUniformLocation(id, "u_NormalMap"), 4);
					qglUniform1i(qglGetUniformLocation(id, "u_deluxMap"), 5);
				}
			}
			qglDrawElements(GL_TRIANGLES, s->numIndices, GL_UNSIGNED_SHORT, s->indices);	
		}
		r_currTex = s->texinfo->image->texnum;
	}
	
	GL_BindNullProgram();
}


/*
=================
R_DrawInlineBModel
=================
*/
extern qboolean bmodelcaust = false;

void R_DrawBrushModel(entity_t * e);

static void R_DrawInlineBModel(void)
{
	int i, k;
	cplane_t *pplane;
	float dot;
	msurface_t *psurf;
	dlight_t *lt;
	
	// calculate dynamic lighting for bmodel
	lt = r_newrefdef.dlights;
		for (k = 0; k < r_newrefdef.num_dlights; k++, lt++) {
			R_MarkLights(lt, 1 << k,
						 currentmodel->nodes + currentmodel->firstnode);
		}
	

	psurf = &currentmodel->surfaces[currentmodel->firstmodelsurface];

	//
	// draw texture
	//
	for (i = 0; i < currentmodel->nummodelsurfaces; i++, psurf++) {
		// find which side of the node we are on
		pplane = psurf->plane;
		dot = DotProduct(modelorg, pplane->normal) - pplane->dist;

		// draw the polygon
		if (((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON))
			|| (!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON))) {

			if (psurf->visframe == r_framecount) //reckless fix
				continue;

			/*===============================
			berserker - flares for brushmodels
			=================================*/
			psurf->visframe = r_framecount;
			psurf->ent = currententity;
			// ================================

			if (psurf->texinfo->flags & (SURF_TRANS33 | SURF_TRANS66)) {	
				psurf->texturechain = r_alpha_surfaces;
				r_alpha_surfaces = psurf;

			} else if (!(psurf->flags & SURF_DRAWTURB)) {
				
				scene_surfaces[num_scene_surfaces++] = psurf;
								
			} 
		}
	}

	if (!(currententity->flags & RF_TRANSLUCENT)) {
		
		qglDisable(GL_BLEND); 
		qglColor4f(1, 1, 1, 1);
	}

	qglDisable(GL_BLEND);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


static void R_DrawInlineBModel2(void)
{
	int i;
	cplane_t *pplane;
	float dot;
	msurface_t *psurf;
		
	psurf = &currentmodel->surfaces[currentmodel->firstmodelsurface];
	//
	// draw texture
	//
	for (i = 0; i < currentmodel->nummodelsurfaces; i++, psurf++) {
		// find which side of the node we are on
		pplane = psurf->plane;
		dot = DotProduct(modelorg, pplane->normal) - pplane->dist;

		// draw the polygon
		if (((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON))
			|| (!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON))) {

			if (psurf->visframe == r_framecount) //reckless fix
				continue;
		
			if (psurf->texinfo->flags & (SURF_TRANS33 | SURF_TRANS66)) {	
				continue;
			} 
			else 
				if (psurf->flags & SURF_DRAWTURB)
						R_RenderBrushPoly(psurf);
						
		}
	}

	qglDisable(GL_BLEND);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


/*
=================
R_DrawBrushModel
=================
*/
int CL_PMpointcontents(vec3_t point);

void R_DrawBrushModel(entity_t * e)
{
	vec3_t		mins, maxs;
	int			i;
	int         contentsAND, contentsOR; 
    qboolean	rotated, viewInWater;
	int			cont[5];
	vec3_t		org, tmp;
	mat3_t		entityAxis;

	if (currentmodel->nummodelsurfaces == 0)
		return;

	currententity = e;
	gl_state.currenttextures[0] = gl_state.currenttextures[1] = -1;

	if (e->angles[0] || e->angles[1] || e->angles[2]) {
		rotated = true;
		for (i = 0; i < 3; i++) {
			mins[i] = e->origin[i] - currentmodel->radius;
			maxs[i] = e->origin[i] + currentmodel->radius;
		}
	} else {
		rotated = false;
		VectorAdd(e->origin, currentmodel->mins, mins);
		VectorAdd(e->origin, currentmodel->maxs, maxs);
	}

	if (R_CullBox(mins, maxs))
		return;

	qglColor3f(1, 1, 1);
	memset(gl_lms.lightmap_surfaces, 0, sizeof(gl_lms.lightmap_surfaces));


	VectorSubtract(r_newrefdef.vieworg, e->origin, modelorg);

	if (rotated) {
		vec3_t temp;
		vec3_t forward, right, up;

		VectorCopy(modelorg, temp);
		AngleVectors(e->angles, forward, right, up);
		modelorg[0] = DotProduct(temp, forward);
		modelorg[1] = -DotProduct(temp, right);
		modelorg[2] = DotProduct(temp, up);
	}

	qglPushMatrix();
	e->angles[0] = -e->angles[0];	// stupid quake bug
	e->angles[2] = -e->angles[2];	// stupid quake bug
	R_RotateForEntity(e);
	e->angles[0] = -e->angles[0];	// stupid quake bug
	e->angles[2] = -e->angles[2];	// stupid quake bug

	// ================================== 
    // detect underwater position 
    // for bmodels caustics by Berserker 
    // modified a bit by reckless. 
    // ================================== 
    currententity->minmax[0] = mins[0]; 
    currententity->minmax[1] = mins[1]; 
    currententity->minmax[2] = mins[2]; 
    currententity->minmax[3] = maxs[0]; 
    currententity->minmax[4] = maxs[1]; 
    currententity->minmax[5] = maxs[2]; 

    VectorSet(org, currententity->minmax[0], currententity->minmax[1], currententity->minmax[5]); 
    cont[0] = CL_PMpointcontents2(org, currentmodel); 
    VectorSet(org, currententity->minmax[3], currententity->minmax[1], currententity->minmax[5]); 
    cont[1] = CL_PMpointcontents2(org, currentmodel); 
    VectorSet(org, currententity->minmax[0], currententity->minmax[4], currententity->minmax[5]); 
    cont[2] = CL_PMpointcontents2(org, currentmodel); 
    VectorSet(org, currententity->minmax[3], currententity->minmax[4], currententity->minmax[5]); 
    cont[3] = CL_PMpointcontents2(org, currentmodel); 
    org[0] = (currententity->minmax[0] + currententity->minmax[3]) * 0.5; 
    org[1] = (currententity->minmax[1] + currententity->minmax[4]) * 0.5; 
    org[2] = (currententity->minmax[2] + currententity->minmax[5]) * 0.5; 
    cont[4] = CL_PMpointcontents2(org, currentmodel); 
    contentsAND = (cont[0] & cont[1] & cont[2] & cont[3] & cont[4]); 
    contentsOR = (cont[0] | cont[1] | cont[2] | cont[3] | cont[4]); 
    viewInWater = (qboolean)(CL_PMpointcontents(r_newrefdef.vieworg) & MASK_WATER);

	//Put camera into model space view angle for bmodels parallax
	if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2])
		{
		VectorSubtract(r_origin, currententity->origin, tmp);
		AnglesToMat3(currententity->angles, entityAxis);
		Mat3_TransposeMultiplyVector(entityAxis, tmp, BmodelViewOrg);
		}
	else
		VectorSubtract(r_origin, currententity->origin, BmodelViewOrg);

	R_DrawInlineBModel2();

	qglEnableVertexAttribArray(ATRB_POSITION);
	qglEnableVertexAttribArray(ATRB_NORMAL);
	qglEnableVertexAttribArray(ATRB_TEX0);
	qglEnableVertexAttribArray(ATRB_TEX1);
	qglEnableVertexAttribArray(ATRB_TANGENT);
	qglEnableVertexAttribArray(ATRB_BINORMAL);
	
	qglVertexAttribPointer(ATRB_POSITION, 3, GL_FLOAT, false, 0, wVertexArray);	
	qglVertexAttribPointer(ATRB_NORMAL, 3, GL_FLOAT, false, 0, nTexArray);
	qglVertexAttribPointer(ATRB_TEX0, 2, GL_FLOAT, false, 0, wTexArray);
	qglVertexAttribPointer(ATRB_TEX1, 2, GL_FLOAT, false, 0, wLMArray);
	qglVertexAttribPointer(ATRB_TANGENT, 3, GL_FLOAT, false, 0, tTexArray);
	qglVertexAttribPointer(ATRB_BINORMAL, 3, GL_FLOAT, false, 0, bTexArray);
	
	r_currTex = -99999;
	num_scene_surfaces = 0;
	R_DrawInlineBModel();
	
	// all types = valid if one or all of these match 
    if ((contentsAND & MASK_WATER) || ((contentsOR & MASK_WATER) && viewInWater)) 
    { 
       // sanity checking since we newer have all the types above. 
       if (contentsOR & CONTENTS_WATER) 
       { 
          GL_BatchLightmappedPoly(true, true); 
       } 
       else 
       { 
          GL_BatchLightmappedPoly(true, false); 
       } 
    } 
    else 
    { 
       GL_BatchLightmappedPoly(true, false); 
    }
	
	qglDisableVertexAttribArray(ATRB_POSITION);
	qglDisableVertexAttribArray(ATRB_NORMAL);
	qglDisableVertexAttribArray(ATRB_TEX0);
	qglDisableVertexAttribArray(ATRB_TEX1);
	qglDisableVertexAttribArray(ATRB_TANGENT);
	qglDisableVertexAttribArray(ATRB_BINORMAL);

	GL_SelectTexture(GL_TEXTURE0_ARB);
	qglPopMatrix();
	qglDisable(GL_BLEND);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

/*
=============================================================

	WORLD MODEL

=============================================================
*/
qboolean SurfInFrustum(msurface_t *s)
{
	if (s->polys)
		return !R_CullBox(s->mins, s->maxs);
	return true;
}



/*
================
R_RecursiveWorldNode
================
*/
static void R_RecursiveWorldNode(mnode_t * node)
{
	int c, side, sidebit;
	cplane_t *plane;
	msurface_t *surf, **mark;
	mleaf_t *pleaf;
	float dot;
	image_t *fx, *image, *nm;
	
	if (node->contents == CONTENTS_SOLID)
		return;					// solid

	if (node->visframe != r_visframecount)
		return;

	if (R_CullBox(node->minmaxs, node->minmaxs + 3))
		return;

	// if a leaf node, draw stuff
	if (node->contents != -1) {
		pleaf = (mleaf_t *) node;

		// check for door connected areas
		if (r_newrefdef.areabits) {
			if (!(r_newrefdef.areabits[pleaf->area >> 3] & (1 << (pleaf->area & 7))))
				return;			// not visible
		}

		mark = pleaf->firstmarksurface;
		c = pleaf->nummarksurfaces;

		if (c) {
			do {
			if (SurfInFrustum(*mark))
					(*mark)->visframe = r_framecount;
				(*mark)->ent = NULL;
				mark++;
			} while (--c);
		}

		return;
	}
	// node is just a decision point, so go down the apropriate sides
	// find which side of the node we are on
	plane = node->plane;

	switch (plane->type) {
	case PLANE_X:
		dot = modelorg[0] - plane->dist;
		break;
	case PLANE_Y:
		dot = modelorg[1] - plane->dist;
		break;
	case PLANE_Z:
		dot = modelorg[2] - plane->dist;
		break;
	default:
		dot = DotProduct(modelorg, plane->normal) - plane->dist;
		break;
	}

	if (dot >= 0) {
		side = 0;
		sidebit = 0;
	} else {
		side = 1;
		sidebit = SURF_PLANEBACK;
	}

	// recurse down the children, front side first
	R_RecursiveWorldNode(node->children[side]);

	// draw stuff
	for (c = node->numsurfaces, surf = r_worldmodel->surfaces + node->firstsurface; c; c--, surf++) {
		
		if (surf->visframe != r_framecount)
			continue;

		if ((surf->flags & SURF_PLANEBACK) != sidebit)
			continue;			// wrong side

		if (surf->texinfo->flags & SURF_SKY) {	// just adds to visible sky bounds
			R_AddSkySurface(surf);
		} else if (surf->texinfo->flags & SURF_NODRAW)
			continue;
		else if (surf->texinfo->flags & (SURF_TRANS33 | SURF_TRANS66)) {	// add to the translucent chain
			surf->texturechain = r_alpha_surfaces;
			r_alpha_surfaces = surf;
		} else {
			if (!(surf->flags & SURF_DRAWTURB)) {
		
				scene_surfaces[num_scene_surfaces++] = surf;

			} else {
				// the polygon is visible, so add it to the texture
				// sorted chain
				// FIXME: this is a hack for animation
				image = R_TextureAnimation(surf->texinfo);
				fx = R_TextureAnimationFx(surf->texinfo); // fix glow hack
				nm = R_TextureAnimationNormal(surf->texinfo);
				surf->texturechain = image->texturechain;
				image->texturechain = surf;

			}
		
		}
	}

	// recurse down the back side
	R_RecursiveWorldNode(node->children[!side]);
}

/*
=============
r_drawWorld
=============
*/
void R_DrawBSP(void)
{
	entity_t ent;

	if (!r_drawWorld->value)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	currentmodel = r_worldmodel;

	VectorCopy(r_newrefdef.vieworg, modelorg);

	// auto cycle the world frame for texture animation
	memset(&ent, 0, sizeof(ent));
	ent.frame = (int) (r_newrefdef.time * 2);
	currententity = &ent;

	gl_state.currenttextures[0] = gl_state.currenttextures[1] = -1;

	qglColor3f(1, 1, 1);
	memset(gl_lms.lightmap_surfaces, 0, sizeof(gl_lms.lightmap_surfaces));
		
	R_ClearSkyBox();

	qglEnableVertexAttribArray(ATRB_POSITION);
	qglEnableVertexAttribArray(ATRB_NORMAL);
	qglEnableVertexAttribArray(ATRB_TEX0);
	qglEnableVertexAttribArray(ATRB_TEX1);
	qglEnableVertexAttribArray(ATRB_TANGENT);
	qglEnableVertexAttribArray(ATRB_BINORMAL);
	
	qglVertexAttribPointer(ATRB_POSITION, 3, GL_FLOAT, false, 0, wVertexArray);	
	qglVertexAttribPointer(ATRB_NORMAL, 3, GL_FLOAT, false, 0, nTexArray);
	qglVertexAttribPointer(ATRB_TEX0, 2, GL_FLOAT, false, 0, wTexArray);
	qglVertexAttribPointer(ATRB_TEX1, 2, GL_FLOAT, false, 0, wLMArray);
	qglVertexAttribPointer(ATRB_TANGENT, 3, GL_FLOAT, false, 0, tTexArray);
	qglVertexAttribPointer(ATRB_BINORMAL, 3, GL_FLOAT, false, 0, bTexArray);

	r_currTex = -99999;
	num_scene_surfaces = 0;
	R_RecursiveWorldNode(r_worldmodel->nodes);
	GL_BatchLightmappedPoly(false, false);
	
	qglDisableVertexAttribArray(ATRB_POSITION);
	qglDisableVertexAttribArray(ATRB_NORMAL);
	qglDisableVertexAttribArray(ATRB_TEX0);
	qglDisableVertexAttribArray(ATRB_TEX1);
	qglDisableVertexAttribArray(ATRB_TANGENT);
	qglDisableVertexAttribArray(ATRB_BINORMAL);

	GL_SelectTexture(GL_TEXTURE0_ARB);
	DrawTextureChains();

	R_DrawSkyBox();

			
}



/*
===============
R_MarkLeaves

Mark the leaves and nodes that are in the PVS for the current
cluster
===============
*/
void R_MarkLeaves(void)
{
	byte *vis;
	byte fatvis[MAX_MAP_LEAFS / 8];
	mnode_t *node;
	int i, c;
	mleaf_t *leaf;
	int cluster;

	if (r_oldviewcluster == r_viewcluster
		&& r_oldviewcluster2 == r_viewcluster2 && !r_noVis->value
		&& r_viewcluster != -1)
		return;

	// development aid to let you run around and see exactly where
	// the pvs ends
	if (r_lockPvs->value)
		return;

	r_visframecount++;
	r_oldviewcluster = r_viewcluster;
	r_oldviewcluster2 = r_viewcluster2;

	if (r_noVis->value || r_viewcluster == -1 || !r_worldmodel->vis) {
		// mark everything
		for (i = 0; i < r_worldmodel->numleafs; i++)
			r_worldmodel->leafs[i].visframe = r_visframecount;
		for (i = 0; i < r_worldmodel->numnodes; i++)
			r_worldmodel->nodes[i].visframe = r_visframecount;
		memset(&viewvis, 0xff, (r_worldmodel->numleafs+7)>>3);
		return;
	}

	vis = Mod_ClusterPVS(r_viewcluster, r_worldmodel);
	// may have to combine two clusters because of solid water boundaries
	if (r_viewcluster2 != r_viewcluster) {
		Q_memcpy(fatvis, vis,
				 (int) ((r_worldmodel->numleafs + 7) * 0.125));
		vis = Mod_ClusterPVS(r_viewcluster2, r_worldmodel);
		c = (r_worldmodel->numleafs + 31) * 0.03125;
		for (i = 0; i < c; i++)
			((int *) fatvis)[i] |= ((int *) vis)[i];
		vis = fatvis;
	}

	memset(&viewvis, 0xff, (r_worldmodel->numleafs+7)>>3);

	for (i = 0, leaf = r_worldmodel->leafs; i < r_worldmodel->numleafs;
		 i++, leaf++) {
		cluster = leaf->cluster;
		if (cluster == -1)
			continue;
		if (vis[cluster >> 3] & (1 << (cluster & 7))) {
			node = (mnode_t *) leaf;
			do {
				if (node->visframe == r_visframecount)
					break;
				node->visframe = r_visframecount;
				node = node->parent;
			} while (node);
		}
	}
}


/*=====================
GLOOM MINI MAP !!!
======================*/

//sul's minimap thing
void R_RecursiveRadarNode(mnode_t * node)
{
	int c, side, sidebit;
	cplane_t *plane;
	msurface_t *surf, **mark;
	mleaf_t *pleaf;
	float dot, distance;
	glpoly_t *p;
	float *v;
	int i;

	if (node->contents == CONTENTS_SOLID)
		return;					// solid

	if (r_radarZoom->value >= 0.1) {
		distance = 1024.0 / r_radarZoom->value;
	} else {
		distance = 1024.0;
	}

	if (r_origin[0] + distance < node->minmaxs[0] ||
		r_origin[0] - distance > node->minmaxs[3] ||
		r_origin[1] + distance < node->minmaxs[1] ||
		r_origin[1] - distance > node->minmaxs[4] ||
		r_origin[2] + 256 < node->minmaxs[2] ||
		r_origin[2] - 256 > node->minmaxs[5])
		return;

	// if a leaf node, draw stuff
	if (node->contents != -1) {
		pleaf = (mleaf_t *) node;
		// check for door connected areas
		if (r_newrefdef.areabits) {
			// not visible
			if (!
				(r_newrefdef.
				 areabits[pleaf->area >> 3] & (1 << (pleaf->area & 7))))
				return;
		}
		mark = pleaf->firstmarksurface;
		c = pleaf->nummarksurfaces;

		if (c) {
			do {
				(*mark)->visframe = r_framecount;
				mark++;
			} while (--c);
		}
		return;
	}
	// node is just a decision point, so go down the apropriate sides
	// find which side of the node we are on
	plane = node->plane;

	switch (plane->type) {
	case PLANE_X:
		dot = modelorg[0] - plane->dist;
		break;
	case PLANE_Y:
		dot = modelorg[1] - plane->dist;
		break;
	case PLANE_Z:
		dot = modelorg[2] - plane->dist;
		break;
	default:
		dot = DotProduct(modelorg, plane->normal) - plane->dist;
		break;
	}

	if (dot >= 0) {
		side = 0;
		sidebit = 0;
	} else {
		side = 1;
		sidebit = SURF_PLANEBACK;
	}

	// recurse down the children, front side first
	R_RecursiveRadarNode(node->children[side]);

	if (plane->normal[2]) {
		// draw stuff
		if (plane->normal[2] > 0)
			for (c = node->numsurfaces, surf =
				 r_worldmodel->surfaces + node->firstsurface; c;
				 c--, surf++) {
				if (surf->texinfo->flags & SURF_SKY) {
					continue;
				}
                
				if (surf->texinfo->flags & (SURF_TRANS33|SURF_TRANS66) && !(surf->texinfo->flags & (SURF_WARP|SURF_FLOWING))) 
				{
					qglColor4f(0,1,0,0.7);
				} else 
				if (surf->texinfo->flags & (SURF_WARP|SURF_FLOWING)) 
				{
				qglColor4f(0,0,1,0.7);
				} 
				else 
				{
				qglColor4f(0.7,0.7,0.7,0.7);
				}

			for ( p = surf->polys; p; p = p->chain ) {
				v = p->verts[0];
				qglBegin (GL_TRIANGLE_FAN);
				for (i=0 ; i< p->numverts; i++, v+= VERTEXSIZE) {
					qglVertex3fv (v);
				}
				qglEnd ();
			}
			}
	}
	// recurse down the back side
	R_RecursiveRadarNode(node->children[!side]);
}



int numRadarEnts = 0;
RadarEnt_t RadarEnts[MAX_RADAR_ENTS];

void GL_DrawRadar(void)
{
	int i;
	float fS[4] = { 0, 0, -1.0 / 512.0, 0 };

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;
	if (!r_radar->value)
		return;

	qglViewport(vid.width - r_radarSize->value, vid.height*0.5 - r_radarSize->value, r_radarSize->value, r_radarSize->value);

	qglMatrixMode(GL_PROJECTION);
	qglPushMatrix();
	qglLoadIdentity();

	qglOrtho(-1024, 1024, -512, 1536, -256, 256);
	

	qglMatrixMode(GL_MODELVIEW);
	qglPushMatrix();
	qglLoadIdentity();

	{
		qglStencilMask(255);
		qglClear(GL_STENCIL_BUFFER_BIT);
		qglEnable(GL_STENCIL_TEST);
		qglStencilFunc(GL_ALWAYS, 4, 4);
		qglStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);


		qglEnable(GL_ALPHA_TEST);
		qglAlphaFunc(GL_LESS, 0.1);
		qglColorMask(0, 0, 0, 0);
                
		
		GL_Bind(r_around->texnum);
		
        qglBegin(GL_TRIANGLE_FAN);
	        
        qglTexCoord2f(0, 1);
		qglVertex3f(1024, -512, 1);
		qglTexCoord2f(1, 1);
		qglVertex3f(-1024, -512, 1);
		qglTexCoord2f(1, 0);
		qglVertex3f(-1024, 1536, 1);
		qglTexCoord2f(0, 0);
		qglVertex3f(1024, 1536, 1);
		
		qglEnd();

		qglColorMask(1, 1, 1, 1);
		qglDisable(GL_ALPHA_TEST);
		qglAlphaFunc(GL_GREATER, 0.5);
		qglStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		qglStencilFunc(GL_NOTEQUAL, 4, 4);

	}

	if (r_radarZoom->value >= 0.1) {
		qglScalef(r_radarZoom->value, r_radarZoom->value,
				  r_radarZoom->value);
	}
	//draw player origin
        qglDisable(GL_TEXTURE_2D);
		qglBegin(GL_TRIANGLES);
		qglColor4f(1, 1, 0, 1);
		qglVertex3f(0, 32, 0);
		qglColor4f(1, 1, 0, 1);
		qglVertex3f(24, -32, 0);
		qglColor4f(1, 1, 0, 1);
        qglVertex3f(-24,-32, 0);
		qglEnd();

	qglRotatef(90 - r_newrefdef.viewangles[1], 0, 0, 1);
	
	qglTranslatef(-r_newrefdef.vieworg[0], -r_newrefdef.vieworg[1],
				  -r_newrefdef.vieworg[2]);

	if (!deathmatch->value) {
		qglBegin(GL_QUADS);
		for (i = 0; i < numRadarEnts; i++) {
			float x = RadarEnts[i].org[0];
			float y = RadarEnts[i].org[1];
			float z = RadarEnts[i].org[2];
			qglColor3fv(RadarEnts[i].color);

			qglVertex3f(x + 9, y + 9, z);
			qglVertex3f(x + 9, y - 9, z);
			qglVertex3f(x - 9, y - 9, z);
			qglVertex3f(x - 9, y + 9, z);
		}
		qglEnd();
		qglColor4f(1, 1, 1, 1);
	}

	qglEnable(GL_TEXTURE_2D);

	GL_Bind(r_radarmap->texnum);
	qglBlendFunc(GL_SRC_ALPHA,GL_ONE);
	qglEnable(GL_BLEND);
	qglColor4f(1, 1, 0, 1);

	fS[3] = 0.5 + r_newrefdef.vieworg[2] / 512.0;
	qglTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

	GLSTATE_ENABLE_TEXGEN;
	qglTexGenfv(GL_S, GL_OBJECT_PLANE, fS);

	// draw the stuff
	R_RecursiveRadarNode(r_worldmodel->nodes);

	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GLSTATE_DISABLE_TEXGEN;
                     
        qglPopMatrix();
        
	qglViewport(0, 0, vid.width, vid.height);

	qglMatrixMode(GL_PROJECTION);
	qglPopMatrix();
	qglMatrixMode(GL_MODELVIEW);
	qglDisable(GL_STENCIL_TEST);
	qglStencilMask(0);
	qglDisable(GL_BLEND);
	qglColor4f(1, 1, 1, 1);
}
