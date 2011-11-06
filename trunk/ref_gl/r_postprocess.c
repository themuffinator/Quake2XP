/*
Copyright (C) 2006-2011 Quake2xp Team and Berserker (sun code base)

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

static vec3_t vert_array[MAX_FLARES_VERTEX];
static vec2_t tex_array[MAX_FLARES_VERTEX];
static vec4_t color_array[MAX_FLARES_VERTEX];


/*
=====================
Draw Occlusion Flares
=====================
*/
void R_BuildFlares(flare_t * light, int Id){
	
	float		dist, dist2, scale;
	vec3_t		j, v, tmp;
	int			sampleCount;
    int			ocCount = 0;
	unsigned	flareIndex[MAX_INDICES];
	int			flareVert=0, index=0;
	
	if (!gl_state.arb_occlusion) 
		return;
	
	if (light->surf->ent) {
		
		if (!VectorCompare(light->surf->ent->angles, vec3_origin))
			return;

		qglPushMatrix();
		R_RotateForEntity(light->surf->ent);
		
	} else { 	
		
		if (R_CullOrigin(light->origin))
			return;

		R_TransformToScreen_Vec3(light->origin, j);

		if( j[0] < r_newrefdef.x || j[0] > r_newrefdef.x + r_newrefdef.width )
			return;
		if( j[1] < r_newrefdef.y || j[1] > r_newrefdef.y + r_newrefdef.height)
			return;
	}
		light->surf->visframe = r_framecount;
	
		// Draw Occlusion Geometry
		qglDisable(GL_TEXTURE_2D);
		qglColorMask(0, 0, 0, 0);
		qglDepthMask(0);
		
		qglEnableClientState(GL_VERTEX_ARRAY);
		qglVertexPointer(3, GL_FLOAT, sizeof(vec3_t), vert);

		qglBeginQueryARB(GL_SAMPLES_PASSED_ARB, ocQueries[Id]);
		
		VA_SetElem3(vert[0],  light->origin[0],  light->origin[1],  light->origin[2]); 
		
			
		qglDrawArrays (GL_POINTS, 0, 1);

		qglEndQueryARB(GL_SAMPLES_PASSED_ARB);
		

		qglDisableClientState(GL_VERTEX_ARRAY);
		qglEnable(GL_TEXTURE_2D);
		qglColorMask(1, 1, 1, 1);
		qglDepthMask(1);
		qglColor4f(1, 1, 1, 1);
	
		qglGetQueryObjectivARB(ocQueries[Id], GL_QUERY_RESULT_ARB, &sampleCount);

		if (!sampleCount) {
			if (light->surf->ent)
				qglPopMatrix();
			return;
		} 
		else
		{

		if(flareVert){
		
		if(gl_state.DrawRangeElements && r_DrawRangeElements->value)
			qglDrawRangeElementsEXT(GL_TRIANGLES, 0, flareVert, index, GL_UNSIGNED_INT, flareIndex);
		else
			qglDrawElements(GL_TRIANGLES, index, GL_UNSIGNED_INT, flareIndex);
		
			flareVert = 0;
			index = 0;
			}
		}

	if(gl_state.nv_conditional_render)
		glBeginConditionalRenderNV(ocQueries[Id], GL_QUERY_WAIT_NV);

	// Draw flares if they occlusion 
	VectorSubtract(light->origin, r_origin, v);
	dist = VectorLength(v) * (light->size * 0.01);

	// Color Fade and cut off by vis distance
	dist2 = VectorLength(v);
	if (dist2 > 2100)
		return;

	scale = ((2048 - dist2) / 2048) * 0.5;
	
	qglDisable(GL_DEPTH_TEST);
	VectorScale(light->color, scale, tmp);

	qglEnableClientState	(GL_TEXTURE_COORD_ARRAY);
	qglTexCoordPointer		(2, GL_FLOAT, 0, tex_array);
	qglEnableClientState	(GL_COLOR_ARRAY);
	qglColorPointer			(4, GL_FLOAT, 0, color_array);
	qglEnableClientState	(GL_VERTEX_ARRAY);
	qglVertexPointer		(3, GL_FLOAT, 0, vert_array);
	
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

	
	qglEnable(GL_DEPTH_TEST);
	qglDisableClientState(GL_VERTEX_ARRAY);
	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
	qglDisableClientState(GL_COLOR_ARRAY);
	qglColor4f(1, 1, 1, 1);
	c_flares++;
	
	if(gl_state.nv_conditional_render)
		glEndConditionalRenderNV();
}


