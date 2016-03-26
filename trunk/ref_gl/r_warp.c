/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2004-2015 Quake2xp Team.

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
char skyname[MAX_QPATH];
float skyrotate;
vec3_t skyaxis;
image_t *sky_images[6];
static float shadelight[3];
void IL_LoadImage (char *filename, byte ** pic, int *width, int *height, ILenum type);
unsigned int	skyCube = -1;

#define DST_SIZE 16
unsigned int dst_texture = 0;

/*
===============
CreateDSTTex

Create the texture which warps texture shaders
===============
*/
void CreateDSTTex () {
	signed char data[DST_SIZE][DST_SIZE][2];
	int x, y;



	for (x = 0; x < DST_SIZE; x++)
	for (y = 0; y < DST_SIZE; y++) {
		data[x][y][0] = rand () % 255 - 128;
		data[x][y][1] = rand () % 255 - 128;
	}

	qglGenTextures (1, &dst_texture);
	qglBindTexture (GL_TEXTURE_2D, dst_texture);
	qglTexImage2D (GL_TEXTURE_2D, 0, 4, DST_SIZE, DST_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	qglGenerateMipmap (GL_TEXTURE_2D);
	qglTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	qglTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	qglTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	qglTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

void R_DrawWaterPolygons (msurface_t *fa, qboolean bmodel) {
	glpoly_t	*p, *bp;
	float		*v, dstscroll, ambient, alpha;
	int			i, nv = fa->polys->numVerts;

	// setup program
	GL_BindProgram (waterProgram, 0);

	if (fa->texInfo->flags & (SURF_TRANS33 | SURF_TRANS66)) {
		alpha = (fa->texInfo->flags & SURF_TRANS33) ? 0.33f : 0.66f;
		qglUniform1i (water_trans, 1);

	}
	else {
		qglUniform1i (water_trans, 0);
		alpha = 1.f;
	}

	ambient = max(r_lightmapScale->value, 0.33f);

	GL_MBind (GL_TEXTURE0_ARB, fa->texInfo->image->texnum);
	GL_MBind (GL_TEXTURE1_ARB, r_DSTTex->texnum);
	GL_MBindRect (GL_TEXTURE2_ARB, ScreenMap->texnum);
	GL_MBindRect (GL_TEXTURE3_ARB, depthMap->texnum);

	qglUniform1f (water_deformMul, 1.0);
	qglUniform1f (water_thickness, 150.0);
	qglUniform2f (water_screenSize, vid.width, vid.height);
	qglUniform2f (water_depthParams, r_newrefdef.depthParms[0], r_newrefdef.depthParms[1]);
	qglUniform1f (water_colorModulate, r_textureColorScale->value);
	qglUniform1f (water_ambient, ambient);
	if (!bmodel)
		qglUniformMatrix4fv(water_mvp, 1, qfalse, (const float *)r_newrefdef.modelViewProjectionMatrix);
	else
		qglUniformMatrix4fv(water_mvp, 1, qfalse, (const float *)currententity->orMatrix);

	if(r_newrefdef.rdflags & RDF_UNDERWATER)
		qglUniform1i(water_mirror, 0);
	else
		qglUniform1i(water_mirror, 1);

	qglUniformMatrix4fv(water_mv, 1, qfalse, (const float *)r_newrefdef.modelViewMatrix);
	qglUniformMatrix4fv(water_pm, 1, qfalse, (const float *)r_newrefdef.projectionMatrix);

	dstscroll = (r_newrefdef.time * 0.15f) - (int)(r_newrefdef.time * 0.15f);

	qglEnableVertexAttribArray (ATT_POSITION);
	qglEnableVertexAttribArray (ATT_TEX0);
	qglEnableVertexAttribArray (ATT_TEX2);
	qglEnableVertexAttribArray (ATT_NORMAL);
	qglEnableVertexAttribArray (ATT_TANGENT);
	qglEnableVertexAttribArray (ATT_BINORMAL);
	qglEnableVertexAttribArray (ATT_COLOR);

	qglVertexAttribPointer (ATT_POSITION, 3, GL_FLOAT, qfalse, 0, wVertexArray);
	qglVertexAttribPointer (ATT_TEX0, 2, GL_FLOAT, qfalse, 0, wTexArray);
	qglVertexAttribPointer (ATT_TEX2, 2, GL_FLOAT, qfalse, 0, wTmu2Array);
	qglVertexAttribPointer (ATT_COLOR, 4, GL_FLOAT, qfalse, 0, wColorArray);
	qglVertexAttribPointer (ATT_NORMAL, 3, GL_FLOAT, qfalse, 0, nTexArray);
	qglVertexAttribPointer (ATT_TANGENT, 3, GL_FLOAT, qfalse, 0, tTexArray);
	qglVertexAttribPointer (ATT_BINORMAL, 3, GL_FLOAT, qfalse, 0, bTexArray);

	for (bp = fa->polys; bp; bp = bp->next) {
		p = bp;
		c_brush_polys += (nv - 2);

		for (i = 0, v = p->verts[0]; i < p->numVerts; i++, v += VERTEXSIZE) {
			VectorCopy (v, wVertexArray[i]);

			wTexArray[i][0] = v[3];
			wTexArray[i][1] = v[4];

			wTmu2Array[i][0] = (v[3] + dstscroll);
			wTmu2Array[i][1] = (v[4] + dstscroll);

			// normals
			nTexArray[i][0] = v[7];
			nTexArray[i][1] = v[8];
			nTexArray[i][2] = v[9];

			tTexArray[i][0] = v[10];
			tTexArray[i][1] = v[11];
			tTexArray[i][2] = v[12];

			bTexArray[i][0] = v[13];
			bTexArray[i][1] = v[14];
			bTexArray[i][2] = v[15];

			wColorArray[i][3] = alpha;
		}

		qglDrawElements (GL_TRIANGLES, fa->numIndices, GL_UNSIGNED_SHORT, fa->indices);
	}

	GL_SelectTexture (GL_TEXTURE0_ARB);

	qglDisableVertexAttribArray (ATT_POSITION);
	qglDisableVertexAttribArray (ATT_TEX0);
	qglDisableVertexAttribArray (ATT_TEX2);
	qglDisableVertexAttribArray (ATT_NORMAL);
	qglDisableVertexAttribArray (ATT_TANGENT);
	qglDisableVertexAttribArray (ATT_BINORMAL);
	qglDisableVertexAttribArray (ATT_COLOR);

	GL_BindNullProgram ();
}


//===================================================================


vec3_t skyclip[6] = {
	{ 1, 1, 0 }
	,
	{ 1, -1, 0 }
	,
	{ 0, -1, 1 }
	,
	{ 0, 1, 1 }
	,
	{ 1, 0, 1 }
	,
	{ -1, 0, 1 }
};
int c_sky;

// 1 = s, 2 = t, 3 = 2048
int st_to_vec[6][3] = {
	{ 3, -1, 2 },
	{ -3, 1, 2 },

	{ 1, 3, 2 },
	{ -1, -3, 2 },

	{ -2, -1, 3 },				// 0 degrees yaw, look straight up
	{ 2, -1, -3 }					// look straight down

	//  {-1,2,3},
	//  {1,2,-3}
};

// s = [0]/[2], t = [1]/[2]
int vec_to_st[6][3] = {
	{ -2, 3, 1 },
	{ 2, 3, -1 },

	{ 1, 3, 2 },
	{ -1, 3, -2 },

	{ -2, -1, 3 },
	{ -2, 1, -3 }

	//  {-1,2,3},
	//  {1,2,-3}
};

float skymins[2][6], skymaxs[2][6];
float sky_min, sky_max;

void DrawSkyPolygon (int nump, vec3_t vecs) {
	int i, j;
	vec3_t v, av;
	float s, t, dv;
	int axis;
	float *vp;

	c_sky++;

	// decide which face it maps to
	VectorCopy (vec3_origin, v);
	for (i = 0, vp = vecs; i < nump; i++, vp += 3) {
		VectorAdd (vp, v, v);
	}
	av[0] = fabs (v[0]);
	av[1] = fabs (v[1]);
	av[2] = fabs (v[2]);
	if (av[0] > av[1] && av[0] > av[2]) {
		if (v[0] < 0)
			axis = 1;
		else
			axis = 0;
	}
	else if (av[1] > av[2] && av[1] > av[0]) {
		if (v[1] < 0)
			axis = 3;
		else
			axis = 2;
	}
	else {
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
void ClipSkyPolygon (int nump, vec3_t vecs, int stage) {
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
		VID_Error (ERR_DROP, "ClipSkyPolygon: MAX_CLIP_VERTS");
	if (stage == 6) {			// fully clipped, so draw it
		DrawSkyPolygon (nump, vecs);
		return;
	}

	front = back = qfalse;
	norm = skyclip[stage];
	for (i = 0, v = vecs; i < nump; i++, v += 3) {
		d = DotProduct (v, norm);
		if (d > ON_EPSILON) {
			front = qtrue;
			sides[i] = SIDE_FRONT;
		}
		else if (d < -ON_EPSILON) {
			back = qtrue;
			sides[i] = SIDE_BACK;
		}
		else
			sides[i] = SIDE_ON;
		dists[i] = d;
	}

	if (!front || !back) {		// not clipped
		ClipSkyPolygon (nump, vecs, stage + 1);
		return;
	}
	// clip it
	sides[i] = sides[0];
	dists[i] = dists[0];
	VectorCopy (vecs, (vecs + (i * 3)));
	newc[0] = newc[1] = 0;

	for (i = 0, v = vecs; i < nump; i++, v += 3) {
		switch (sides[i]) {
			case SIDE_FRONT:
				VectorCopy (v, newv[0][newc[0]]);
				newc[0]++;
				break;
			case SIDE_BACK:
				VectorCopy (v, newv[1][newc[1]]);
				newc[1]++;
				break;
			case SIDE_ON:
				VectorCopy (v, newv[0][newc[0]]);
				newc[0]++;
				VectorCopy (v, newv[1][newc[1]]);
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
	ClipSkyPolygon (newc[0], newv[0][0], stage + 1);
	ClipSkyPolygon (newc[1], newv[1][0], stage + 1);
}

/*
=================
R_AddSkySurface
=================
*/
void R_AddSkySurface (msurface_t * fa) {
	int i;
	vec3_t verts[MAX_CLIP_VERTS];
	glpoly_t *p;

	// calculate vertex values for sky box
	for (p = fa->polys; p; p = p->next) {
		for (i = 0; i < p->numVerts; i++) {
			VectorSubtract (p->verts[i], r_origin, verts[i]);
		}
		ClipSkyPolygon (p->numVerts, verts[0], 0);
	}
}


/*
==============
R_ClearSkyBox
==============
*/
void R_ClearSkyBox (void) {
	int i;

	for (i = 0; i < 6; i++) {
		skymins[0][i] = skymins[1][i] = 9999;
		skymaxs[0][i] = skymaxs[1][i] = -9999;
	}
}


vec2_t		SkyTexCoordArray[2 * MAX_TRIANGLES];
vec3_t		SkyVertexArray[3 * MAX_TRIANGLES];
vec4_t		SkyColorArray[4 * MAX_TRIANGLES];
index_t		skyIndex[MAX_INDICES];
static int	numVerts, idx;

void MakeSkyVec (float s, float t, int axis) {
	vec3_t v, b;
	int j, k;

	b[0] = s *	2300;
	b[1] = t *	2300;
	b[2] =		2300;

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

	VA_SetElem3 (SkyVertexArray[numVerts], v[0], v[1], v[2]);
	VA_SetElem2 (SkyTexCoordArray[numVerts], s, t);
	VA_SetElem4 (SkyColorArray[numVerts], 1, 1, 1, 1);

	skyIndex[idx++] = numVerts + 0;
	skyIndex[idx++] = numVerts + 1;
	skyIndex[idx++] = numVerts + 3;
	skyIndex[idx++] = numVerts + 3;
	skyIndex[idx++] = numVerts + 1;
	skyIndex[idx++] = numVerts + 2;

	numVerts++;

}

/*
==============
R_DrawSkyBox
==============
*/
int skytexorder[6] = { 0, 2, 1, 3, 4, 5 };
void R_DrawSkyBox (qboolean color) {
	int i;

	if (color) {
		GL_BindProgram (genericProgram, 0);
		qglUniform1i (gen_sky, 1);
		qglUniform1i (gen_3d, 0);
		qglUniform1i (gen_attribColors, 0);
		qglUniform1i (gen_attribConsole, 0);
		qglUniform1f (gen_colorModulate, 1.0);

		qglEnableVertexAttribArray (ATT_TEX0);
		qglEnableVertexAttribArray (ATT_COLOR);
		qglVertexAttribPointer (ATT_TEX0, 2, GL_FLOAT, qfalse, 0, SkyTexCoordArray);
		qglVertexAttribPointer (ATT_COLOR, 4, GL_FLOAT, qfalse, 0, SkyColorArray);
		GL_Enable(GL_POLYGON_OFFSET_FILL);
		GL_PolygonOffset(-1, -1);

	}
	qglEnableVertexAttribArray (ATT_POSITION);
	qglVertexAttribPointer (ATT_POSITION, 3, GL_FLOAT, qfalse, 0, SkyVertexArray);



	if (skyrotate) {			// check for no sky at all
		for (i = 0; i < 6; i++)
		if (skymins[0][i] < skymaxs[0][i]
			&& skymins[1][i] < skymaxs[1][i])
			break;
		if (i == 6)
			return;	// nothing visible
	}

	if(color)
		qglUniformMatrix4fv(gen_mvp, 1, qfalse, (const float *)r_newrefdef.skyMatrix);
	else
		qglUniformMatrix4fv(null_mvp, 1, qfalse, (const float *)r_newrefdef.skyMatrix);

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

		if (color)
			GL_MBind (GL_TEXTURE0_ARB, sky_images[skytexorder[i]]->texnum);

		numVerts = idx = 0;

		MakeSkyVec (skymins[0][i], skymins[1][i], i);
		MakeSkyVec (skymins[0][i], skymaxs[1][i], i);
		MakeSkyVec (skymaxs[0][i], skymaxs[1][i], i);
		MakeSkyVec (skymaxs[0][i], skymins[1][i], i);

		qglDrawElements (GL_TRIANGLES, idx, GL_UNSIGNED_SHORT, skyIndex);
	}

	qglDisableVertexAttribArray (ATT_POSITION);
	
	if (color) {
		qglDisableVertexAttribArray (ATT_TEX0);
		qglDisableVertexAttribArray (ATT_COLOR);
		GL_BindNullProgram ();
		GL_Disable(GL_POLYGON_OFFSET_FILL);
	}
}


/*
============
R_SetSky
============
*/
// 3dstudio environment map names
char *suf[6] = { "rt", "bk", "lf", "ft", "up", "dn" };

void R_SetSky (char *name, float rotate, vec3_t axis) {
	int i;
	char pathname[MAX_QPATH];

	strncpy (skyname, name, sizeof(skyname)-1);
	skyrotate = rotate;
	VectorCopy (axis, skyaxis);

	for (i = 0; i < 6; i++) {

		Com_sprintf (pathname, sizeof(pathname), "env/%s%s.tga", skyname,
			suf[i]);

		sky_images[i] = GL_FindImage (pathname, it_sky);
		if (!sky_images[i])
			sky_images[i] = r_notexture;

		// Com_Printf("sky box is: %s\n",pathname );

		sky_min = 0.001953125f;
		sky_max = 0.998046875f;

	}
}

void R_GenSkyCubeMap (char *name) {
	int		i, w, h;
	char	pathname[MAX_QPATH];
	byte	*pic;

	strncpy (skyname, name, sizeof(skyname)-1);

	qglDisable (GL_TEXTURE_2D);
	qglEnable (GL_TEXTURE_CUBE_MAP_ARB);

	qglGenTextures (1, &skyCube);
	qglBindTexture (GL_TEXTURE_CUBE_MAP_ARB, skyCube);

	for (i = 0; i < 6; i++) {
		Com_sprintf (pathname, sizeof(pathname), "env/%s%s.tga", skyname, suf[i]);

		IL_LoadImage (pathname, &pic, &w, &h, IL_TGA);
		if (pic) {
			qglTexImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + i, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pic);
			free (pic);
		}
	}

	qglTexParameteri (GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	qglTexParameteri (GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexParameteri (GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	qglTexParameteri (GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	qglTexParameteri (GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	qglDisable (GL_TEXTURE_CUBE_MAP_ARB);
	qglEnable (GL_TEXTURE_2D);
}