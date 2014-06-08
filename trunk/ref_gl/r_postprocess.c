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

void R_BuildFlares(flare_t * light){
	
	float		dist, dist2, scale;
	vec3_t		v, tmp;
	unsigned	flareIndex[MAX_INDICES];
	int			flareVert=0, index=0;
	
	if(r_pplWorld->value > 1 && !light->surf->ent)
		return;

	if (light->surf->ent) {
		
		if (!VectorCompare(light->surf->ent->angles, vec3_origin))
		return;

		qglPushMatrix();
		R_RotateForLightEntity(light->surf->ent);
		
	}
	
	light->surf->visframe = r_framecount;
		
	if(flareVert){
		
	if(gl_state.DrawRangeElements && r_DrawRangeElements->value)
		qglDrawRangeElementsEXT(GL_TRIANGLES, 0, flareVert, index, GL_UNSIGNED_INT, flareIndex);
	else
		qglDrawElements(GL_TRIANGLES, index, GL_UNSIGNED_INT, flareIndex);
		
		flareVert = 0;
		index = 0;
		}
	
	// Color Fade
	VectorSubtract(light->origin, r_origin, v);
	dist = VectorLength(v) * (light->size * 0.01);
	dist2 = VectorLength(v);

	scale = ((1024 - dist2) / 1024) * 0.5;
	if(r_lightScale->value)
		scale /= r_lightScale->value;

	VectorScale(light->color, scale, tmp);

	VectorMA (light->origin, -1-dist, vup, vert_array[flareVert+0]);
	VectorMA (vert_array[flareVert+0], 1+dist, vright, vert_array[flareVert+0]);
	VA_SetElem2(tex_array[flareVert+0], 0, 1);
	VA_SetElem4(color_array[flareVert+0], tmp[0],tmp[1],tmp[2], 1);

	VectorMA (light->origin, -1-dist, vup, vert_array[flareVert+1]);
	VectorMA (vert_array[flareVert+1], -1-dist, vright, vert_array[flareVert+1]);
    VA_SetElem2(tex_array[flareVert+1], 0, 0);
	VA_SetElem4(color_array[flareVert+1], tmp[0],tmp[1],tmp[2], 1);

    VectorMA (light->origin, 1+dist, vup, vert_array[flareVert+2]);
	VectorMA (vert_array[flareVert+2], -1-dist, vright, vert_array[flareVert+2]);
    VA_SetElem2(tex_array[flareVert+2], 1, 0);
	VA_SetElem4(color_array[flareVert+2], tmp[0],tmp[1],tmp[2], 1);

	VectorMA (light->origin, 1+dist, vup, vert_array[flareVert+3]);
	VectorMA (vert_array[flareVert+3], 1+dist, vright, vert_array[flareVert+3]);
    VA_SetElem2(tex_array[flareVert+3], 1, 1);
	VA_SetElem4(color_array[flareVert+3], tmp[0],tmp[1],tmp[2], 1);
	
	flareIndex[index++] = flareVert+0;
	flareIndex[index++] = flareVert+1;
	flareIndex[index++] = flareVert+3;
	flareIndex[index++] = flareVert+3;
	flareIndex[index++] = flareVert+1;
	flareIndex[index++] = flareVert+2;
			
	flareVert+=4;

	
	if(flareVert)
	{
		if(gl_state.DrawRangeElements && r_DrawRangeElements->value)
			qglDrawRangeElementsEXT(GL_TRIANGLES, 0, flareVert, index, GL_UNSIGNED_INT, flareIndex);
		else
			qglDrawElements(GL_TRIANGLES, index, GL_UNSIGNED_INT, flareIndex);

	}

	if (light->surf->ent)
		qglPopMatrix();

	qglColor4f(1, 1, 1, 1);
	c_flares++;
}

qboolean PF_inPVS(vec3_t p1, vec3_t p2);

