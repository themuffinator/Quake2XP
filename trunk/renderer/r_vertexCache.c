/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
#include "r_local.h"
 
int8_t cube_idx[] = {
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
uint	ibo_md3Shadow[MD3_MAX_MODEL_VERTICES];

void R_InitVertexBuffers() {

	vec2_t		tmpVerts[4];
	int			i, idx = 0;

	Com_Printf("Initializing Vertex Buffers: ");
	
	int8_t	ibo_2pl[]	= { 0, 1 };
	int8_t	ibo_quad[]	= { 0, 1, 2, 0, 2, 3 };
	
	for (i = 0; i < MAX_VERTICES_2D; i += 4) {

		ibo_quadString[idx++] = i + 0;
		ibo_quadString[idx++] = i + 1;
		ibo_quadString[idx++] = i + 2;
		ibo_quadString[idx++] = i + 0;
		ibo_quadString[idx++] = i + 2;
		ibo_quadString[idx++] = i + 3;
	}
	
	VA_SetElem2(tmpVerts[0], 0, vid.height);
	VA_SetElem2(tmpVerts[1], vid.width, vid.height);
	VA_SetElem2(tmpVerts[2], vid.width, 0);
	VA_SetElem2(tmpVerts[3], 0, 0);
	
	for (i = 0; i < MD3_MAX_MODEL_VERTICES; i++)
		ibo_md3Shadow[i] = i;

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

	vbo.md2AliasVbo		= R_Alloc_VBO("Md2Alias_Vbo",			GL_ARRAY_BUFFER,			sizeof(tesselator_t), &tess2, GL_DYNAMIC_DRAW);

	vbo.tess2dVbo		= R_Alloc_VBO("Tess2D_Vbo",				GL_ARRAY_BUFFER,			sizeof(tess2d_t), &tess2d, GL_DYNAMIC_DRAW);
	vbo.tess2dArrayVbo	= R_Alloc_VBO("Tess2D_Array_Vbo",		GL_ARRAY_BUFFER,			sizeof(tess2dArray_t), &tess2dArray, GL_DYNAMIC_DRAW);
	vbo.fsqVbo			= R_Alloc_VBO("Full_Screen_Quad_Vbo",	GL_ARRAY_BUFFER,			sizeof(vec2_t) * 4, tmpVerts, GL_STATIC_DRAW);
	vbo.md3ShadowVbo	= R_Alloc_VBO("md3_Shadow_Vbo",			GL_ARRAY_BUFFER,			MD3_MAX_MODEL_VERTICES * sizeof(vec4_t), 0, GL_DYNAMIC_DRAW);
	vbo.md2ShadowVbo	= R_Alloc_VBO("md2_Shadow_Vbo",			GL_ARRAY_BUFFER,			MD3_MAX_TRIANGLES * sizeof(vec4_t), 0, GL_DYNAMIC_DRAW);
	vbo.dynamicVbo		= R_Alloc_VBO("Dynamic_Vbo",			GL_ARRAY_BUFFER,			sizeof(tess_t), 0, GL_DYNAMIC_DRAW);
	vbo.skyBoxVbo		= R_Alloc_VBO("Sky_Box_Vbo",			GL_ARRAY_BUFFER,			sizeof(vec3_t) * 8, v, GL_STATIC_DRAW);
	
	vbo.quadIbo			= R_Alloc_VBO("QuadIbo",				GL_ELEMENT_ARRAY_BUFFER,	sizeof(ibo_quad), ibo_quad, GL_STATIC_DRAW);
	vbo.quadStringIbo	= R_Alloc_VBO("Quad_String_Ibo",		GL_ELEMENT_ARRAY_BUFFER,	sizeof(ibo_quadString), ibo_quadString, GL_STATIC_DRAW);
	vbo.md2ShadowIbo	= R_Alloc_VBO("Md2_Shadow_Ibo",			GL_ELEMENT_ARRAY_BUFFER,	MD3_MAX_TRIANGLES * 3 * sizeof(int16_t), 0, GL_DYNAMIC_DRAW);
	vbo.md3ShadowIbo	= R_Alloc_VBO("Md3_Shadow_Ibo",			GL_ELEMENT_ARRAY_BUFFER,	MD3_MAX_MODEL_VERTICES * sizeof(uint), ibo_md3Shadow, GL_DYNAMIC_DRAW);
	vbo.dynamicIbo		= R_Alloc_VBO("Dynamic_Ibo",			GL_ELEMENT_ARRAY_BUFFER,	MD3_MAX_INDICES * sizeof(uint), 0, GL_DYNAMIC_DRAW);
	vbo.cubeIbo			= R_Alloc_VBO("Cube_Ibo",				GL_ELEMENT_ARRAY_BUFFER,	sizeof(cube_idx), cube_idx, GL_STATIC_DRAW);
	vbo.twoPointLineIbo = R_Alloc_VBO("twoPointLineIbo",		GL_ELEMENT_ARRAY_BUFFER,	sizeof(ibo_2pl), ibo_2pl, GL_STATIC_DRAW);

	// Gen VAOs
	vao.sky = R_Alloc_VAO("skyVao", ATTF_POS);
	GL_BindVBO(vbo.skyBoxVbo);
	GL_BindVBO(vbo.cubeIbo);
	qglVertexAttribPointer(ATT_POSITION,	3, GL_FLOAT, qfalse, 0, 0);

	vao.tess2d = R_Alloc_VAO("tess2dVao", ATTF_POS | ATTF_ST0 | ATTF_COLOR);
	GL_BindVBO(vbo.tess2dVbo);
	GL_BindVBO(vbo.quadIbo);
	qglVertexAttribPointer(ATT_POSITION,	2, GL_FLOAT, qfalse, sizeof(vertex2d_t), VERT2D_POS);
	qglVertexAttribPointer(ATT_TEX0,		2, GL_FLOAT, qfalse, sizeof(vertex2d_t), VERT2D_TC);
	qglVertexAttribPointer(ATT_COLOR,		4, GL_FLOAT, qfalse, sizeof(vertex2d_t), VERT2D_COLOR);

	vao.tess2dArray = R_Alloc_VAO("tess2dArrayVao", ATTF_POS | ATTF_ST0 | ATTF_COLOR);
	GL_BindVBO(vbo.tess2dArrayVbo);
	GL_BindVBO(vbo.quadStringIbo);
	qglVertexAttribPointer(ATT_POSITION,	2, GL_FLOAT, qfalse, sizeof(vertex2d_t), VERT2D_POS);
	qglVertexAttribPointer(ATT_TEX0,		2, GL_FLOAT, qfalse, sizeof(vertex2d_t), VERT2D_TC);
	qglVertexAttribPointer(ATT_COLOR,		4, GL_FLOAT, qfalse, sizeof(vertex2d_t), VERT2D_COLOR);

	vao.fsq = R_Alloc_VAO("fsqVao", ATTF_POS);
	GL_BindVBO(vbo.fsqVbo);
	GL_BindVBO(vbo.quadIbo);
	qglVertexAttribPointer(ATT_POSITION,	2, GL_FLOAT, qfalse, 0, 0);

	vao.md2shadow = R_Alloc_VAO("md2shadowVao", ATTF_POS);
	GL_BindVBO(vbo.md2ShadowVbo);
	GL_BindVBO(vbo.md2ShadowIbo);
	qglVertexAttribPointer(ATT_POSITION,	4, GL_FLOAT, qfalse, 0, 0);

	vao.md3shadow = R_Alloc_VAO("md3shadowVao", ATTF_POS);
	GL_BindVBO(vbo.md3ShadowVbo);
	GL_BindVBO(vbo.md3ShadowIbo);
	qglVertexAttribPointer(ATT_POSITION,	4, GL_FLOAT, qfalse, 0, 0);

	vao.dynamic = R_Alloc_VAO("dynamicVao", ATTF_POS);
	GL_BindVBO(vbo.dynamicVbo);
	GL_BindVBO(vbo.dynamicIbo);
	qglVertexAttribPointer(ATT_POSITION,	4, GL_FLOAT, qfalse, 0, 0);

	vao.tessStream = R_Alloc_VAO("tessStreamVao", ATTF_POS | ATTF_ST0 | ATTF_COLOR);
	GL_BindVBO(vbo.dynamicVbo);
	GL_BindVBO(vbo.dynamicIbo);
	qglVertexAttribPointer(ATT_POSITION,	4, GL_FLOAT, qfalse, 0, ((tess_t *)0)->position);
	qglVertexAttribPointer(ATT_TEX0,		2, GL_FLOAT, qfalse, 0, ((tess_t *)0)->texCoord);
	qglVertexAttribPointer(ATT_COLOR,		4, GL_FLOAT, qfalse, 0, ((tess_t *)0)->color);

	vao.tessStreamVaoQuad = R_Alloc_VAO("tessStreamVaoQuad", ATTF_POS | ATTF_ST0 | ATTF_COLOR);
	GL_BindVBO(vbo.dynamicVbo);
	GL_BindVBO(vbo.quadIbo);
	qglVertexAttribPointer(ATT_POSITION,	4, GL_FLOAT, qfalse, 0, ((tess_t *)0)->position);
	qglVertexAttribPointer(ATT_TEX0,		2, GL_FLOAT, qfalse, 0, ((tess_t *)0)->texCoord);
	qglVertexAttribPointer(ATT_COLOR,		4, GL_FLOAT, qfalse, 0, ((tess_t *)0)->color);

	vao.md2 = R_Alloc_VAO("md2Vao", ATTF_POS | ATTF_ST0 | ATTF_COLOR | ATTF_TANGENT | ATTF_BINORMAL | ATTF_NORMAL);
	GL_BindVBO(vbo.dynamicVbo);

	qglVertexAttribPointer(ATT_POSITION,	4, GL_FLOAT, qfalse, 0, ((tess_t *)0)->position);
	qglVertexAttribPointer(ATT_TEX0,		2, GL_FLOAT, qfalse, 0, ((tess_t *)0)->texCoord);
	qglVertexAttribPointer(ATT_COLOR,		4, GL_FLOAT, qfalse, 0, ((tess_t *)0)->color);
	qglVertexAttribPointer(ATT_TANGENT,		3, GL_FLOAT, qfalse, 0, ((tess_t *)0)->tangent);
	qglVertexAttribPointer(ATT_BINORMAL,	3, GL_FLOAT, qfalse, 0, ((tess_t *)0)->binormal);
	qglVertexAttribPointer(ATT_NORMAL,		3, GL_FLOAT, qfalse, 0, ((tess_t *)0)->normal);

	vao.md3 = R_Alloc_VAO("md3Vao", ATTF_POS | ATTF_ST0 | ATTF_COLOR | ATTF_TANGENT | ATTF_BINORMAL | ATTF_NORMAL);
	GL_BindVBO(vbo.dynamicVbo);
	GL_BindVBO(vbo.dynamicIbo);
	qglVertexAttribPointer(ATT_POSITION,	4, GL_FLOAT, qfalse, 0, ((tess_t *)0)->position);
	qglVertexAttribPointer(ATT_TEX0,		2, GL_FLOAT, qfalse, 0, ((tess_t *)0)->texCoord);
	qglVertexAttribPointer(ATT_COLOR,		4, GL_FLOAT, qfalse, 0, ((tess_t *)0)->color);
	qglVertexAttribPointer(ATT_TANGENT,		3, GL_FLOAT, qfalse, 0, ((tess_t *)0)->tangent);
	qglVertexAttribPointer(ATT_BINORMAL,	3, GL_FLOAT, qfalse, 0, ((tess_t *)0)->binormal);
	qglVertexAttribPointer(ATT_NORMAL,		3, GL_FLOAT, qfalse, 0, ((tess_t *)0)->normal);

	vao.dynamicCube_verts = R_Alloc_VAO("dynamicCube_verts", ATTF_POS );
	GL_BindVBO(vbo.dynamicVbo);
	GL_BindVBO(vbo.cubeIbo);
	qglVertexAttribPointer(ATT_POSITION, 4, GL_FLOAT, qfalse, 0, ((tess_t *)0)->position);

	vao.drawLine = R_Alloc_VAO("drawLine", ATTF_POS);
	GL_BindVBO(vbo.dynamicVbo);
	GL_BindVBO(vbo.twoPointLineIbo);
	qglVertexAttribPointer(ATT_POSITION, 4, GL_FLOAT, qfalse, 0, ((tess_t *)0)->position);
	
	GL_BindNullVAO();

	Com_Printf(S_COLOR_GREEN"ok\n\n");	
}

void R_ShutDownVertexBuffers() {

	R_ShotdownVAO();
	R_ShotdownVBO();
}