void R_RenderFlares(void)
{
	int i;
	flare_t *fl;

	if (!r_drawFlares->value)
		return;
	
	if ( r_newrefdef.rdflags & RDF_IRGOGGLES)
		return;

	if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
		return;

	qglDepthMask(0);
	qglEnable(GL_BLEND);

	GL_MBind(GL_TEXTURE0_ARB, r_flare->texnum);

	GL_Overbrights(false);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE);

	fl = r_flares;
	for (i = 0; i < r_numflares; i++, fl++) {
		int sidebit;
		float viewplane;
		
		if (fl->surf->visframe != r_framecount)	// pvs culling... haha, nicest optimisation!
			continue;
		
		if(fl->ignore)
			continue;

		viewplane = DotProduct(r_origin, fl->surf->plane->normal) - fl->surf->plane->dist;
		if (viewplane >= 0)
			sidebit = 0;
		else
			sidebit = SURF_PLANEBACK;

		if ((fl->surf->flags & SURF_PLANEBACK) != sidebit)
			continue;			// wrong light poly side!
		
		R_BuildFlares(fl, i);

	}

	qglColor4f(1, 1, 1, 1);
	qglDisable(GL_BLEND);
	qglBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	qglDepthMask(1);
	GL_Overbrights(false);

}

/*
====================
GLSL Full Screen 
Post Process Effects
====================
*/

unsigned int bloomtex = 0;

extern cvar_t	*r_bloomThreshold;
extern cvar_t	*r_bloomIntens;

