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

void R_InitVertexBuffers() {

	vec2_t		tmpVerts[4];
	int			i, idx = 0;

	Com_Printf("Initializing Vertex Buffers: ");

	// 2d drawing

	index_t	ibo_quad[] = { 0, 1, 2, 0, 2, 3 };
	qglGenBuffers(1, &vbo.ibo_quad);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quad);
	qglBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ibo_quad), ibo_quad, GL_STATIC_DRAW);
	qglObjectLabel(GL_BUFFER, vbo.ibo_quad, strlen("***ibo_quad***"), "***ibo_quad***");

	for (i = 0; i < MAX_DRAW_STRING_LENGTH * 4; i += 4)
	{
		ibo_quadString[idx++] = i + 0;
		ibo_quadString[idx++] = i + 1;
		ibo_quadString[idx++] = i + 2;
		ibo_quadString[idx++] = i + 0;
		ibo_quadString[idx++] = i + 2;
		ibo_quadString[idx++] = i + 3;
	}
	qglGenBuffers(1, &vbo.ibo_quadString);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadString);
	qglBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ibo_quadString), ibo_quadString, GL_STATIC_DRAW);
	qglObjectLabel(GL_BUFFER, vbo.ibo_quadString, strlen("***ibo_quadString***"), "***ibo_quadString***");

	qglGenBuffers(1, &vbo.vbo_draw2d);
	glGenVertexArrays(1, &vao.draw2d);

	glBindVertexArray(vao.draw2d);
	qglObjectLabel(GL_VERTEX_ARRAY, vao.draw2d, strlen("***vao2d***"), "***vao2d***");
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_draw2d);
	qglBufferData(GL_ARRAY_BUFFER, sizeof(tess2d), &tess2d, GL_STREAM_DRAW);

	qglEnableVertexAttribArray	(ATT_POSITION);
	qglEnableVertexAttribArray	(ATT_TEX0);
	qglEnableVertexAttribArray	(ATT_COLOR);
	qglVertexAttribPointer		(ATT_POSITION,	2, GL_FLOAT, qfalse, sizeof(vertex2d_t), VERT2D_POS);
	qglVertexAttribPointer		(ATT_TEX0,		2, GL_FLOAT, qfalse, sizeof(vertex2d_t), VERT2D_TC);
	qglVertexAttribPointer		(ATT_COLOR,		4, GL_FLOAT, qfalse, sizeof(vertex2d_t), VERT2D_COLOR);

	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quad);

	glBindVertexArray(0);
//-------------

	qglGenBuffers(1, &vbo.vbo_draw2dString);
	glGenVertexArrays(1, &vao.draw2dString);

	glBindVertexArray(vao.draw2dString);
	qglObjectLabel(GL_VERTEX_ARRAY, vao.draw2dString, strlen("***vao2dString***"), "***vao2dString***");
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_draw2dString);
	qglBufferData(GL_ARRAY_BUFFER, sizeof(tess2dString), &tess2dString, GL_STREAM_DRAW);

	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TEX0);
	qglEnableVertexAttribArray(ATT_COLOR);
	qglVertexAttribPointer(ATT_POSITION, 2, GL_FLOAT, qfalse, sizeof(vertex2d_t), VERT2D_POS);
	qglVertexAttribPointer(ATT_TEX0, 2, GL_FLOAT, qfalse, sizeof(vertex2d_t), VERT2D_TC);
	qglVertexAttribPointer(ATT_COLOR, 4, GL_FLOAT, qfalse, sizeof(vertex2d_t), VERT2D_COLOR);

	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadString);

	glBindVertexArray(0);

//-------------------------

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
	qglObjectLabel(GL_VERTEX_ARRAY, vao.fullscreenQuad, strlen("***vaoFSQ***"), "***vaoFSQ***");
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_fullScreenQuad);
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
	qglObjectLabel(GL_VERTEX_ARRAY, vao.md2Shadow, strlen("***vaoMd2shadow***"), "***vaoMd2shadow***");
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_aliasShadow);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_md2Shadow);
	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 4, GL_FLOAT, qfalse, 0, 0);

	glBindVertexArray(0);
//---------------------
	glGenVertexArrays(1, &vao.md3Shadow);
	glBindVertexArray(vao.md3Shadow);
	qglObjectLabel(GL_VERTEX_ARRAY, vao.md3Shadow, strlen("***vaoMd3shadow***"), "***vaoMd3shadow***");
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_aliasShadow);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_md3Shadow);
	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 4, GL_FLOAT, qfalse, 0, 0);

	glBindVertexArray(0);

