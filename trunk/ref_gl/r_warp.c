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
// gl_warp.c -- sky and water polygons

#include "r_local.h"


extern model_t *loadmodel;
vec4_t shadelight_surface;
char skyname[MAX_QPATH];
float skyrotate;
vec3_t skyaxis;
image_t *sky_images[6];
static float shadelight[3];

/*
=============
R_DrawWaterPolygons

Does a water warp on the pre-fragmented glpoly_t chain
=============
*/
float WarpColorArray[MAX_BATCH_SURFS][4];

void RenderLavaSurfaces(msurface_t * fa)
{
	int			i, nv = fa->polys->numverts;
	float		*v;
	glpoly_t	*p, *bp;
	float		scale[2];
	int			id;
	unsigned	defBits = 0;

	defBits = worldDefs.VertexLightBits;
	
	if (fa->texinfo->image->has_alpha && r_parallax->value)
		defBits |= worldDefs.ParallaxBit;

	// setup program
	GL_BindProgram(ambientWorldProgram, defBits);
	id = ambientWorldProgram->id[defBits];


	if(!fa->texinfo->image->specularScale)
			qglUniform1f(qglGetUniformLocation(id, "u_specularScale"), 1.0);
		else
			qglUniform1f(qglGetUniformLocation(id, "u_specularScale"), fa->texinfo->image->specularScale);

		if(!fa->texinfo->image->SpecularExp)
			qglUniform1f(qglGetUniformLocation(id, "u_specularExp"), 16.0);
		else
			qglUniform1f(qglGetUniformLocation(id, "u_specularExp"), fa->texinfo->image->SpecularExp);

		if(r_parallax->value){
			
			if(!fa->texinfo->image->parallaxScale){

			scale[0] = 7.0 / fa->texinfo->image->width;
			scale[1] = 7.0 / fa->texinfo->image->height;
			}
			else
			{
			scale[0] = fa->texinfo->image->parallaxScale / fa->texinfo->image->width;
			scale[1] = fa->texinfo->image->parallaxScale / fa->texinfo->image->height;
			}
			qglUniform2f(qglGetUniformLocation(id, "u_parallaxScale"), scale[0], scale[1]);
			qglUniform2f(qglGetUniformLocation(id, "u_texSize"), fa->texinfo->image->upload_width, fa->texinfo->image->upload_height);
		}

	qglUniform1f	(qglGetUniformLocation(id, "u_ColorModulate"),	1.0);
	qglUniform3fv	(qglGetUniformLocation(id, "u_viewOriginES"),	1 , r_origin);
	qglUniform1i	(qglGetUniformLocation(id, "u_parallaxType"),	(int)r_parallax->value);
	qglUniform1f	(qglGetUniformLocation(id, "u_ambientScale"),	0.0);


	GL_MBind		(GL_TEXTURE0_ARB, fa->texinfo->image->texnum);
	qglUniform1i	(qglGetUniformLocation(id, "u_Diffuse"), 0);
	GL_MBind		(GL_TEXTURE1_ARB, fa->texinfo->normalmap->texnum);
	qglUniform1i	(qglGetUniformLocation(id, "u_NormalMap"), 1);

	qglEnableVertexAttribArray(ATRB_POSITION);
	qglEnableVertexAttribArray(ATRB_NORMAL);
	qglEnableVertexAttribArray(ATRB_TEX0);
	qglEnableVertexAttribArray(ATRB_TANGENT);
	qglEnableVertexAttribArray(ATRB_BINORMAL);
	qglEnableVertexAttribArray(ATRB_COLOR);

	qglVertexAttribPointer(ATRB_POSITION, 3, GL_FLOAT, false,	0, wVertexArray);	
	qglVertexAttribPointer(ATRB_NORMAL, 3, GL_FLOAT, false,		0, nTexArray);
	qglVertexAttribPointer(ATRB_TEX0, 2, GL_FLOAT, false,		0, wTexArray);
	qglVertexAttribPointer(ATRB_TANGENT, 3, GL_FLOAT, false,	0, tTexArray);
	qglVertexAttribPointer(ATRB_BINORMAL, 3, GL_FLOAT, false,	0, bTexArray);
	qglVertexAttribPointer(ATRB_COLOR, 4, GL_FLOAT, false,		0, WarpColorArray);
	
	for (bp = fa->polys; bp; bp = bp->next) {
		p = bp;

	c_brush_polys += (nv-2);

	for (i = 0, v = p->verts[0]; i < p->numverts; i++, v += VERTEXSIZE) {
			
			VectorCopy(v, wVertexArray[i]);
			
			// diffuse
			wTexArray[i][0] = v[3];
			wTexArray[i][1] = v[4];
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
		R_LightColor	(v, shadelight_surface);
		VA_SetElem4		(WarpColorArray[i],	shadelight_surface[0], shadelight_surface[1], shadelight_surface[2], 1.0);		
		}
		
		qglDrawElements(GL_TRIANGLES, fa->numIndices, GL_UNSIGNED_SHORT, fa->indices);
	}
	
	GL_SelectTexture(GL_TEXTURE0_ARB);
	qglDisableVertexAttribArray(ATRB_POSITION);
	qglDisableVertexAttribArray(ATRB_NORMAL);
	qglDisableVertexAttribArray(ATRB_TEX0);
	qglDisableVertexAttribArray(ATRB_TANGENT);
	qglDisableVertexAttribArray(ATRB_BINORMAL);
	qglDisableVertexAttribArray(ATRB_COLOR);
	GL_BindNullProgram();

}


