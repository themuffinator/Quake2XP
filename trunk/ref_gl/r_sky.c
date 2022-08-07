/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
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
// gl_warp.c -- sky surfaces

#include "r_local.h"
extern model_t* loadmodel;
char skyname[MAX_QPATH];
float skyrotate;
vec3_t skyaxis;
vec3_t		SkyVertexArray[MAX_TRIANGLES];
index_t		skyIndex[MAX_INDICES];
static int	numSkyVerts, numSkyIdx;
void IL_LoadImage(char* filename, byte** pic, int* width, int* height, ILenum type);

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

void DrawSkyPolygon(int nump, vec3_t vecs) {
	int i, j;
	vec3_t v, av;
	float s, t, dv;
	int axis;
	float* vp;

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
void ClipSkyPolygon(int nump, vec3_t vecs, int stage) {
	float* norm;
	float* v;
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

	front = back = qfalse;
	norm = skyclip[stage];
	for (i = 0, v = vecs; i < nump; i++, v += 3) {
		d = DotProduct(v, norm);
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
void R_AddSkySurface(msurface_t* fa) {
	int i;
	vec3_t verts[MAX_CLIP_VERTS];
	glpoly_t* p;

	// calculate vertex values for sky box
	for (p = fa->polys; p; p = p->next) {
		for (i = 0; i < p->numVerts; i++) {
			VectorSubtract(p->verts[i], r_origin, verts[i]);
		}
		ClipSkyPolygon(p->numVerts, verts[0], 0);
	}
}


/*
==============
R_ClearSkyBox
==============
*/
void R_ClearSkyBox(void) {
	int i;

	for (i = 0; i < 6; i++) {
		skymins[0][i] = skymins[1][i] = 9999;
		skymaxs[0][i] = skymaxs[1][i] = -9999;
	}
}


void GenSkyVertices(float x, float y, int axis) {
	vec3_t v, b;
	int j, k;

	b[0] = x * 8192;
	b[1] = y * 8192;
	b[2] = 8192;

	for (j = 0; j < 3; j++) {
		k = st_to_vec[axis][j];
		if (k < 0)
			v[j] = -b[-k - 1];
		else
			v[j] = b[k - 1];
	}
	VA_SetElem3(SkyVertexArray[numSkyVerts], v[0], v[1], v[2]);
	numSkyVerts++;

}

/*
==============
R_DrawSkyBox
==============
*/
void R_DrawSkyBox(qboolean color) {
	int i;

	glBindVertexArray(vao.dynamic);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_dynamic);

	GL_BindProgram(skyProgram);

	if (color)
		qglUniform1i(U_PARAM_INT_0, 1);
	else
		qglUniform1i(U_PARAM_INT_0, 0); // depth pass

	if(r_earthSky->integer && color)
		qglUniform1i(U_PARAM_INT_1, 1);
	else
		qglUniform1i(U_PARAM_INT_1, 0);
	
	float rad = 10471e3;
	float angle = 1.0 + r_newrefdef.time * 0.125;
	float x = rad * cos(angle); 
	float hipos = rad * sin(angle);

	qglUniform4f(U_PARAM_VEC4_0, 0, x, hipos, r_earthSunIntens->value);

	qglUniformMatrix4fv(U_MVP_MATRIX, 1, qfalse, (const float*)r_newrefdef.skyMatrix);

	numSkyVerts = numSkyIdx = 0;

	if (skyrotate) {			// check for no sky at all
		for (i = 0; i < 6; i++)
			if (skymins[0][i] < skymaxs[0][i] && skymins[1][i] < skymaxs[1][i])
				break;

		if (i == 6) {
			glBindVertexArray(0);
			qglBindBuffer(GL_ARRAY_BUFFER, 0);
			return;	// nothing visible
		}
	}
	
	if (color)
		GL_SetBindlessTexture(U_TMU0, skyCube_handle);

	for (i = 0; i < 6; i++) {

		if (skyrotate) {		// hack, forces full sky to draw when rotating
			skymins[0][i] = -1;
			skymins[1][i] = -1;
			skymaxs[0][i] = 1;
			skymaxs[1][i] = 1;
		}

		if (skymins[0][i] >= skymaxs[0][i] 
			|| skymins[1][i] >= skymaxs[1][i])
			continue;

		skyIndex[numSkyIdx++] = numSkyVerts + 0;
		skyIndex[numSkyIdx++] = numSkyVerts + 1;
		skyIndex[numSkyIdx++] = numSkyVerts + 3;
		skyIndex[numSkyIdx++] = numSkyVerts + 3;
		skyIndex[numSkyIdx++] = numSkyVerts + 1;
		skyIndex[numSkyIdx++] = numSkyVerts + 2;

		GenSkyVertices(skymins[0][i], skymins[1][i], i);
		GenSkyVertices(skymins[0][i], skymaxs[1][i], i);
		GenSkyVertices(skymaxs[0][i], skymaxs[1][i], i);
		GenSkyVertices(skymaxs[0][i], skymins[1][i], i);	
	}
	qglBufferSubData(GL_ARRAY_BUFFER, 0, numSkyVerts * sizeof(vec3_t), SkyVertexArray);
	qglBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, numSkyVerts * 3 * sizeof(uint), skyIndex);

	qglDrawElements(GL_TRIANGLES, numSkyIdx, GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(0);
	qglBindBuffer(GL_ARRAY_BUFFER, 0);
}


/*
============
R_SetSky
============
*/
// 3dstudio environment map names
// fix q2 skybox faces - flip back and left sides 
char* cubeSufGL[6] = { "rt", "lf", "bk", "ft", "up", "dn" };

void R_SetSky(char* name, float rotate, vec3_t axis) {

	skyrotate = rotate;
	VectorCopy(axis, skyaxis);
	sky_min = 0.001953125f;
	sky_max = 0.998046875f;

}

#define STB_IMAGE_IMPLEMENTATION
#include "ImageLib/stb_image.h"

uint	transi[4096 * 4096];
float	transf[1024 * 1024];

void R_FlipImageFloat(int i, hdri_t* hdri, float* dst) {
	float* from;
	float* src = hdri->data;
	int	width = hdri->width;
	int	height = hdri->height;
	int	x, y;

	if (i == 1)		// bk
	{
		for (y = height - 1; y >= 0; y--) {
			for (x = width - 1; x >= 0; x--) {	// copy rgb components
				from = src + (x * height + y) * 3;
				dst[0] = from[0];
				dst[1] = from[1];
				dst[2] = from[2];
				dst += 3;
			}
		}
		return;
	}

	if (i == 2)		// lf
	{
		for (y = height - 1; y >= 0; y--) {
			for (x = 0; x < width; x++) {	// copy rgb components
				from = src + (y * width + x) * 3;
				dst[0] = from[0];
				dst[1] = from[1];
				dst[2] = from[2];
				dst += 3;
			}
		}
		return;
	}

	if (i == 3)		// rt
	{
		for (y = 0; y < height; y++) {
			for (x = width - 1; x >= 0; x--) {	// copy rgb components
				from = src + (y * width + x) * 3;
				dst[0] = from[0];
				dst[1] = from[1];
				dst[2] = from[2];
				dst += 3;
			}
		}
		return;
	}

	// ft, up, dn
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {	// copy rgb components
			from = src + (x * height + y) * 3;
			dst[0] = from[0];
			dst[1] = from[1];
			dst[2] = from[2];
			dst += 3;
		}
	}
}

void R_GenSkyCubeMap(char* name) {
	int		i;
	char	pathname[MAX_QPATH];
	img_t	pix[6];
	hdri_t	hdri[6];

	qboolean hdr = qfalse;

	strncpy(skyname, name, sizeof(skyname) - 1);

	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &skyCube);

	Com_sprintf(pathname, sizeof(pathname), "%s/env/hdr/%s%s.hdr", FS_Gamedir(), skyname, cubeSufGL[0]);
	if (stbi_is_hdr(pathname)) {
		hdr = qtrue;
	}

	int bpp = 0;
	for (i = 0; i < 6; i++) {
		pix[i].pixels = NULL;
		pix[i].width = pix[i].height = 0;
		
		hdri[i].width = hdri[i].height = 0;
		hdri[i].data = NULL;

		if (hdr) {
			Com_sprintf(pathname, sizeof(pathname), "%s/env/hdr/%s%s.hdr", FS_Gamedir(), skyname, cubeSufGL[i]);
			hdri[i].data = stbi_loadf(pathname, &hdri[i].width, &hdri[i].height, &bpp, 0);
		}
		else {
			Com_sprintf(pathname, sizeof(pathname), "env/%s%s.tga", skyname, cubeSufGL[i]);

			// Berserker: stop spam
			if (FS_LoadFile(pathname, NULL) != -1) {

				IL_LoadImage(pathname, &pix[i].pixels, &pix[i].width, &pix[i].height, IL_TGA);
			}
		}
	}

	int numMips;
	if (hdr) {
		numMips = CalcMipmapCount(hdri[0].width, hdri[0].height);
		glTextureStorage2D(skyCube, numMips, GL_RGB32F, hdri[0].width, hdri[0].height);
	}
	else {
		numMips = CalcMipmapCount(pix[0].width, pix[0].height);
		glTextureStorage2D(skyCube, numMips, GL_RGB8, pix[0].width, pix[0].height);
	}

	for (i = 0; i < 6; i++) {

		if (!hdr) {
			R_FlipImage(i, &pix[i], (byte*)transi);
			free(pix[i].pixels);
			glTextureSubImage3D(skyCube, 0, 0, 0, i, pix[i].width, pix[i].height, 1, GL_RGB, GL_UNSIGNED_BYTE, transi);
		}
		else {
			R_FlipImageFloat(i, &hdri[i], transf);
			stbi_image_free(hdri[i].data);
			glTextureSubImage3D(skyCube, 0, 0, 0, i, hdri[i].width, hdri[i].height, 1, GL_RGB, GL_FLOAT,transf);
		}
	}

	glTextureParameteri(skyCube, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(skyCube, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(skyCube, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(skyCube, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(skyCube, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteri(skyCube, GL_TEXTURE_BASE_LEVEL, 0);
	glTextureParameteri(skyCube, GL_TEXTURE_MAX_LEVEL, numMips);
	glGenerateTextureMipmap(skyCube);

	skyCube_handle = glGetTextureHandleARB(skyCube);
	glMakeTextureHandleResidentARB(skyCube_handle);
}