//---------------------------------------------------

	qglGenBuffers(1, &vbo.vbo_dynamic);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_dynamic);
	qglObjectLabel(GL_VERTEX_ARRAY, vbo.vbo_dynamic, strlen("***vboDinamic***"), "***vboDinamic***");
	qglBufferData(GL_ARRAY_BUFFER, MAX_STREAM_VBO_VERTS * sizeof(vec3_t), 0, GL_STREAM_DRAW);

	qglGenBuffers(1, &vbo.ibo_dynamic);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_dynamic);
	qglObjectLabel(GL_VERTEX_ARRAY, vbo.ibo_dynamic, strlen("***iboDinamic***"), "***iboDinamic***");
	qglBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_STREAM_IBO_IDX * sizeof(uint), 0, GL_STREAM_DRAW);

	glGenVertexArrays(1, &vao.dynamic);
	glBindVertexArray(vao.dynamic);
	qglObjectLabel(GL_VERTEX_ARRAY, vao.dynamic, strlen("***vaoDinamic***"), "***vaoDinamic***");
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_dynamic);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_dynamic);
	
	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, 0);

//-------------
	glGenVertexArrays(1, &vao.tessStream);
	glBindVertexArray(vao.tessStream);
	qglObjectLabel(GL_VERTEX_ARRAY, vao.tessStream, strlen("***vaoTessStream***"), "***vaoTessStream***");
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_dynamic);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_dynamic);

	qglEnableVertexAttribArray(ATT_POSITION);
	qglEnableVertexAttribArray(ATT_TEX0);
	qglEnableVertexAttribArray(ATT_COLOR);
	qglEnableVertexAttribArray(ATT_TANGENT);
	qglEnableVertexAttribArray(ATT_BINORMAL);
	qglEnableVertexAttribArray(ATT_NORMAL);

	qglVertexAttribPointer(ATT_POSITION,	3, GL_FLOAT, qfalse, 0, ((tess_t *)0)->xyz);
	qglVertexAttribPointer(ATT_TEX0,		2, GL_FLOAT, qfalse, 0, ((tess_t *)0)->st);
	qglVertexAttribPointer(ATT_COLOR,		4, GL_FLOAT, qfalse, 0, ((tess_t *)0)->rgb);
	qglVertexAttribPointer(ATT_TANGENT,		3, GL_FLOAT, qfalse, 0, ((tess_t *)0)->tangent);
	qglVertexAttribPointer(ATT_BINORMAL,	3, GL_FLOAT, qfalse, 0, ((tess_t *)0)->binormal);
	qglVertexAttribPointer(ATT_NORMAL,		3, GL_FLOAT, qfalse, 0, ((tess_t *)0)->normal);

	glBindVertexArray(0);

//------------------------------
	qglGenBuffers(1, &vbo.ibo_cube);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_cube);
	qglBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_idx), cube_idx, GL_STATIC_DRAW);
	qglObjectLabel(GL_BUFFER, vbo.ibo_cube, strlen("***ibo_cube***"), "***ibo_cube***");

	qglBindBuffer(GL_ARRAY_BUFFER, 0);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

//------------------------------------------- skybox
	vec3_t v[8];
	float size = 4096.0;
	vec3_t org = { 0.0, 0.0, 0.0 };
	VectorSet(v[0], org[0] - size, org[1] - size, org[2] + size);
	VectorSet(v[1], org[0] + size, org[1] - size, org[2] + size);
	VectorSet(v[2], org[0] + size, org[1] + size, org[2] + size);
	VectorSet(v[3], org[0] - size, org[1] + size, org[2] + size);

	VectorSet(v[4], org[0] - size, org[1] - size, org[2] - size);
	VectorSet(v[5], org[0] + size, org[1] - size, org[2] - size);
	VectorSet(v[6], org[0] + size, org[1] + size, org[2] - size);
	VectorSet(v[7], org[0] - size, org[1] + size, org[2] - size);

	qglGenBuffers(1, &vbo.vbo_skyBox);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_skyBox);
	qglBufferData(GL_ARRAY_BUFFER, sizeof(vec3_t) * 8, v, GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao.sky);
	glBindVertexArray(vao.sky);
	qglObjectLabel(GL_VERTEX_ARRAY, vao.sky, strlen("***vaoSky***"), "***vaoSky***");
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_skyBox);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_cube);

	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, 0);

	glBindVertexArray(0);

	Com_Printf(S_COLOR_GREEN"ok\n\n");	
}

void R_ShutDownVertexBuffers() {

	qglDeleteBuffers(1, &vbo.vbo_fullScreenQuad);
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
	glDeleteVertexArrays(1, &vao.draw2d);
}