void R_Bloom (void) {
	
	unsigned	defBits = 0;
	int			id;

	if(!r_bloom->value)
		return;
		
    if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
          return;

	if(r_newrefdef.rdflags & RDF_IRGOGGLES)
		return;

		GL_SelectTexture		(GL_TEXTURE0_ARB);
		qglDisable				(GL_TEXTURE_2D);
		qglEnable				(GL_TEXTURE_RECTANGLE_ARB);
		
		// downsample and cut color
		GL_BindRect				(ScreenMap->texnum);
        qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
		
		// setup program
		GL_BindProgram			(bloomdsProgram, defBits);
		id = bloomdsProgram->id[defBits];

		qglUniform1f			(qglGetUniformLocation(id, "threshold"), r_bloomThreshold->value);
		qglUniform1i			(qglGetUniformLocation(id, "u_map"), 0);
			
		qglBegin(GL_QUADS);
        qglVertex2f(0, vid.height);
        qglVertex2f(vid.width * 0.25 , vid.height);
        qglVertex2f(vid.width * 0.25 , vid.height  * 0.25);
        qglVertex2f(0, vid.height * 0.25);
        qglEnd();

		// create bloom texture (set to zero in default state)
		if (!bloomtex) {
		qglGenTextures			(1, &bloomtex);
		GL_BindRect				(bloomtex);
		qglTexParameteri		(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameteri		(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglCopyTexImage2D		(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, 0, 0, vid.width*0.25, vid.height*0.25, 0);
		}
		
		// blur x
		GL_BindRect				(bloomtex);
		qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);
		
		GL_BindProgram(gaussXProgram, defBits);
		id = gaussXProgram->id[defBits];
		qglUniform1i(qglGetUniformLocation(id, "u_map"), 0);
	
		qglBegin(GL_QUADS);
        qglVertex2f(0, vid.height);
        qglVertex2f(vid.width * 0.25 , vid.height);
        qglVertex2f(vid.width * 0.25 , vid.height  * 0.25);
        qglVertex2f(0, vid.height * 0.25);
        qglEnd();

		// blur y
		GL_BindRect				(bloomtex);
		qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

		GL_BindProgram(gaussYProgram, defBits);
		id = gaussYProgram->id[defBits];
		qglUniform1i(qglGetUniformLocation(id, "u_map"), 0);
		
		qglBegin(GL_QUADS);
        qglVertex2f(0, vid.height);
        qglVertex2f(vid.width * 0.25 , vid.height);
        qglVertex2f(vid.width * 0.25 , vid.height  * 0.25);
        qglVertex2f(0, vid.height * 0.25);
        qglEnd();
		
		// store 2 pass gauss blur 
		qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.25, vid.height*0.25);

		//final pass
		GL_BindProgram(bloomfpProgram, defBits);
		id = bloomfpProgram->id[defBits];
		
		GL_BindRect			(ScreenMap->texnum);
		qglUniform1i		(qglGetUniformLocation(id, "u_map0"), 0);

		GL_SelectTexture	(GL_TEXTURE1_ARB);
		qglDisable			(GL_TEXTURE_2D);
		qglEnable			(GL_TEXTURE_RECTANGLE_ARB);
		GL_BindRect			(bloomtex);
		qglUniform1i		(qglGetUniformLocation(id, "u_map1"), 1);
		
		qglUniform1f		(qglGetUniformLocation(id, "u_bloomAlpha"), r_bloomIntens->value);
		
		qglBegin(GL_QUADS);
        qglVertex2f(0, vid.height);
        qglVertex2f(vid.width, vid.height);
        qglVertex2f(vid.width, 0);
        qglVertex2f(0, 0);
        qglEnd();
						
		GL_BindNullProgram();
		GL_SelectTexture		(GL_TEXTURE1_ARB);
		qglDisable				(GL_TEXTURE_2D);
		qglDisable				(GL_TEXTURE_RECTANGLE_ARB);

		GL_SelectTexture		(GL_TEXTURE0_ARB);
        qglDisable				(GL_TEXTURE_RECTANGLE_ARB);
        qglEnable				(GL_TEXTURE_2D);
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
      qglDisable			(GL_TEXTURE_2D);
      qglEnable				(GL_TEXTURE_RECTANGLE_ARB);
		
    if (!thermaltex) {
      qglGenTextures		(1, &thermaltex);
      GL_BindRect			(thermaltex);
      qglTexParameteri		(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      qglTexParameteri		(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      qglCopyTexImage2D		(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, 0, 0, vid.width, vid.height, 0);
    }
    else {
        GL_BindRect				(thermaltex);
        qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
    }            
		
       // setup program
		GL_BindProgram(thermalProgram, defBits);
		id = thermalProgram->id[defBits];
		qglUniform1i(qglGetUniformLocation(id, "u_screenTex"), 0);

        qglBegin(GL_QUADS);
        qglVertex2f(0, vid.height);
        qglVertex2f(vid.width * 0.5 , vid.height);
        qglVertex2f(vid.width * 0.5 , vid.height  * 0.5);
        qglVertex2f(0, vid.height * 0.5);
        qglEnd();

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

		qglBegin(GL_QUADS);
        qglVertex2f(0, vid.height);
        qglVertex2f(vid.width * 0.5 , vid.height);
        qglVertex2f(vid.width * 0.5 , vid.height  * 0.5);
        qglVertex2f(0, vid.height * 0.5);
        qglEnd();

		// blur y
		GL_BindRect				(thermaltex);
		qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.5, vid.height*0.5);
		
		GL_BindProgram(gaussYProgram, defBits);
		id = gaussYProgram->id[defBits];
		qglUniform1i(qglGetUniformLocation(id, "u_map"), 0);
			
		qglBegin(GL_QUADS);
        qglVertex2f(0, vid.height);
        qglVertex2f(vid.width * 0.5 , vid.height);
        qglVertex2f(vid.width * 0.5 , vid.height  * 0.5);
        qglVertex2f(0, vid.height * 0.5);
        qglEnd();

		// store 2 pass gauss blur 
		qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width*0.5, vid.height*0.5);

		//final pass
		GL_BindProgram(thermalfpProgram, defBits);
		id = thermalfpProgram->id[defBits];
		
		GL_BindRect				(thermaltex);
		qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
		qglUniform1i			(qglGetUniformLocation(id, "u_map"), 0);
			
		qglBegin(GL_QUADS);
        qglVertex2f(0, vid.height);
        qglVertex2f(vid.width, vid.height);
        qglVertex2f(vid.width, 0);
        qglVertex2f(0, 0);
        qglEnd();
			
							
		GL_BindNullProgram();
	
		GL_SelectTexture(GL_TEXTURE0_ARB);
        qglDisable(GL_TEXTURE_RECTANGLE_ARB);
        qglEnable(GL_TEXTURE_2D);
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

	qglDisable				(GL_TEXTURE_2D);
    qglEnable				(GL_TEXTURE_RECTANGLE_ARB);
	GL_BindRect				(ScreenMap->texnum);
    qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);

	// setup program
	GL_BindProgram(radialProgram, defBits);
	id = radialProgram->id[defBits];
	qglUniform1i(qglGetUniformLocation(id, "u_screenMap"), 0);

	if (r_newrefdef.rdflags & RDF_PAIN)
		blur = 0.01;
		else if (r_newrefdef.rdflags & RDF_UNDERWATER)
					blur = 0.0075;
					else
						blur = 0.01;

	// xy = radial center screen space position, z = radius attenuation, w = blur strength
	qglUniform4f(qglGetUniformLocation(id, "u_radialBlurParams"), vid.width*0.5, vid.height*0.5, 1.0/vid.height, blur);
	qglUniform1i(qglGetUniformLocation(id, "u_samples"), r_radialBlurSamples->value);

	qglBegin(GL_QUADS);
    qglVertex2f(0, vid.height);
    qglVertex2f(vid.width, vid.height);
    qglVertex2f(vid.width, 0);
    qglVertex2f(0, 0);
    qglEnd();

	GL_BindNullProgram		();
	qglDisable				(GL_TEXTURE_RECTANGLE_ARB);
    qglEnable				(GL_TEXTURE_2D);
	}
}

