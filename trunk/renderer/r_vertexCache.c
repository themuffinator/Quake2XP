/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
#include "r_local.h"
 
ushort cube_idx[] = {
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

	for (i = 0; i < MAX_VERTICES_2D; i += 4){

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
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_draw2d);
	qglObjectLabel(GL_VERTEX_ARRAY, vbo.vbo_draw2d, strlen("***vbo_draw2d***"), "***vbo_draw2d***");
	qglBufferData(GL_ARRAY_BUFFER, sizeof(tess2d), &tess2d, GL_DYNAMIC_DRAW);

	qglGenBuffers(1, &vbo.vbo_draw2dArray);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_draw2dArray);
	qglObjectLabel(GL_VERTEX_ARRAY, vbo.vbo_draw2dArray, strlen("***vbo_draw2dArray***"), "***vbo_draw2dArray***");
	qglBufferData(GL_ARRAY_BUFFER, sizeof(tess2dArray), &tess2dArray, GL_DYNAMIC_DRAW);

	VA_SetElem2(tmpVerts[0], 0, vid.height);
	VA_SetElem2(tmpVerts[1], vid.width, vid.height);
	VA_SetElem2(tmpVerts[2], vid.width, 0);
	VA_SetElem2(tmpVerts[3], 0, 0);
	
	qglGenBuffers(1, &vbo.vbo_fullScreenQuad);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_fullScreenQuad);
	qglObjectLabel(GL_VERTEX_ARRAY, vbo.vbo_fullScreenQuad, strlen("***vboFSQ***"), "***vboFSQ***");
	qglBufferData(GL_ARRAY_BUFFER, sizeof(vec2_t) * 4, tmpVerts, GL_STATIC_DRAW);

	qglGenBuffers(1, &vbo.vbo_aliasShadow);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_aliasShadow);
	qglObjectLabel(GL_VERTEX_ARRAY, vbo.vbo_aliasShadow, strlen("***vboMd2shadow***"), "***vboMd2shadow***");
	qglBufferData(GL_ARRAY_BUFFER, MAX_STREAM_VBO_VERTS * sizeof(vec4_t), 0, GL_DYNAMIC_DRAW);

	qglGenBuffers(1, &vbo.ibo_md2Shadow);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_md2Shadow);
	qglObjectLabel(GL_VERTEX_ARRAY, vbo.ibo_md2Shadow, strlen("***iboMd2shadow***"), "***iboMd2shadow***");
	qglBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_STREAM_IBO_IDX * sizeof(uint), 0, GL_DYNAMIC_DRAW);

	for (i = 0; i < MD3_MAX_VERTS * MD3_MAX_MESHES; i++)
		ibo_md3Shadow[i] = i;

	qglGenBuffers(1, &vbo.ibo_md3Shadow);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_md3Shadow);
	qglObjectLabel(GL_VERTEX_ARRAY, vbo.ibo_md3Shadow, strlen("***iboMd3shadow***"), "***iboMd3shadow***");
	qglBufferData(GL_ELEMENT_ARRAY_BUFFER, i * sizeof(uint), ibo_md3Shadow, GL_DYNAMIC_DRAW);

	qglGenBuffers(1, &vbo.vbo_dynamic);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_dynamic);
	qglObjectLabel(GL_VERTEX_ARRAY, vbo.vbo_dynamic, strlen("***vboDinamic***"), "***vboDinamic***");
	qglBufferData(GL_ARRAY_BUFFER, sizeof(tess_t), 0, GL_DYNAMIC_DRAW);

	qglGenBuffers(1, &vbo.ibo_dynamic);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_dynamic);
	qglObjectLabel(GL_VERTEX_ARRAY, vbo.ibo_dynamic, strlen("***iboDinamic***"), "***iboDinamic***");
	qglBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_STREAM_IBO_IDX * sizeof(uint), 0, GL_DYNAMIC_DRAW);

	
//------------------------------
	qglGenBuffers(1, &vbo.ibo_cube);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_cube);
	qglBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_idx), cube_idx, GL_STATIC_DRAW);
	qglObjectLabel(GL_BUFFER, vbo.ibo_cube, strlen("***ibo_cube***"), "***ibo_cube***");

	vec3_t v[8];
	float size = 17000.0;
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

	qglBindBuffer(GL_ARRAY_BUFFER, 0);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