#define DST_SIZE 16
unsigned int dst_texture = 0;

/*
===============
CreateDSTTex

Create the texture which warps texture shaders
===============
*/
void CreateDSTTex()
{
	signed char data[DST_SIZE][DST_SIZE][2];
	int x, y;



	for (x = 0; x < DST_SIZE; x++)
		for (y = 0; y < DST_SIZE; y++) {
			data[x][y][0] = rand() % 255 - 128;
			data[x][y][1] = rand() % 255 - 128;
		}

	qglGenTextures(1, &dst_texture);
	qglBindTexture(GL_TEXTURE_2D, dst_texture);
	qglTexImage2D(GL_TEXTURE_2D, 0, 4, DST_SIZE, DST_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	qglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

}

void R_DrawWaterPolygons(msurface_t * fa)
{
	glpoly_t	*p, *bp;
	float		*v, dstscroll;
	int			id, i, nv = fa->polys->numverts;
	unsigned	defBits = 0, texture = -1;
	
		
	qglDisable(GL_BLEND);

	if (fa->texinfo->flags & (SURF_TRANS33 | SURF_TRANS66)){
		defBits = worldDefs.WaterTransBits;
	
	}
	else
		defBits = 0;

	// setup program
	GL_BindProgram(waterProgram, defBits);
	id = waterProgram->id[defBits];

	GL_SelectTexture			(GL_TEXTURE0_ARB);
	GL_Bind						(fa->texinfo->image->texnum);
	qglUniform1i				(qglGetUniformLocation(id, "u_colorMap"), 0);

	GL_SelectTexture			(GL_TEXTURE1_ARB);
	GL_Bind						(r_DSTTex->texnum);
	qglUniform1i				(qglGetUniformLocation(id, "u_dstMap"), 1);
	
	if(defBits >0){
	GL_SelectTexture			(GL_TEXTURE2_ARB);
	GL_BindRect					(ScreenMap->texnum);
	qglUniform1i				(qglGetUniformLocation(id, "g_colorBufferMap"), 2);
	
	GL_SelectTexture			(GL_TEXTURE3_ARB);
	GL_BindRect					(depthMap->texnum);
	qglUniform1i				(qglGetUniformLocation(id, "g_depthBufferMap"), 3);
	}
	
	qglUniform1f				(qglGetUniformLocation(id, "u_deformMul"),	1.0);
	qglUniform1f				(qglGetUniformLocation(id, "u_alpha"),	0.499);
	qglUniform1f				(qglGetUniformLocation(id, "u_thickness"),	150.0);
	qglUniform2f				(qglGetUniformLocation(id, "u_viewport"),	vid.width, vid.height);
	qglUniform2f				(qglGetUniformLocation(id, "u_depthParms"), r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);
	qglUniform1f				(qglGetUniformLocation(id, "u_ColorModulate"), r_worldColorScale->value);
			
	dstscroll = ((r_newrefdef.time * 0.15f) - (int) (r_newrefdef.time * 0.15f));

	qglEnableVertexAttribArray(ATRB_POSITION);
	qglEnableVertexAttribArray(ATRB_TEX0);
	qglEnableVertexAttribArray(ATRB_TEX2);
	qglEnableVertexAttribArray(ATRB_COLOR);

	qglVertexAttribPointer(ATRB_POSITION, 3, GL_FLOAT, false,	0, wVertexArray);	
	qglVertexAttribPointer(ATRB_TEX0, 2, GL_FLOAT, false,		0, wTexArray);
	qglVertexAttribPointer(ATRB_TEX2, 2, GL_FLOAT, false,		0, wTmu2Array);
	qglVertexAttribPointer(ATRB_COLOR, 4, GL_FLOAT, false,		0, WarpColorArray);
	
	for (bp = fa->polys; bp; bp = bp->next) {
		p = bp;
		c_brush_polys += (nv-2);

	for (i = 0, v = p->verts[0]; i < p->numverts; i++, v += VERTEXSIZE) {
		
		VectorCopy(v, wVertexArray[i]);
			
		wTexArray[i][0] = v[3];
		wTexArray[i][1] = v[4];

		wTmu2Array[i][0] = (v[3] + dstscroll);
		wTmu2Array[i][1] = (v[4] + dstscroll);

		R_LightColor	(v, shadelight_surface);
		VA_SetElem4		(WarpColorArray[i],	shadelight_surface[0], shadelight_surface[1], shadelight_surface[2], 1.0);	
		}

		qglDrawElements(GL_TRIANGLES, fa->numIndices, GL_UNSIGNED_SHORT, fa->indices);
	}
		
	GL_SelectTexture(GL_TEXTURE0_ARB);
	qglDisableVertexAttribArray(ATRB_POSITION);
	qglDisableVertexAttribArray(ATRB_TEX0);
	qglDisableVertexAttribArray(ATRB_TEX2);
	qglDisableVertexAttribArray(ATRB_COLOR);
	GL_BindNullProgram();
}


//===================================================================


vec3_t skyclip[6] = {
	{1, 1, 0}
	,
	{1, -1, 0}
	,
	{0, -1, 1}
	,
	{0, 1, 1}
	,
	{1, 0, 1}
	,
	{-1, 0, 1}
};
int c_sky;

// 1 = s, 2 = t, 3 = 2048
int st_to_vec[6][3] = {
	{3, -1, 2},
	{-3, 1, 2},

	{1, 3, 2},
	{-1, -3, 2},

	{-2, -1, 3},				// 0 degrees yaw, look straight up
	{2, -1, -3}					// look straight down

//  {-1,2,3},
//  {1,2,-3}
};

// s = [0]/[2], t = [1]/[2]
int vec_to_st[6][3] = {
	{-2, 3, 1},
	{2, 3, -1},

	{1, 3, 2},
	{-1, 3, -2},

	{-2, -1, 3},
	{-2, 1, -3}

//  {-1,2,3},
//  {1,2,-3}
};

float skymins[2][6], skymaxs[2][6];
float sky_min, sky_max;

void DrawSkyPolygon(int nump, vec3_t vecs)
{
	int i, j;
	vec3_t v, av;
	float s, t, dv;
	int axis;
	float *vp;

	c_sky++;

	// decide which face it maps to
	VectorCopy(vec3_origin, v);
	for (i = 0, vp = vecs; i < nump; i++, vp += 3) {
		VectorAdd(vp, v, v);
	}
	av[0] = fabs(v[0]);
	av[1] = fabs(v[1]);
	av[2] = fabs(v[2]);
	if (av[0] > av[1] && av[0] > av[2]) {
		if (v[0] < 0)
			axis = 1;
		else
			axis = 0;
	} else if (av[1] > av[2] && av[1] > av[0]) {
		if (v[1] < 0)
			axis = 3;
		else
			axis = 2;
	} else {
		if (v[2] < 0)
			axis = 5;
		else
			axis = 4;
	}

	// project new texture coords
	for (i = 0; i < nump; i++, vecs += 3) {
		j = vec_to_st[axis][2];
		if (j > 0)
			dv = vecs[j - 1];
		else
			dv = -vecs[-j - 1];
		if (dv < 0.001)
			continue;			// don't divide by zero
		j = vec_to_st[axis][0];
		if (j < 0)
			s = -vecs[-j - 1] / dv;
		else
			s = vecs[j - 1] / dv;
		j = vec_to_st[axis][1];
		if (j < 0)
			t = -vecs[-j - 1] / dv;
		else
			t = vecs[j - 1] / dv;

		if (s < skymins[0][axis])
			skymins[0][axis] = s;
		if (t < skymins[1][axis])
			skymins[1][axis] = t;
		if (s > skymaxs[0][axis])
			skymaxs[0][axis] = s;
		if (t > skymaxs[1][axis])
			skymaxs[1][axis] = t;
	}
}

#define	ON_EPSILON		0.1		// point on plane side epsilon
#define	MAX_CLIP_VERTS	64
void ClipSkyPolygon(int nump, vec3_t vecs, int stage)
{
	float *norm;
	float *v;
	qboolean front, back;
	float d, e;
	float dists[MAX_CLIP_VERTS];
	int sides[MAX_CLIP_VERTS];
	vec3_t newv[2][MAX_CLIP_VERTS];
	int newc[2];
	int i, j;

	if (nump > MAX_CLIP_VERTS - 2)
		VID_Error(ERR_DROP, "ClipSkyPolygon: MAX_CLIP_VERTS");
	if (stage == 6) {			// fully clipped, so draw it
		DrawSkyPolygon(nump, vecs);
		return;
	}

	front = back = false;
	norm = skyclip[stage];
	for (i = 0, v = vecs; i < nump; i++, v += 3) {
		d = DotProduct(v, norm);
		if (d > ON_EPSILON) {
			front = true;
			sides[i] = SIDE_FRONT;
		} else if (d < -ON_EPSILON) {
			back = true;
			sides[i] = SIDE_BACK;
		} else
			sides[i] = SIDE_ON;
		dists[i] = d;
	}

	if (!front || !back) {		// not clipped
		ClipSkyPolygon(nump, vecs, stage + 1);
		return;
	}
	// clip it
	sides[i] = sides[0];
	dists[i] = dists[0];
	VectorCopy(vecs, (vecs + (i * 3)));
	newc[0] = newc[1] = 0;

	for (i = 0, v = vecs; i < nump; i++, v += 3) {
		switch (sides[i]) {
		case SIDE_FRONT:
			VectorCopy(v, newv[0][newc[0]]);
			newc[0]++;
			break;
		case SIDE_BACK:
			VectorCopy(v, newv[1][newc[1]]);
			newc[1]++;
			break;
		case SIDE_ON:
			VectorCopy(v, newv[0][newc[0]]);
			newc[0]++;
			VectorCopy(v, newv[1][newc[1]]);
			newc[1]++;
			break;
		}

		if (sides[i] == SIDE_ON || sides[i + 1] == SIDE_ON
			|| sides[i + 1] == sides[i])
			continue;

		d = dists[i] / (dists[i] - dists[i + 1]);
		for (j = 0; j < 3; j++) {
			e = v[j] + d * (v[j + 3] - v[j]);
			newv[0][newc[0]][j] = e;
			newv[1][newc[1]][j] = e;
		}
		newc[0]++;
		newc[1]++;
	}

	// continue
	ClipSkyPolygon(newc[0], newv[0][0], stage + 1);
	ClipSkyPolygon(newc[1], newv[1][0], stage + 1);
}

/*
=================
R_AddSkySurface
=================
*/
void R_AddSkySurface(msurface_t * fa)
{
	int i;
	vec3_t verts[MAX_CLIP_VERTS];
	glpoly_t *p;

	// calculate vertex values for sky box
	for (p = fa->polys; p; p = p->next) {
		for (i = 0; i < p->numverts; i++) {
			VectorSubtract(p->verts[i], r_origin, verts[i]);
		}
		ClipSkyPolygon(p->numverts, verts[0], 0);
	}
}


/*
==============
R_ClearSkyBox
==============
*/
void R_ClearSkyBox(void)
{
	int i;

	for (i = 0; i < 6; i++) {
		skymins[0][i] = skymins[1][i] = 9999;
		skymaxs[0][i] = skymaxs[1][i] = -9999;
	}
}


vec2_t SkyTexCoordArray [2 * MAX_TRIANGLES];
vec3_t SkyVertexArray	[3 * MAX_TRIANGLES];
vec4_t SkyColorArray	[4 * MAX_TRIANGLES];
unsigned	skyIndex[MAX_INDICES];
static int numVerts, myIndex;

void MakeSkyVec(float s, float t, int axis)
{
	vec3_t v, b;
	int j, k;

	b[0] = s * 16384;// 2300;
	b[1] = t * 16384;//2300;
	b[2] = 16384;//2300;

	for (j = 0; j < 3; j++) {
		k = st_to_vec[axis][j];
		if (k < 0)
			v[j] = -b[-k - 1];
		else
			v[j] = b[k - 1];
	}

	// avoid bilerp seam
	s = (s + 1) * 0.5;
	t = (t + 1) * 0.5;

	if (s < sky_min)
		s = sky_min;
	else if (s > sky_max)
		s = sky_max;
	if (t < sky_min)
		t = sky_min;
	else if (t > sky_max)
		t = sky_max;

	t = 1.0 - t;

	VA_SetElem3(SkyVertexArray[numVerts],	v[0], v[1], v[2]);
	VA_SetElem2(SkyTexCoordArray[numVerts], s, t);
	VA_SetElem4(SkyColorArray[numVerts],	1, 1, 1, 1);

	skyIndex[myIndex++] = numVerts+0;
	skyIndex[myIndex++] = numVerts+1;
	skyIndex[myIndex++] = numVerts+3;
	skyIndex[myIndex++] = numVerts+3;
	skyIndex[myIndex++] = numVerts+1;
	skyIndex[myIndex++] = numVerts+2;

	numVerts++;

}

/*
==============
R_DrawSkyBox
==============
*/
int skytexorder[6] = { 0, 2, 1, 3, 4, 5 };
void R_DrawSkyBox(void)
{
	int i, id;
	unsigned defBits = 0;
	
	defBits = worldDefs.AttribColorBits;
	GL_BindProgram(genericProgram, defBits);
	id = genericProgram->id[defBits];
	qglUniform1i(qglGetUniformLocation(id, "u_map"), 0);
	qglUniform1f(qglGetUniformLocation(id, "u_colorScale"), 1.0);

	qglEnableVertexAttribArray(ATRB_POSITION);
	qglEnableVertexAttribArray(ATRB_TEX0);
	qglEnableVertexAttribArray(ATRB_COLOR);

	qglVertexAttribPointer(ATRB_TEX0, 2, GL_FLOAT, false, 0, SkyTexCoordArray);
	qglVertexAttribPointer(ATRB_POSITION, 3, GL_FLOAT, false, 0, SkyVertexArray);
	qglVertexAttribPointer(ATRB_COLOR, 4, GL_FLOAT, false, 0, SkyColorArray);
	
	if (skyrotate) {			// check for no sky at all
		for (i = 0; i < 6; i++)
			if (skymins[0][i] < skymaxs[0][i]
				&& skymins[1][i] < skymaxs[1][i])
				break;
		if (i == 6)
			return;				// nothing visible
	}

	qglPushMatrix();
	qglTranslatef(r_origin[0], r_origin[1], r_origin[2]);
	qglRotatef(r_newrefdef.time * skyrotate, skyaxis[0], skyaxis[1],
			   skyaxis[2]);
	
	for (i = 0; i < 6; i++) {
		
		if (skyrotate) {		// hack, forces full sky to draw when
								// rotating
			skymins[0][i] = -1;
			skymins[1][i] = -1;
			skymaxs[0][i] = 1;
			skymaxs[1][i] = 1;
		}

		if (skymins[0][i] >= skymaxs[0][i]
			|| skymins[1][i] >= skymaxs[1][i])
			continue;

		GL_MBind(GL_TEXTURE0_ARB, sky_images[skytexorder[i]]->texnum);
		
		numVerts = myIndex = 0;
		
		MakeSkyVec(skymins[0][i], skymins[1][i], i);
		MakeSkyVec(skymins[0][i], skymaxs[1][i], i);
		MakeSkyVec(skymaxs[0][i], skymaxs[1][i], i);
		MakeSkyVec(skymaxs[0][i], skymins[1][i], i);

		qglDrawElements(GL_TRIANGLES, myIndex, GL_UNSIGNED_INT, skyIndex);
	}

	qglDisableVertexAttribArray(ATRB_POSITION);
	qglDisableVertexAttribArray(ATRB_TEX0);
    qglDisableVertexAttribArray(ATRB_COLOR);
	GL_BindNullProgram();
	qglPopMatrix();
}


/*
============
R_SetSky
============
*/
// 3dstudio environment map names
char *suf[6] = { "rt", "bk", "lf", "ft", "up", "dn" };

void R_SetSky(char *name, float rotate, vec3_t axis)
{
	int i;
	char pathname[MAX_QPATH];

	strncpy(skyname, name, sizeof(skyname) - 1);
	skyrotate = rotate;
	VectorCopy(axis, skyaxis);

	for (i = 0; i < 6; i++) {
	
		Com_sprintf(pathname, sizeof(pathname), "env/%s%s.tga", skyname,
					suf[i]);

		sky_images[i] = GL_FindImage(pathname, it_sky);
		if (!sky_images[i])
			sky_images[i] = r_notexture;

		// Com_Printf("sky box is: %s\n",pathname );
	
			sky_min = 0.001953125f;
			sky_max = 0.998046875f;
		
	}
}