extern cvar_t	*r_dof;
extern cvar_t	*r_dofBias;
extern cvar_t	*r_dofFocus;
trace_t CL_PMTraceWorld(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int mask);

void R_DofBlur (void) {
	
	unsigned	defBits = 0;
	int			id;

	if(!r_dof->value)
		return;

    if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
            return;
	if (r_newrefdef.rdflags & RDF_IRGOGGLES)
            return;

	// setup program
	GL_BindProgram(dofProgram, defBits);
	id = dofProgram->id[defBits];
	qglUniform2f(qglGetUniformLocation(id, "u_screenSize"), vid.width, vid.height);
	qglUniform2f(qglGetUniformLocation(id, "u_depthParms"), r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);

	if(r_dof->value >1){
	float		dist;
	trace_t		trace;
	vec3_t		end, tmp, forvard, right, up, _mins = {-32,-32,-32}, _maxs = {32, 32, 32};
	
	AngleVectors(r_newrefdef.viewangles, forvard, right, up);
	VectorMA(r_origin, 9000, forvard, end);     
	trace = CL_PMTraceWorld (r_origin, _mins, _maxs, end, (MASK_SOLID));
	
	if(trace.fraction >0 && trace.fraction <1){
	
	VectorSubtract(r_origin, trace.endpos, tmp);
	dist = VectorLength(tmp);
	qglUniform1f(qglGetUniformLocation(id, "u_focus"), dist);
	}
	else
		qglUniform1f(qglGetUniformLocation(id, "u_focus"), r_dofFocus->value);
	}
	else
		qglUniform1f(qglGetUniformLocation(id, "u_focus"), r_dofFocus->value);
	
	
	qglUniform1f(qglGetUniformLocation(id, "u_bias"), r_dofBias->value);

	GL_SelectTexture		(GL_TEXTURE0_ARB);	
	qglDisable				(GL_TEXTURE_2D);
    qglEnable				(GL_TEXTURE_RECTANGLE_ARB);
	GL_BindRect				(ScreenMap->texnum);
    qglCopyTexSubImage2D	(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vid.width, vid.height);
	qglUniform1i			(qglGetUniformLocation(id, "u_ScreenTex"), 0);

	GL_SelectTexture		(GL_TEXTURE1_ARB);	
	qglDisable				(GL_TEXTURE_2D);
    qglEnable				(GL_TEXTURE_RECTANGLE_ARB);
	GL_BindRect				(depthMap->texnum);
    qglUniform1i			(qglGetUniformLocation(id, "u_DepthTex"), 1);

	qglBegin(GL_QUADS);
    qglVertex2f(0, vid.height);
    qglVertex2f(vid.width, vid.height);
    qglVertex2f(vid.width, 0);
    qglVertex2f(0, 0);
    qglEnd();

	GL_BindNullProgram		();
	GL_SelectTexture		(GL_TEXTURE1_ARB);	
	qglDisable				(GL_TEXTURE_RECTANGLE_ARB);

	GL_SelectTexture		(GL_TEXTURE0_ARB);	
	qglDisable				(GL_TEXTURE_RECTANGLE_ARB);
    qglEnable				(GL_TEXTURE_2D);
}