// Gen VAOs
	skyVao = R_Alloc_VAO("skyVao", ATTF_POS);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_skyBox);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_cube);
	qglVertexAttribPointer(ATT_POSITION, 3, GL_FLOAT, qfalse, 0, 0);

	tess2dVao = R_Alloc_VAO("tess2dVao", ATTF_POS | ATTF_ST0 | ATTF_COLOR);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_draw2d);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quad);

	qglVertexAttribPointer(ATT_POSITION,	2, GL_FLOAT, qfalse, sizeof(vertex2d_t), VERT2D_POS);
	qglVertexAttribPointer(ATT_TEX0,		2, GL_FLOAT, qfalse, sizeof(vertex2d_t), VERT2D_TC);
	qglVertexAttribPointer(ATT_COLOR,		4, GL_FLOAT, qfalse, sizeof(vertex2d_t), VERT2D_COLOR);

	tess2dArrayVao = R_Alloc_VAO("tess2dArrayVao", ATTF_POS | ATTF_ST0 | ATTF_COLOR);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_draw2dArray);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadString);

	qglVertexAttribPointer(ATT_POSITION,	2, GL_FLOAT, qfalse, sizeof(vertex2d_t), VERT2D_POS);
	qglVertexAttribPointer(ATT_TEX0,		2, GL_FLOAT, qfalse, sizeof(vertex2d_t), VERT2D_TC);
	qglVertexAttribPointer(ATT_COLOR,		4, GL_FLOAT, qfalse, sizeof(vertex2d_t), VERT2D_COLOR);

	fsqVao = R_Alloc_VAO("fsqVao", ATTF_POS);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_fullScreenQuad);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quad);

	qglVertexAttribPointer(ATT_POSITION, 2, GL_FLOAT, qfalse, 0, 0);

	md2shadowVao = R_Alloc_VAO("md2shadowVao", ATTF_POS);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_aliasShadow);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_md2Shadow);

	qglVertexAttribPointer(ATT_POSITION, 4, GL_FLOAT, qfalse, 0, 0);

	md3shadowVao = R_Alloc_VAO("md3shadowVao", ATTF_POS);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_aliasShadow);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_md3Shadow);

	qglVertexAttribPointer(ATT_POSITION, 4, GL_FLOAT, qfalse, 0, 0);

	dynamicVao = R_Alloc_VAO("dynamicVao", ATTF_POS);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_dynamic);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_dynamic);

	qglVertexAttribPointer(ATT_POSITION, 4, GL_FLOAT, qfalse, 0, 0);

	tessStreamVao = R_Alloc_VAO("tessStreamVao", ATTF_POS | ATTF_ST0 | ATTF_COLOR);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_dynamic);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_dynamic);

	qglVertexAttribPointer(ATT_POSITION,	4, GL_FLOAT, qfalse, 0, ((tess_t *)0)->position);
	qglVertexAttribPointer(ATT_TEX0,		2, GL_FLOAT, qfalse, 0, ((tess_t *)0)->texCoord);
	qglVertexAttribPointer(ATT_COLOR,		4, GL_FLOAT, qfalse, 0, ((tess_t *)0)->color);

	md2Vao = R_Alloc_VAO("md2Vao", ATTF_POS | ATTF_ST0 | ATTF_COLOR | ATTF_TANGENT | ATTF_BINORMAL | ATTF_NORMAL);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_dynamic);

	qglVertexAttribPointer(ATT_POSITION,	4, GL_FLOAT, qfalse, 0, ((tess_t *)0)->position);
	qglVertexAttribPointer(ATT_TEX0,		2, GL_FLOAT, qfalse, 0, ((tess_t *)0)->texCoord);
	qglVertexAttribPointer(ATT_COLOR,		4, GL_FLOAT, qfalse, 0, ((tess_t *)0)->color);
	qglVertexAttribPointer(ATT_TANGENT,		3, GL_FLOAT, qfalse, 0, ((tess_t *)0)->tangent);
	qglVertexAttribPointer(ATT_BINORMAL,	3, GL_FLOAT, qfalse, 0, ((tess_t *)0)->binormal);
	qglVertexAttribPointer(ATT_NORMAL,		3, GL_FLOAT, qfalse, 0, ((tess_t *)0)->normal);
	
	GL_BindNullVao();
	Com_Printf(S_COLOR_GREEN"ok\n\n");	
}

void R_ShutDownVertexBuffers() {

	int    i;
	vertexObject_t *va;

	for (i = 0, va = r_vertexObject; i < r_numVertexObject; i++, va++) {
		glDeleteVertexArrays(1, &va->id);
		memset(va, 0, sizeof(*va));
	}

	qglDeleteBuffers(1, &vbo.vbo_fullScreenQuad);
	qglDeleteBuffers(1, &vbo.vbo_aliasShadow);
	qglDeleteBuffers(1, &vbo.vbo_BSP);
	qglDeleteBuffers(1, &vbo.vbo_draw2d);

	qglDeleteBuffers(1, &vbo.ibo_quad);
	qglDeleteBuffers(1, &vbo.ibo_md2Shadow);
	qglDeleteBuffers(1, &vbo.ibo_md3Shadow);
	qglDeleteBuffers(1, &vbo.ibo_dynamic);
	qglDeleteBuffers(1, &vbo.ibo_cube);
	qglDeleteBuffers(1, &vbo.ibo_quadString);
}