void R_RenderFlares(void)
{
	int i, id;
	flare_t *fl;
	unsigned defBits = 0;

	if (!r_drawFlares->value)
		return;
	
	if ( r_newrefdef.rdflags & RDF_IRGOGGLES)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	qglEnableVertexAttribArray(ATRB_POSITION);
	qglEnableVertexAttribArray(ATRB_TEX0);
	qglEnableVertexAttribArray(ATRB_COLOR);

	qglVertexAttribPointer(ATRB_POSITION, 3, GL_FLOAT, false, 0, vert_array);
	qglVertexAttribPointer(ATRB_TEX0, 2, GL_FLOAT, false, 0, tex_array);
	qglVertexAttribPointer(ATRB_COLOR, 4, GL_FLOAT, false, 0, color_array);

	qglDepthMask(0);
	qglEnable(GL_BLEND);
	qglBlendFunc(GL_ONE, GL_ONE);

	GL_BindProgram(particlesProgram, defBits);
	id = particlesProgram->id[defBits];

	GL_MBind				(GL_TEXTURE0_ARB, r_flare->texnum);
	qglUniform1i			(qglGetUniformLocation(id, "u_map0"), 0);

	GL_SelectTexture		(GL_TEXTURE1_ARB);	
	GL_BindRect				(depthMap->texnum);
    qglUniform1i			(qglGetUniformLocation(id, "u_depthBufferMap"), 1);
	qglUniform2f			(qglGetUniformLocation(id, "u_depthParms"), r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);
	qglUniform2f			(qglGetUniformLocation(id, "u_mask"), 1.0, 0.0);
	qglUniform1f			(qglGetUniformLocation(id, "u_colorScale"), 1.0);

	fl = r_flares;
	for (i = 0; i < r_numflares; i++, fl++) {
		int sidebit;
		float viewplane;		

		if(fl->ignore)
			continue;

		if(!PF_inPVS(fl->origin, r_origin))
			continue;

		if (fl->surf->visframe != r_framecount)	// pvs culling... haha, nicest optimisation!
			continue;
		
		viewplane = DotProduct(r_origin, fl->surf->plane->normal) - fl->surf->plane->dist;
		if (viewplane >= 0)
			sidebit = 0;
		else
			sidebit = SURF_PLANEBACK;

		if ((fl->surf->flags & SURF_PLANEBACK) != sidebit)
			continue;			// wrong light poly side!
		
		if(r_softParticles->value)
			qglUniform1f(qglGetUniformLocation(id, "u_thickness"), fl->size * 1.5);
		else
			qglUniform1f(qglGetUniformLocation(id, "u_thickness"), 0.0);
		
		R_BuildFlares(fl);

	}
	GL_BindNullProgram();
	qglDisableVertexAttribArray(ATRB_POSITION);
	qglDisableVertexAttribArray(ATRB_TEX0);
    qglDisableVertexAttribArray(ATRB_COLOR);
	qglDisable(GL_BLEND);
	qglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	qglDepthMask(1);
	GL_SelectTexture(GL_TEXTURE0_ARB);
}

/*
====================
GLSL Full Screen 
Post Process Effects
====================
*/

void R_DrawFullScreenQuad()
{
	qglBindBuffer(GL_ARRAY_BUFFER_ARB, gl_state.vbo_fullScreenQuad);
	qglEnableVertexAttribArray(ATRB_POSITION);
	qglVertexAttribPointer(ATRB_POSITION, 2, GL_FLOAT, false, 0, 0);
	
	qglDrawArrays (GL_TRIANGLE_FAN, 0, 4);

	qglDisableVertexAttribArray(ATRB_POSITION);
	qglBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
}

void R_DrawHalfScreenQuad()
{
	qglBindBuffer(GL_ARRAY_BUFFER_ARB, gl_state.vbo_halfScreenQuad);
	qglEnableVertexAttribArray(ATRB_POSITION);
	qglVertexAttribPointer(ATRB_POSITION, 2, GL_FLOAT, false, 0, 0);
	
	qglDrawArrays (GL_TRIANGLE_FAN, 0, 4);

	qglDisableVertexAttribArray(ATRB_POSITION);
	qglBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
}

void R_DrawQuarterScreenQuad()
{
	qglBindBuffer(GL_ARRAY_BUFFER_ARB, gl_state.vbo_quarterScreenQuad);
	qglEnableVertexAttribArray(ATRB_POSITION);
	qglVertexAttribPointer(ATRB_POSITION, 2, GL_FLOAT, false, 0, 0);
	
	qglDrawArrays (GL_TRIANGLE_FAN, 0, 4);

	qglDisableVertexAttribArray(ATRB_POSITION);
	qglBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
}

unsigned int bloomtex = 0;

