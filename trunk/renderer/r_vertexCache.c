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

	vec2_t		scrnVerts[4];
	int			i, idx = 0;

	Com_Printf("Initializing Vertex Buffers: ");
	
	int8_t	ibo_2pl[]	= { 0, 1 };
	int8_t	ibo_quad[]	= { 0, 1, 2, 0, 2, 3 };
	
	for (i = 0; i < MAX_VERTICES; i += 4) {

		tess2dArray.indices[idx++] = i + 0;
		tess2dArray.indices[idx++] = i + 1;
		tess2dArray.indices[idx++] = i + 2;
		tess2dArray.indices[idx++] = i + 0;
		tess2dArray.indices[idx++] = i + 2;
		tess2dArray.indices[idx++] = i + 3;
	}
	
	VA_SetElem2(scrnVerts[0], 0, vid.height);
	VA_SetElem2(scrnVerts[1], vid.width, vid.height);
	VA_SetElem2(scrnVerts[2], vid.width, 0);
	VA_SetElem2(scrnVerts[3], 0, 0);
	
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

	vbo.stream3d		= R_Alloc_VBO("stream3d_Vbo",			GL_ARRAY_BUFFER,			MAX_VERTICES * sizeof(vertex3d_t), &tess3d, GL_DYNAMIC_DRAW);
	vbo.tess2dVbo		= R_Alloc_VBO("Tess2D_Vbo",				GL_ARRAY_BUFFER,			QUAD_VERTS * sizeof(vertex2d_t), &tess2d, GL_DYNAMIC_DRAW);
	vbo.tess2dArrayVbo	= R_Alloc_VBO("Tess2D_Array_Vbo",		GL_ARRAY_BUFFER,			MAX_VERTICES * sizeof(vertex2d_t), &tess2dArray, GL_DYNAMIC_DRAW);
	vbo.skyBoxVbo		= R_Alloc_VBO("Sky_Box_Vbo",			GL_ARRAY_BUFFER,			CUBE_VERTS * sizeof(vec3_t), v, GL_STATIC_DRAW);
	vbo.fsqVbo			= R_Alloc_VBO("Full_Screen_Quad_Vbo",	GL_ARRAY_BUFFER,			QUAD_VERTS * sizeof(vec2_t), scrnVerts, GL_STATIC_DRAW);
	
	vbo.quadIbo			= R_Alloc_VBO("QuadIbo",				GL_ELEMENT_ARRAY_BUFFER,	sizeof(ibo_quad), ibo_quad, GL_STATIC_DRAW);
	vbo.quadStringIbo	= R_Alloc_VBO("Quad_String_Ibo",		GL_ELEMENT_ARRAY_BUFFER,	sizeof(tess2dArray.indices), tess2dArray.indices, GL_STATIC_DRAW);
	vbo.dynamicIbo		= R_Alloc_VBO("Dynamic_Ibo",			GL_ELEMENT_ARRAY_BUFFER,	MAX_INDICES * sizeof(uint), 0, GL_DYNAMIC_DRAW);
	vbo.cubeIbo			= R_Alloc_VBO("Cube_Ibo",				GL_ELEMENT_ARRAY_BUFFER,	sizeof(cube_idx), cube_idx, GL_STATIC_DRAW);
	vbo.twoPointLineIbo = R_Alloc_VBO("twoPointLineIbo",		GL_ELEMENT_ARRAY_BUFFER,	sizeof(ibo_2pl), ibo_2pl, GL_STATIC_DRAW);

	// Gen VAOs
	vao.sky = R_Alloc_VAO("skyVao", ATTF_POS);
	GL_BindVBO(vbo.skyBoxVbo);
	GL_BindVBO(vbo.cubeIbo);
	qglVertexAttribPointer(ATT_POSITION,	3, GL_FLOAT, false, 0, 0);

	vao.tess2d = R_Alloc_VAO("tess2dVao", ATTF_POS | ATTF_ST0 | ATTF_COLOR);
	GL_BindVBO(vbo.tess2dVbo);
	GL_BindVBO(vbo.quadIbo);
	qglVertexAttribPointer(ATT_POSITION,	4, GL_FLOAT, false, sizeof(vertex2d_t), TESS_OFFSET_POS);
	qglVertexAttribPointer(ATT_TEX0,		2, GL_FLOAT, false, sizeof(vertex2d_t), TESS_OFFSET_TC);
	qglVertexAttribPointer(ATT_COLOR,		4, GL_FLOAT, false, sizeof(vertex2d_t), TESS_OFFSET_COLOR);

	vao.tess2dArray = R_Alloc_VAO("tess2dArrayVao", ATTF_POS | ATTF_ST0 | ATTF_COLOR);
	GL_BindVBO(vbo.tess2dArrayVbo);
	GL_BindVBO(vbo.quadStringIbo);
	qglVertexAttribPointer(ATT_POSITION,	4, GL_FLOAT, false, sizeof(vertex2d_t), TESS_OFFSET_POS);
	qglVertexAttribPointer(ATT_TEX0,		2, GL_FLOAT, false, sizeof(vertex2d_t), TESS_OFFSET_TC);
	qglVertexAttribPointer(ATT_COLOR,		4, GL_FLOAT, false, sizeof(vertex2d_t), TESS_OFFSET_COLOR);

	vao.stream3d = R_Alloc_VAO("stream3dVao", ATTF_POS | ATTF_ST0 | ATTF_COLOR | ATTF_TANGENT | ATTF_BINORMAL | ATTF_NORMAL);
	GL_BindVBO(vbo.stream3d);
//	GL_BindVBO(vbo.dynamicIbo);
	qglVertexAttribPointer(ATT_POSITION,	4, GL_FLOAT, false, sizeof(vertex3d_t), TESS_OFFSET_POS);
	qglVertexAttribPointer(ATT_TEX0,		2, GL_FLOAT, false, sizeof(vertex3d_t), TESS_OFFSET_TC);
	qglVertexAttribPointer(ATT_COLOR,		4, GL_FLOAT, false, sizeof(vertex3d_t), TESS_OFFSET_COLOR);
	qglVertexAttribPointer(ATT_TANGENT,		3, GL_FLOAT, false, sizeof(vertex3d_t), TESS_OFFSET_TANHENT);
	qglVertexAttribPointer(ATT_BINORMAL,	3, GL_FLOAT, false, sizeof(vertex3d_t), TESS_OFFSET_BINORMAL);
	qglVertexAttribPointer(ATT_NORMAL,		3, GL_FLOAT, false, sizeof(vertex3d_t), TESS_OFFSET_NORMAL);

	vao.fsq = R_Alloc_VAO("fsqVao", ATTF_POS);
	GL_BindVBO(vbo.fsqVbo);
	GL_BindVBO(vbo.quadIbo);
	qglVertexAttribPointer(ATT_POSITION,	2, GL_FLOAT, false, 0, 0);
	
	GL_BindNullVAO();

	Com_Printf(S_COLOR_GREEN"ok\n\n");	
}

void R_ShutDownVertexBuffers() {

	R_ShotdownVAO();
	R_ShotdownVBO();
}