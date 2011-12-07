/*
Copyright (C) 2004-2011 Quake2xp Team.

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

vec4_t shadelight_surface;
image_t *r_particletexture[PT_MAX];

typedef vec3_t mat3_t[3];

int CL_PMpointcontents(vec3_t point);

/*
===============
R_DrawParticles
===============
*/
void MakeNormalVectors(vec3_t forward, vec3_t right, vec3_t up);

void ClampVertexColor(vec4_t color)
{
	if (color[0] < 0.15)
		color[0] = 0.15;
	if (color[1] < 0.15)
		color[1] = 0.15;
	if (color[2] < 0.15)
		color[2] = 0.15;
}

#define MAX_PARTICLE_VERT 4096*4

vec4_t ParticleColor[MAX_PARTICLE_VERT];
vec3_t ParticleVert[MAX_PARTICLE_VERT];
vec2_t ParticleTextCoord[MAX_PARTICLE_VERT];

int SortPart(particle_t *a, particle_t *b ){
	return (a->type + a->flags) - (b->type + b->flags);
}

void R_DrawParticles(qboolean WaterCheck)
{
	particle_t *p;
	unsigned	ParticleIndex[MAX_INDICES];
	unsigned	texId, texture = -1, flagId, flags = -1;
	int			i, len, loc, partVert=0, index=0, id, defBits =0;
	vec3_t		point, width;
	vec3_t		move, vec, dir1, dir2, dir3, spdir;
	vec3_t		up, right;
	vec3_t		axis[3];
	vec3_t		oldOrigin;
	vec4_t		outcolor;
	float		scale, r, g, b, a;
	float		c, d, s;

	GL_Overbrights (false);

	// setup program
	GL_BindProgram(particlesProgram, defBits);
	id = particlesProgram->id[defBits];

	GL_SelectTexture		(GL_TEXTURE0_ARB);
	qglEnableClientState	(GL_TEXTURE_COORD_ARRAY);
	qglTexCoordPointer		(2, GL_FLOAT, 0, ParticleTextCoord);
	qglUniform1i			(qglGetUniformLocation(id, "u_map0"), 0);

	GL_SelectTexture		(GL_TEXTURE1_ARB);	
	GL_BindRect				(depthMap->texnum);
    qglUniform1i			(qglGetUniformLocation(id, "u_depthBufferMap"), 1);
	qglUniform2f			(qglGetUniformLocation(id, "u_depthParms"), r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);

	qglEnableClientState	(GL_COLOR_ARRAY);
	qglColorPointer			(4, GL_FLOAT, 0, ParticleColor);
	qglEnableClientState	(GL_VERTEX_ARRAY);
	qglVertexPointer		(3, GL_FLOAT, 0, ParticleVert);

	qglDepthMask(0);		// no z buffering
	qglEnable(GL_BLEND);

	
//	qsort(r_newrefdef.particles, r_newrefdef.num_particles, sizeof(particle_t), (int (*)(const void *, const void *))SortPart);

	for (p = r_newrefdef.particles, i = 0; i < r_newrefdef.num_particles; i++, p++) {
		
		if (WaterCheck) {

			if (!(CL_PMpointcontents(p->origin) & MASK_WATER)) { //in water
				continue;
			}
		}

		if (!WaterCheck) { // on air :-)

			if (CL_PMpointcontents(p->origin) & MASK_WATER) {
				continue;
			}
		}
				
		switch (p->type) {

		case PT_BUBBLE:
			texId = r_particletexture[PT_BUBBLE]->texnum;
			break;

		case PT_FLY:
			texId = fly[((int) (r_newrefdef.time * 10)) & (MAX_FLY - 1)]->texnum;
			break;

		case PT_BLOOD:
			texId = r_particletexture[PT_BLOOD]->texnum;
			break;

		case PT_BLOOD2:
			texId = r_particletexture[PT_BLOOD2]->texnum;
			break;

		case PT_BLASTER:
			texId = r_particletexture[PT_BLASTER]->texnum;
			break;

		case PT_SMOKE:
			texId = r_particletexture[PT_SMOKE]->texnum;
			break;

		case PT_SPLASH:
			texId = r_particletexture[PT_SPLASH]->texnum;
			break;

		case PT_SPARK:
			texId = r_particletexture[PT_SPARK]->texnum;
			break;

		case PT_BEAM:
			texId = r_particletexture[PT_BEAM]->texnum;
			break;

		case PT_SPIRAL:
			texId = r_particletexture[PT_SPIRAL]->texnum;
			break;


		case PT_FLAME:
			texId = flameanim[((int) ((r_newrefdef.time - p->time) * 10)) % MAX_FLAMEANIM]->texnum;
			break;

		case PT_BLOODSPRAY:
			texId = r_blood[((int) ((r_newrefdef.time - p->time) * 15)) % MAX_BLOOD]->texnum;
			break;

		case PT_xBLOODSPRAY:
			texId = r_xblood[((int) ((r_newrefdef.time - p->time) * 15)) % MAX_BLOOD]->texnum;
			break;

		case PT_EXPLODE:
			texId = r_explode[((int) ((r_newrefdef.time - p->time) * 20)) % MAX_EXPLODE]->texnum;
			break;

		case PT_WATERPULME:
			texId = r_particletexture[PT_WATERPULME]->texnum;
			break;

		case PT_WATERCIRCLE:
			texId = r_particletexture[PT_WATERCIRCLE]->texnum;
			break;

		case PT_BLOODDRIP:
			texId = r_particletexture[PT_BLOODDRIP]->texnum;
			break;

		case PT_BLOODMIST:
			texId = r_particletexture[PT_BLOODMIST]->texnum;
			break;

		case PT_BLASTER_BOLT:
			texId =  r_particletexture[PT_BLASTER_BOLT]->texnum;
			break;

		default:
			texId = r_particletexture[PT_DEFAULT]->texnum;

		}


		scale = p->size;
		flagId = p->flags;
		
		if (texture != texId || flags != flagId){

		if (partVert){
		if(gl_state.DrawRangeElements && r_DrawRangeElements->value)
			qglDrawRangeElementsEXT(GL_TRIANGLES, 0, partVert, index, GL_UNSIGNED_INT, ParticleIndex);
		else
			qglDrawElements(GL_TRIANGLES, index, GL_UNSIGNED_INT, ParticleIndex);
		c_part_tris += index/3;
		}
        texture = texId;
		flags = flagId;
        partVert = 0;
        index = 0;

		GL_SelectTexture		(GL_TEXTURE0_ARB);
		GL_Bind					(texId);
		qglBlendFunc			(p->sFactor, p->dFactor);

		if(r_softParticles->value){
		if(p->sFactor == GL_ONE && p->dFactor == GL_ONE)
			qglUniform2f(qglGetUniformLocation(id, "u_mask"), 1.0, 0.0); //color
		else
			qglUniform2f(qglGetUniformLocation(id, "u_mask"), 0.0, 1.0); //alpha
		}

		if(p->flags & PARTICLE_NOFADE || !r_softParticles->value)
			qglUniform1f(qglGetUniformLocation(id, "u_thickness"), 0.0);
		else
			qglUniform1f(qglGetUniformLocation(id, "u_thickness"), scale*0.75); // soft blend scale


		if (p->flags & PARTICLE_OVERBRIGHT)
			qglUniform1f(qglGetUniformLocation(id, "u_colorScale"), 2.0);
		else
			qglUniform1f(qglGetUniformLocation(id, "u_colorScale"), 1.0);
		
		}
		
		r = p->color[0];
		g = p->color[1];
		b = p->color[2];
		a = p->alpha;

		if (p->flags & PARTICLE_VERTEXLIGHT) {
			outcolor[0] = shadelight_surface[0] * r - r;
			outcolor[1] = shadelight_surface[1] * g - g;
			outcolor[2] = shadelight_surface[2] * b - b;
			VectorScale(outcolor, 0.3, outcolor);
			outcolor[3] = a;
			R_LightColor(p->origin, outcolor);
			ClampVertexColor(outcolor);
			r = outcolor[0];
			g = outcolor[1];
			b = outcolor[2];
			a = outcolor[3];
		}

		if (p->flags & PARTICLE_STRETCH) {
			
				VectorSubtract (p->origin, r_newrefdef.vieworg, point);
				CrossProduct (point, p->length, width);
				VectorNormalize (width);
				VectorScale (width, scale, width);

				VA_SetElem2(ParticleTextCoord[partVert+0],1, 1);
				VA_SetElem3(ParticleVert[partVert+0],	p->origin[0] + width[0],
														p->origin[1] + width[1],
														p->origin[2] + width[2]);
				VA_SetElem4(ParticleColor[partVert+0],r, g, b, a);
			

				VA_SetElem2(ParticleTextCoord[partVert+1],0, 0);
				VA_SetElem3(ParticleVert[partVert+1],	p->origin[0] - width[0],
														p->origin[1] - width[1],
														p->origin[2] - width[2]);
				VA_SetElem4(ParticleColor[partVert+1],r, g, b, a);

				VectorAdd (point, p->length, point);
				CrossProduct (point, p->length, width);
				VectorNormalize (width);
				VectorScale (width, scale, width);

				VA_SetElem2(ParticleTextCoord[partVert+2],0, 0);
				VA_SetElem3(ParticleVert[partVert+2],	p->origin[0] + p->length[0] - width[0],
														p->origin[1] + p->length[1] - width[1],
														p->origin[2] + p->length[2] - width[2]);
				VA_SetElem4(ParticleColor[partVert+2],r, g, b, a);

				VA_SetElem2(ParticleTextCoord[partVert+3],1, 1);
				VA_SetElem3(ParticleVert[partVert+3],	p->origin[0] + p->length[0] + width[0],
														p->origin[1] + p->length[1] + width[1],
														p->origin[2] + p->length[2] + width[2]);
				VA_SetElem4(ParticleColor[partVert+3],r, g, b, a);
				
				ParticleIndex[index++] = partVert+0;
				ParticleIndex[index++] = partVert+1;
				ParticleIndex[index++] = partVert+3;
				ParticleIndex[index++] = partVert+3;
				ParticleIndex[index++] = partVert+1;
				ParticleIndex[index++] = partVert+2;

				partVert+=4;
			

		} 
			
			if (p->flags & PARTICLE_SPIRAL) {
			
				VectorCopy(p->origin, move);
				VectorCopy(p->length, vec);
				len = VectorNormalize(vec);
				MakeNormalVectors(vec, right, up);

				for (loc = 0; loc < len; loc++) {
					d = loc * 0.1;
					c = cos(d);
					s = sin(d);

					VectorScale(right, c * 5, dir1);
					VectorMA(dir1, s * 5, up, dir1);

					d = (loc + 1) * 0.1;
					c = cos(d);
					s = sin(d);

					VectorScale(right, c * 5, dir2);
					VectorMA(dir2, s * 5, up, dir2);
					VectorAdd(dir2, vec, dir2);

					d = (loc + 2) * 0.1;
					c = cos(d);
					s = sin(d);

					VectorScale(right, c * 5, dir3);
					VectorMA(dir3, s * 5, up, dir3);
					VectorMA(dir3, 2, vec, dir3);

					VectorAdd(move, dir1, point);
					VectorSubtract(dir2, dir1, spdir);

					VectorSubtract(point, r_origin, point);
					CrossProduct(point, spdir, width);

					if (VectorLength(width))
						VectorNormalize(width);
					else
						VectorCopy(vup, width);


					VA_SetElem2(ParticleTextCoord[partVert+0],0.5, 1);
					VA_SetElem3(ParticleVert[partVert+0],	point[0] + width[0] + r_origin[0],
															point[1] + width[1] + r_origin[1],
															point[2] + width[2] + r_origin[2]);
					VA_SetElem4(ParticleColor[partVert+0],r, g, b, a);

					VA_SetElem2(ParticleTextCoord[partVert+1],0.5, 0);
					VA_SetElem3(ParticleVert[partVert+1],	point[0] - width[0] + r_origin[0],
															point[1] - width[1] + r_origin[1],
															point[2] - width[2] + r_origin[2]);
					VA_SetElem4(ParticleColor[partVert+1],r, g, b, a);


					VectorAdd(move, dir2, point);
					VectorSubtract(dir3, dir2, spdir);


					VectorSubtract(point, r_origin, point);
					CrossProduct(point, spdir, width);

					if (VectorLength(width))
						VectorNormalize(width);
					else
						VectorCopy(vup, width);


					VA_SetElem2(ParticleTextCoord[partVert+2],0.5, 0);
					VA_SetElem3(ParticleVert[partVert+2],	point[0] - width[0] + r_origin[0],
															point[1] - width[1] + r_origin[1],
															point[2] - width[2] + r_origin[2]);
					VA_SetElem4(ParticleColor[partVert+2],r, g, b, a);

					VA_SetElem2(ParticleTextCoord[partVert+3],0.5, 1);
					VA_SetElem3(ParticleVert[partVert+3],	point[0] + width[0] + r_origin[0],
															point[1] + width[1] + r_origin[1],
															point[2] + width[2] + r_origin[2]);
					VA_SetElem4(ParticleColor[partVert+3],r, g, b, a);
					
					ParticleIndex[index++] = partVert+0;
					ParticleIndex[index++] = partVert+1;
					ParticleIndex[index++] = partVert+3;
					ParticleIndex[index++] = partVert+3;
					ParticleIndex[index++] = partVert+1;
					ParticleIndex[index++] = partVert+2;

					partVert+=4;
				
					VectorAdd(move, vec, move);
				
				}
				

		} 
		
		if (p->flags & PARTICLE_DIRECTIONAL) {
			// find orientation vectors
			VectorSubtract(r_newrefdef.vieworg, p->origin, axis[0]);
			VectorSubtract(p->oldOrg, p->origin, axis[1]);
			CrossProduct(axis[0], axis[1], axis[2]);
			VectorNormalize(axis[1]);
			VectorNormalize(axis[2]);

			// find normal
			CrossProduct(axis[1], axis[2], axis[0]);
			VectorNormalize(axis[0]);

			VectorMA(p->origin, -p->len, axis[1], oldOrigin);
			VectorScale(axis[2], p->size, axis[2]);

						
				VA_SetElem2(ParticleTextCoord[partVert+0],1, 1);
				VA_SetElem3(ParticleVert[partVert+0],	oldOrigin[0] + axis[2][0],
														oldOrigin[1] + axis[2][1],
														oldOrigin[2] + axis[2][2]);
				VA_SetElem4(ParticleColor[partVert+0],r, g, b, a);

				VA_SetElem2(ParticleTextCoord[partVert+1],0, 1);
				VA_SetElem3(ParticleVert[partVert+1],	p->origin[0] + axis[2][0],
														p->origin[1] + axis[2][1],
														p->origin[2] + axis[2][2]);
				VA_SetElem4(ParticleColor[partVert+1],r, g, b, a);

				VA_SetElem2(ParticleTextCoord[partVert+2],0, 0);
				VA_SetElem3(ParticleVert[partVert+2],	p->origin[0] - axis[2][0],
														p->origin[1] - axis[2][1],
														p->origin[2] - axis[2][2]);
				VA_SetElem4(ParticleColor[partVert+2],r, g, b, a);

				VA_SetElem2(ParticleTextCoord[partVert+3],1, 0);
				VA_SetElem3(ParticleVert[partVert+3],	oldOrigin[0] - axis[2][0],
														oldOrigin[1] - axis[2][1],
														oldOrigin[2] - axis[2][2]);
				VA_SetElem4(ParticleColor[partVert+3],r, g, b, a);
				
				ParticleIndex[index++] = partVert+0;
				ParticleIndex[index++] = partVert+1;
				ParticleIndex[index++] = partVert+3;
				ParticleIndex[index++] = partVert+3;
				ParticleIndex[index++] = partVert+1;
				ParticleIndex[index++] = partVert+2;
				
				partVert+=4;
											
		
		} 
		
		if (p->flags & PARTICLE_ALIGNED) {
			// Find axes
			VectorCopy(p->dir, axis[0]);
			MakeNormalVectors(axis[0], axis[1], axis[2]);

			// Scale the axes by radius
			VectorScale(axis[1], p->size, axis[1]);
			VectorScale(axis[2], p->size, axis[2]);


			VA_SetElem2(ParticleTextCoord[partVert+0],0, 1);
			VA_SetElem3(ParticleVert[partVert+0],	p->origin[0] + axis[1][0] + axis[2][0],
													p->origin[1] + axis[1][1] + axis[2][1],
													p->origin[2] + axis[1][2] + axis[2][2]);
			VA_SetElem4(ParticleColor[partVert+0],r, g, b, a);

			VA_SetElem2(ParticleTextCoord[partVert+1],0, 0);
			VA_SetElem3(ParticleVert[partVert+1],	p->origin[0] - axis[1][0] + axis[2][0],
													p->origin[1] - axis[1][1] + axis[2][1],
													p->origin[2] - axis[1][2] + axis[2][2]);
			VA_SetElem4(ParticleColor[partVert+1],r, g, b, a);

			VA_SetElem2(ParticleTextCoord[partVert+2],1, 0);
			VA_SetElem3(ParticleVert[partVert+2],	p->origin[0] - axis[1][0] - axis[2][0],
													p->origin[1] - axis[1][1] - axis[2][1],
													p->origin[2] - axis[1][2] - axis[2][2]);
			VA_SetElem4(ParticleColor[partVert+2],r, g, b, a);

			VA_SetElem2(ParticleTextCoord[partVert+3],1, 1);
			VA_SetElem3(ParticleVert[partVert+3],	p->origin[0] + axis[1][0] - axis[2][0],
													p->origin[1] + axis[1][1] - axis[2][1],
													p->origin[2] + axis[1][2] - axis[2][2]);
			VA_SetElem4(ParticleColor[partVert+3],r, g, b, a);
			
			ParticleIndex[index++] = partVert+0;
			ParticleIndex[index++] = partVert+1;
			ParticleIndex[index++] = partVert+3;
			ParticleIndex[index++] = partVert+3;
			ParticleIndex[index++] = partVert+1;
			ParticleIndex[index++] = partVert+2;

			partVert+=4;


		}
		if(	!(p->flags & PARTICLE_ALIGNED)	&& !(p->flags & PARTICLE_DIRECTIONAL) && 
			!(p->flags & PARTICLE_SPIRAL)	&& !(p->flags & PARTICLE_STRETCH)){
			//  standart particles 
			VectorScale(vup, scale, up);
			VectorScale(vright, scale, right);

			// hack a scale up to keep particles from disapearing
			scale = (p->origin[0] - r_origin[0]) * vpn[0] +
					(p->origin[1] - r_origin[1]) * vpn[1] +
					(p->origin[2] - r_origin[2]) * vpn[2];

			scale = (scale < 20) ? 1 : 1 + scale * 0.0004;


			if (p->orient) {
			
				float c = (float)cos (DEG2RAD (p->orient)) * scale;
				float s = (float)sin (DEG2RAD (p->orient)) * scale;
				
					VA_SetElem2(ParticleTextCoord[partVert+0], 0, 1);
					VA_SetElem3(ParticleVert[partVert+0],	p->origin[0] - right[0] * c - up[0] * s, 
															p->origin[1] - right[1] * c - up[1] * s,
															p->origin[2] - right[2] * c - up[2] * s);
					VA_SetElem4(ParticleColor[partVert+0],r, g, b, a);

					VA_SetElem2(ParticleTextCoord[partVert+1],0, 0);
					VA_SetElem3(ParticleVert[partVert+1],	p->origin[0] - right[0] * s + up[0] * c,
															p->origin[1] - right[1] * s + up[1] * c,
															p->origin[2] - right[2] * s + up[2] * c);
					VA_SetElem4(ParticleColor[partVert+1],r, g, b, a);

					VA_SetElem2(ParticleTextCoord[partVert+2],1, 0);
					VA_SetElem3(ParticleVert[partVert+2],	p->origin[0] + right[0] * c + up[0] * s,
															p->origin[1] + right[1] * c + up[1] * s,
															p->origin[2] + right[2] * c + up[2] * s);
					VA_SetElem4(ParticleColor[partVert+2],r, g, b, a);

					VA_SetElem2(ParticleTextCoord[partVert+3],1, 1);
					VA_SetElem3(ParticleVert[partVert+3],	p->origin[0] + right[0] * s - up[0] * c,
															p->origin[1] + right[1] * s - up[1] * c,
															p->origin[2] + right[2] * s - up[2] * c);
					VA_SetElem4(ParticleColor[partVert+3],r, g, b, a);
					
					ParticleIndex[index++] = partVert+0;
					ParticleIndex[index++] = partVert+1;
					ParticleIndex[index++] = partVert+3;
					ParticleIndex[index++] = partVert+3;
					ParticleIndex[index++] = partVert+1;
					ParticleIndex[index++] = partVert+2;

					partVert+=4;

			} else	{
				
					VA_SetElem2(ParticleTextCoord[partVert+0], 0, 1);
					VA_SetElem3(ParticleVert[partVert+0],	p->origin[0] - right[0] * scale - up[0] * scale,
															p->origin[1] - right[1] * scale - up[1] * scale,
															p->origin[2] - right[2] * scale - up[2] * scale);
					VA_SetElem4(ParticleColor[partVert+0],r, g, b, a);

					VA_SetElem2(ParticleTextCoord[partVert+1],0, 0);
					VA_SetElem3(ParticleVert[partVert+1],	p->origin[0] - right[0] * scale + up[0] * scale,
															p->origin[1] - right[1] * scale + up[1] * scale,
															p->origin[2] - right[2] * scale + up[2] * scale);
					VA_SetElem4(ParticleColor[partVert+1],r, g, b, a);

					VA_SetElem2(ParticleTextCoord[partVert+2],1, 0);
					VA_SetElem3(ParticleVert[partVert+2],	p->origin[0] + right[0] * scale + up[0] * scale,
															p->origin[1] + right[1] * scale + up[1] * scale,
															p->origin[2] + right[2] * scale + up[2] * scale);
					VA_SetElem4(ParticleColor[partVert+2],r, g, b, a);

					VA_SetElem2(ParticleTextCoord[partVert+3],1, 1);
					VA_SetElem3(ParticleVert[partVert+3],	p->origin[0] + right[0] * scale - up[0] * scale,
															p->origin[1] + right[1] * scale - up[1] * scale,
															p->origin[2] + right[2] * scale - up[2] * scale);
					VA_SetElem4(ParticleColor[partVert+3],r, g, b, a);
					
					ParticleIndex[index++] = partVert+0;
					ParticleIndex[index++] = partVert+1;
					ParticleIndex[index++] = partVert+3;
					ParticleIndex[index++] = partVert+3;
					ParticleIndex[index++] = partVert+1;
					ParticleIndex[index++] = partVert+2;
			
					partVert+=4;
					
			}
			
		}
	}

	if(partVert)
	{
		if(gl_state.DrawRangeElements && r_DrawRangeElements->value)
			qglDrawRangeElementsEXT(GL_TRIANGLES, 0, partVert, index, GL_UNSIGNED_INT, ParticleIndex);
		else
			qglDrawElements(GL_TRIANGLES, index, GL_UNSIGNED_INT, ParticleIndex);
	
		c_part_tris += index/3;
	}

	qglDisable(GL_BLEND);
	qglColor4f(1, 1, 1, 1);
	qglDepthRange(0, 1);
	qglDepthMask(1);			// back to normal Z buffering

	GL_BindNullProgram();
	GL_SelectTexture(GL_TEXTURE0_ARB);	
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	qglDisableClientState(GL_VERTEX_ARRAY);
	qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
	qglDisableClientState(GL_COLOR_ARRAY);
	
}