void R_Bloom (void) {
	
	unsigned	defBits = 0;
	int			id;

	if(!r_bloom->value)
		return;
		
    if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
          return;

	if(r_newrefdef.rdflags & RDF_IRGOGGLES)
		return;

		// downsample and cut color
		GL_SelectTexture		(GL_TEXTURE0_ARB);
		GL_BindRect				(ScreenMap->texnum);
        qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
		
		// setup program
		GL_BindProgram			(bloomdsProgram, defBits);
		id = bloomdsProgram->id[defBits];

		qglUniform1f			(qglGetUniformLocation(id, "u_BloomThreshold"), r_bloomThreshold->value);
		qglUniform1i			(qglGetUniformLocation(id, "u_map"), 0);
			
		R_DrawQuarterScreenQuad();

		// create bloom texture (set to zero in default state)
		if (!bloomtex) {
		qglGenTextures			(1, &bloomtex);
		GL_BindRect				(bloomtex);
		qglTexParameteri		(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameteri		(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglCopyTexImage2D		(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, 0, 0, vid.width*0.25, vid.height*0.25, 0);
		}

		// star blur
		GL_BindRect(bloomtex);
		qglCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

		GL_BindProgram(blurStarProgram, defBits);
		id = blurStarProgram->id[defBits];
		qglUniform1i(qglGetUniformLocation(id, "u_map"), 0);
		qglUniform1f(qglGetUniformLocation(id, "u_starIntens"), r_bloomStarIntens->value);

		R_DrawQuarterScreenQuad();
		qglCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

		// blur x
		GL_BindRect				(bloomtex);
		qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);
		
		GL_BindProgram(gaussXProgram, defBits);
		id = gaussXProgram->id[defBits];
		qglUniform1i(qglGetUniformLocation(id, "u_map"), 0);
	
		R_DrawQuarterScreenQuad();
		
		// blur y
		GL_BindRect				(bloomtex);
		qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

		GL_BindProgram(gaussYProgram, defBits);
		id = gaussYProgram->id[defBits];
		qglUniform1i(qglGetUniformLocation(id, "u_map"), 0);
		
		R_DrawQuarterScreenQuad();
		
		// store 2 pass gauss blur 
		qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

		//final pass
		GL_BindProgram(bloomfpProgram, defBits);
		id = bloomfpProgram->id[defBits];
		
		GL_BindRect			(ScreenMap->texnum);
		qglUniform1i		(qglGetUniformLocation(id, "u_map0"), 0);

		GL_SelectTexture	(GL_TEXTURE1_ARB);
		GL_BindRect			(bloomtex);
		qglUniform1i		(qglGetUniformLocation(id, "u_map1"), 1);
		qglUniform3f		(qglGetUniformLocation(id, "u_bloomParams"), r_bloomIntens->value, r_bloomBright->value, r_bloomExposure->value);
		
		R_DrawFullScreenQuad();
						
		GL_BindNullProgram();
		GL_SelectTexture		(GL_TEXTURE0_ARB);
}


unsigned int thermaltex = 0;

void R_ThermalVision (void) {
	
	unsigned defBits = 0;
	int	id;
		
    if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
            return;

    if (!(r_newrefdef.rdflags & RDF_IRGOGGLES))
            return;
	
	GL_SelectTexture		(GL_TEXTURE0_ARB);
		
    if (!thermaltex) {
      qglGenTextures		(1, &thermaltex);
      GL_BindRect			(thermaltex);
      qglTexParameteri		(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      qglTexParameteri		(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      qglCopyTexImage2D		(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, 0, 0, vid.width, vid.height, 0);
    }
    else 
	{
        GL_BindRect				(thermaltex);
        qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
    }            
		
       // setup program
		GL_BindProgram(thermalProgram, defBits);
		id = thermalProgram->id[defBits];
		qglUniform1i(qglGetUniformLocation(id, "u_screenTex"), 0);

		R_DrawHalfScreenQuad();

		// create thermal texture (set to zero in default state)
		if (!thermaltex) {
		qglGenTextures			(1, &thermaltex);
		GL_BindRect				(thermaltex);
		qglTexParameteri		(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameteri		(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglCopyTexImage2D		(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, 0, 0, vid.width*0.5, vid.height*0.5, 0);
		}

		// blur x
		GL_BindRect				(thermaltex);
		qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.5, vid.height*0.5);
  
		GL_BindProgram(gaussXProgram, defBits);
		id = gaussXProgram->id[defBits];
		qglUniform1i(qglGetUniformLocation(id, "u_map"), 0);

		R_DrawHalfScreenQuad();

		// blur y
		GL_BindRect				(thermaltex);
		qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.5, vid.height*0.5);
		
		GL_BindProgram(gaussYProgram, defBits);
		id = gaussYProgram->id[defBits];
		qglUniform1i(qglGetUniformLocation(id, "u_map"), 0);
			
		R_DrawHalfScreenQuad();

		// store 2 pass gauss blur 
		qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.5, vid.height*0.5);

		//final pass
		GL_BindProgram(thermalfpProgram, defBits);
		id = thermalfpProgram->id[defBits];
		
		GL_BindRect				(thermaltex);
		qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
		qglUniform1i			(qglGetUniformLocation(id, "u_map"), 0);
			
		R_DrawFullScreenQuad();
							
		GL_BindNullProgram();
		GL_SelectTexture(GL_TEXTURE0_ARB);
}



void R_RadialBlur (void) {
	
	unsigned	defBits = 0;
	int			id;
	float		blur;
	
	if(!r_radialBlur->value)
		return;

    if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
            return;
	
	if(r_newrefdef.fov_x <= r_radialBlurFov->value)
		goto hack;

	if (r_newrefdef.rdflags & (RDF_UNDERWATER | RDF_PAIN))
	{

hack:

	GL_SelectTexture		(GL_TEXTURE0_ARB);
	GL_BindRect				(ScreenMap->texnum);
    qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);

	// setup program
	GL_BindProgram(radialProgram, defBits);
	id = radialProgram->id[defBits];
	qglUniform1i(qglGetUniformLocation(id, "u_screenMap"), 0);

	if (r_newrefdef.rdflags & RDF_PAIN)
		blur = 0.01;
		else if (r_newrefdef.rdflags & RDF_UNDERWATER)
					blur = 0.0085;
					else
						blur = 0.01;

	// xy = radial center screen space position, z = radius attenuation, w = blur strength
	qglUniform4f(qglGetUniformLocation(id, "u_radialBlurParams"), vid.width*0.5, vid.height*0.5, 1.0/vid.height, blur);

	R_DrawFullScreenQuad();

	GL_BindNullProgram();
	}
}

float DecodeDepth (float d, vec2_t parms) {
	return parms[0] / (parms[1] - d);
}

void R_DofBlur (void) {
	unsigned		defBits = 0;
	int				id, i, probeSize = 32;
    float			delta, bias, nearestDepth = 1.0, depth[4096];
	static float    dofDepth = 0;
    static int      dofFadeTime = 0;
	vec2_t          dofParams;

	if(!r_dof->value)
		return;

    if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
            return;
	if (r_newrefdef.rdflags & RDF_IRGOGGLES)
            return;

	// berserker's dof autofocus
	if(!r_dofFocus->value){
	qglReadPixels((vid.width - probeSize) / 2, (vid.height - probeSize) / 2, probeSize, probeSize, GL_DEPTH_COMPONENT, GL_FLOAT, &depth[0]);

	for (i = 0; i < probeSize * probeSize; i++)
		if (nearestDepth > depth[i])
			nearestDepth = depth[i];

	nearestDepth = DecodeDepth(nearestDepth, r_newrefdef.depthParms);

	delta = (nearestDepth - dofDepth) * (r_dofAdjust->value * (Sys_Milliseconds() - dofFadeTime) / 1000.0f);
    dofDepth += delta;
    // auto bias
    bias = min(1 / nearestDepth, r_dofBias->value);

	dofParams[0] = max(dofDepth, 0);
	dofParams[1] = bias;
	} 
	else
	{
	dofParams[0] = r_dofFocus->value;
	dofParams[1] = r_dofBias->value;
	}

	// setup program
	GL_BindProgram(dofProgram, defBits);
	id = dofProgram->id[defBits];
	qglUniform2f(qglGetUniformLocation(id, "u_screenSize"), vid.width, vid.height);
	qglUniform4f(qglGetUniformLocation(id, "u_dofParams"), dofParams[0], dofParams[1], r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);


	GL_SelectTexture		(GL_TEXTURE0_ARB);	
	GL_BindRect				(ScreenMap->texnum);
    qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
	qglUniform1i			(qglGetUniformLocation(id, "u_ScreenTex"), 0);

	GL_SelectTexture		(GL_TEXTURE1_ARB);	
	GL_BindRect				(depthMap->texnum);
    qglUniform1i			(qglGetUniformLocation(id, "u_DepthTex"), 1);

	R_DrawFullScreenQuad();

	GL_BindNullProgram		();
	GL_SelectTexture		(GL_TEXTURE0_ARB);	
}

unsigned int fxaatex = 0;

void R_FXAA (void) {
	
	unsigned	defBits = 0;
	int			id;
	
	if(!r_fxaa->value)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
            return;

	// setup program
	GL_BindProgram(fxaaProgram, defBits);
	id = fxaaProgram->id[defBits];
	
	GL_SelectTexture	(GL_TEXTURE0_ARB);	

	if (!fxaatex) {
	qglGenTextures			(1, &fxaatex);
	GL_Bind					(fxaatex);
	qglTexParameteri		(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameteri		(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	qglCopyTexImage2D		(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, vid.width, vid.height, 0);
	}
	GL_Bind					(fxaatex);
    qglCopyTexSubImage2D	(GL_TEXTURE_2D, 0, 0, 0, 0, 0, vid.width, vid.height);
	qglUniform1i			(qglGetUniformLocation(id, "u_ScreenTex"), 0);
	qglUniform2f			(qglGetUniformLocation(id, "u_ScreenSize"), vid.width, vid.height);

	R_DrawFullScreenQuad	();

	GL_BindNullProgram		();

}

void R_FilmGrain (void) {
	
	unsigned	defBits = 0;
	int			id;

	if(!r_filmGrain->value)
		return;

    if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
            return;

	// setup program
	GL_BindProgram(filmGrainProgram, defBits);
	id = filmGrainProgram->id[defBits];

	GL_SelectTexture		(GL_TEXTURE0_ARB);	
	GL_BindRect				(ScreenMap->texnum);
    qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
	qglUniform1i			(qglGetUniformLocation(id, "u_ScreenTex"), 0);
	qglUniform1f			(qglGetUniformLocation(id, "u_scroll"), -3 * (r_newrefdef.time / 40.0));

	R_DrawFullScreenQuad();

	GL_BindNullProgram();
}

void R_GammaRamp (void) {
	
	unsigned	defBits = 0;
	int			id;

	// setup program
	GL_BindProgram(gammaProgram, defBits);
	id = gammaProgram->id[defBits];

	GL_SelectTexture		(GL_TEXTURE0_ARB);	
	GL_BindRect				(ScreenMap->texnum);
    qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
	qglUniform1i			(qglGetUniformLocation(id, "u_ScreenTex"), 0);
	qglUniform4f			(qglGetUniformLocation(id, "u_control"), r_brightness->value, r_contrast->value, r_saturation->value, 1 / r_gamma->value);

	R_DrawFullScreenQuad();
	
	GL_BindNullProgram();
}

unsigned int mbtex = 0;

void R_MotionBlur (void) {
	
	unsigned	defBits = 0;
	int			id;

    if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
            return;
	if (r_newrefdef.rdflags & RDF_IRGOGGLES)
            return;

	// setup program
	GL_BindProgram(motionBlurProgram, defBits);
	id = motionBlurProgram->id[defBits];
	
	qglUniformMatrix4fv	(qglGetUniformLocation(id, "u_PrevModelViewProj"), 1, GL_FALSE, (const GLfloat*)r_newrefdef.oldMvpMatrix);
	qglUniformMatrix4fv	(qglGetUniformLocation(id, "u_InverseModelViewMat"), 1, GL_FALSE, (const GLfloat*)r_newrefdef.inverseMvpMatrix);
	qglUniform2f		(qglGetUniformLocation(id, "u_screenSize"), vid.width, vid.height);
	qglUniform2f		(qglGetUniformLocation(id, "u_depthParms"), r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);

	if (!mbtex) {
		qglGenTextures(1, &mbtex);
		GL_Bind(mbtex);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		qglCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, vid.width, vid.height, 0);
	}
	GL_Bind(mbtex);
	qglCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, vid.width, vid.height);
	qglUniform1i		(qglGetUniformLocation(id, "u_ScreenTex"), 0);

	GL_SelectTexture		(GL_TEXTURE1_ARB);	
	GL_BindRect				(depthMap->texnum);
    qglUniform1i			(qglGetUniformLocation(id, "u_DepthTex"), 1);

	R_DrawFullScreenQuad();

	GL_BindNullProgram();
	GL_SelectTexture(GL_TEXTURE0_ARB);	
}
