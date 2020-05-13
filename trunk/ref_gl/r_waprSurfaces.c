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

#include "r_local.h"

void R_AddWarpPolygons(msurface_t *fa) {
	glpoly_t	*p;
	float		*v, alpha;
	int			i, nv = fa->polys->numVerts;
	int			numIdx = 0, numVerts = 0;	

	if (fa->texInfo->flags & (SURF_TRANS33 | SURF_TRANS66)) {
		alpha =0.33f;
		qglUniform1i (U_WATER_TRANS, 1);

	}
	else {
		qglUniform1i (U_WATER_TRANS, 0);
		alpha = 1.f;
	}

	GL_SetBindlessTexture(U_TMU0, fa->texInfo->image->handle);

	p = fa->polys;
	v = p->verts[0];
	c_brush_polys += (nv - 2);

	for (i = 0; i < nv - 2; i++) {
		indexArray[numIdx++] = numVerts;
		indexArray[numIdx++] = numVerts + i + 1;
		indexArray[numIdx++] = numVerts + i + 2;
	}

	for (i = 0; i < p->numVerts; i++, v += VERTEXSIZE) {
		VectorCopy (v, wVertexArray[i]);

		wTexArray[i][0] = v[3];
		wTexArray[i][1] = v[4];

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

	qglDrawElements(GL_TRIANGLES, numIdx, GL_UNSIGNED_INT, indexArray);
}