/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
#include "r_local.h"
 
uchar cube_idx[] = {
	// front
	0, 1, 2,
	2, 3, 0,
	// right
	1, 5, 6,
	6, 2, 1,
	// back
	7, 6, 5,
	5, 4, 7,
	// left
	4, 0, 3,
	3, 7, 4,
	// bottom
	4, 5, 1,
	1, 0, 4,
	// top
	3, 2, 6,
	6, 7, 3
};
uint	ibo_md3Shadow[MD3_MAX_VERTS * MD3_MAX_MESHES];
ushort	ibo_quad[] = { 0, 1, 2, 0, 2, 3 }; //index buffer for 2d drawing

void R_InitVertexBuffers() {

	vec2_t		tmpVerts[4];
	int			i;

	Com_Printf("Initializing Vertex Buffers: ");

	qglGenBuffers(1, &vbo.ibo_quad);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quad);
	qglBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ibo_quad), ibo_quad, GL_STATIC_DRAW);

	// precalc screen quads for postprocessing
	// full quad
	VA_SetElem2(tmpVerts[0], 0, vid.height);
	VA_SetElem2(tmpVerts[1], vid.width, vid.height);
	VA_SetElem2(tmpVerts[2], vid.width, 0);
	VA_SetElem2(tmpVerts[3], 0, 0);

	qglGenBuffers(1, &vbo.vbo_fullScreenQuad);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_fullScreenQuad);
	qglBufferData(GL_ARRAY_BUFFER, sizeof(vec2_t) * 4, tmpVerts, GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao.fullscreenQuad);
	glBindVertexArray(vao.fullscreenQuad);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_fullScreenQuad);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quad);

	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 2, GL_FLOAT, qfalse, 0, 0);

	glBindVertexArray(0);

	// half quad
	VA_SetElem2(tmpVerts[0], 0, vid.height);
	VA_SetElem2(tmpVerts[1], vid.width * 0.5, vid.height);
	VA_SetElem2(tmpVerts[2], vid.width * 0.5, vid.height * 0.5);
	VA_SetElem2(tmpVerts[3], 0, vid.height * 0.5);

	qglGenBuffers(1, &vbo.vbo_halfScreenQuad);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_halfScreenQuad);
	qglBufferData(GL_ARRAY_BUFFER, sizeof(vec2_t) * 4, tmpVerts, GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao.halfScreenQuad);
	glBindVertexArray(vao.halfScreenQuad);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_halfScreenQuad);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quad);

	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 2, GL_FLOAT, qfalse, 0, 0);

	glBindVertexArray(0);

	// quater quad
	VA_SetElem2(tmpVerts[0], 0, vid.height);
	VA_SetElem2(tmpVerts[1], vid.width * 0.25, vid.height);
	VA_SetElem2(tmpVerts[2], vid.width * 0.25, vid.height * 0.25);
	VA_SetElem2(tmpVerts[3], 0, vid.height * 0.25);

	qglGenBuffers(1, &vbo.vbo_quarterScreenQuad);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_quarterScreenQuad);
	qglBufferData(GL_ARRAY_BUFFER, sizeof(vec2_t) * 4, tmpVerts, GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao.quaterScreenQuad);
	glBindVertexArray(vao.quaterScreenQuad);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_quarterScreenQuad);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quad);

	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 2, GL_FLOAT, qfalse, 0, 0);

	glBindVertexArray(0);
	
	//------------------------------------------------
	// md2-3 shadows

	qglGenBuffers(1, &vbo.vbo_aliasShadow);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_aliasShadow);
	qglBufferData(GL_ARRAY_BUFFER, MAX_STREAM_VBO_VERTS * sizeof(vec4_t), 0, GL_STREAM_DRAW);

	qglGenBuffers(1, &vbo.ibo_md2Shadow);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_md2Shadow);
	qglBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_STREAM_IBO_IDX * sizeof(uint), 0, GL_STREAM_DRAW);

	for (i = 0; i < MD3_MAX_VERTS * MD3_MAX_MESHES; i++)
		ibo_md3Shadow[i] = i;

	qglGenBuffers(1, &vbo.ibo_md3Shadow);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_md3Shadow);
	qglBufferData(GL_ELEMENT_ARRAY_BUFFER, i * sizeof(uint), ibo_md3Shadow, GL_STATIC_DRAW);

//--------------------
	glGenVertexArrays(1, &vao.md2Shadow);
	glBindVertexArray(vao.md2Shadow);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_aliasShadow);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_md2Shadow);
	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 4, GL_FLOAT, qfalse, 0, 0);

	glBindVertexArray(0);
//---------------------
	glGenVertexArrays(1, &vao.md3Shadow);
	glBindVertexArray(vao.md3Shadow);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_aliasShadow);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_md3Shadow);
	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 4, GL_FLOAT, qfalse, 0, 0);

	glBindVertexArray(0);

//---------------------------------------------------

	qglGenBuffers(1, &vbo.vbo_dynamic);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_dynamic);
	qglBufferData(GL_ARRAY_BUFFER, MAX_STREAM_VBO_VERTS * sizeof(vec3_t), 0, GL_DYNAMIC_DRAW);

	qglGenBuffers(1, &vbo.ibo_dynamic);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_dynamic);
	qglBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_STREAM_IBO_IDX * sizeof(uint), 0, GL_DYNAMIC_DRAW);

	glGenVertexArrays(1, &vao.dynamic);
	glBindVertexArray(vao.dynamic);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_dynamic);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_dynamic);
	
	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, 0);
	
	glBindVertexArray(0);
//------------------------------
	qglGenBuffers(1, &vbo.ibo_cube);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_cube);
	qglBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_idx), cube_idx, GL_STATIC_DRAW);

	qglBindBuffer(GL_ARRAY_BUFFER, 0);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	Com_Printf(S_COLOR_GREEN"ok\n\n");	
}

void R_ShutDownVertexBuffers() {

	qglDeleteBuffers(1, &vbo.vbo_fullScreenQuad);
	qglDeleteBuffers(1, &vbo.vbo_halfScreenQuad);
	qglDeleteBuffers(1, &vbo.vbo_quarterScreenQuad);
	qglDeleteBuffers(1, &vbo.vbo_aliasShadow);
	qglDeleteBuffers(1, &vbo.vbo_BSP);
	qglDeleteBuffers(1, &vbo.vbo_draw2d);

	qglDeleteBuffers(1, &vbo.ibo_quad);
	qglDeleteBuffers(1, &vbo.ibo_md2Shadow);
	qglDeleteBuffers(1, &vbo.ibo_md3Shadow);
	qglDeleteBuffers(1, &vbo.ibo_dynamic);
	qglDeleteBuffers(1, &vbo.ibo_cube);

	glDeleteVertexArrays(1, &vao.bsp);
	glDeleteVertexArrays(1, &vao.dynamic);
	glDeleteVertexArrays(1, &vao.fullscreenQuad);
	glDeleteVertexArrays(1, &vao.halfScreenQuad);
	glDeleteVertexArrays(1, &vao.quaterScreenQuad);
	glDeleteVertexArrays(1, &vao.draw2d);
}