/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
#include "r_local.h"

int VA_GetCompileFlags(int compileFlags) {
	if (FBitSet(compileFlags, ATTF_POS))
		return ATT_POSITION;
	if (FBitSet(compileFlags, ATTF_ST0))
		return ATT_TEX0;
	if (FBitSet(compileFlags, ATTF_ST1))
		return ATT_TEX1;
	if (FBitSet(compileFlags, ATTF_ST2))
		return ATT_TEX2;
	if (FBitSet(compileFlags, ATTF_COLOR))
		return ATT_COLOR;
	if (FBitSet(compileFlags, ATTF_TANGENT))
		return ATT_TANGENT;
	if (FBitSet(compileFlags, ATTF_BINORMAL))
		return ATT_BINORMAL;
	if (FBitSet(compileFlags, ATTF_NORMAL))
		return ATT_NORMAL;

	return ATT_POSITION;
}

vertexObject_t *R_Alloc_VAO(const char *name, int flags) {

	vertexObject_t *va;
	int		i;

	if (vao.r_numVertexObject == MAX_VERTEX_OBJECTS)
		VID_Error(ERR_DROP, "R_Alloc_VAO: MAX_VERTEX_OBJECTS hit");

	for (i = 0, va = vao.r_vertexObject; i < vao.r_numVertexObject; i++, va++) {
		if (!va->id)
			break;
	}

	if (i == vao.r_numVertexObject) {
		if (vao.r_numVertexObject == MAX_VERTEX_OBJECTS)
			VID_Error(ERR_DROP, "MAX_VERTEX_OBJECTS");
		vao.r_numVertexObject++;
	}
	strcpy(va->name, name);

	glGenVertexArrays(1, &va->id);
	glBindVertexArray(va->id);
	qglObjectLabel(GL_VERTEX_ARRAY, va->id, strlen(va->name), va->name);
	
	for (i=0; i < 31; i++) {
		int  setBit = FBitSet(flags, BIT(i));
		if (setBit == 0)
			continue;
		qglEnableVertexAttribArray(VA_GetCompileFlags(setBit));
	}
	return va;	
}

int GL_GetVaoBinding(){
	uint  current_vao = 0;
	qglGetIntegerv(GL_VERTEX_ARRAY_BINDING, (int *)&current_vao);
	return current_vao;
}

void GL_BindVAO(vertexObject_t *va) {
	
	if (gl_state.vaoId !=va->id) {
		glBindVertexArray(va->id);
		gl_state.vaoId = va->id;
	}
}

void GL_BindNullVAO(void) {
	glBindVertexArray(0);
	gl_state.vaoId = 0;
}

void R_DeleteVAO(vertexObject_t *in) {

	if (in == NULL)
		return;

	if (gl_state.vaoId = in->id)
		gl_state.vaoId = 0;
			
//	Com_Printf("delete vao %s\n", in->name);

	glDeleteVertexArrays(1, &in->id);
	memset(in, 0, sizeof(*in));
}

void R_VaoListing_f(void) {
	vertexObject_t *va;
	int i;

	Com_Printf(S_COLOR_YELLOW"VAO List:\n");
	for (i = 0, va = vao.r_vertexObject; i < vao.r_numVertexObject; i++, va++) {
		Com_Printf(">" S_COLOR_GREEN "%s\n", va->name );
	}
}

void R_ShotdownVAO(void) {
	int    i;
	vertexObject_t *va;

	for (i = 0, va = vao.r_vertexObject; i < vao.r_numVertexObject; i++, va++) {
		glDeleteVertexArrays(1, &va->id);
		memset(va, 0, sizeof(*va));
	}
}

//--------------------

vertexBuffer_t *R_Alloc_VBO(const char *name, GLuint target, GLuint size, const void *data, GLuint usage) {

	vertexBuffer_t *vb;
	int i;

	if (vbo.r_numVertexBuffers == MAX_VERTEX_OBJECTS)
		VID_Error(ERR_DROP, "R_AllocVertexBuffer: MAX_VERTEX_OBJECTS hit");

	for (i = 0, vb = vbo.r_vertexBuffer; i < vbo.r_numVertexBuffers; i++, vb++) {
		if (!vb->id)
			break;
	}

	if (i == vbo.r_numVertexBuffers) {
		if (vbo.r_numVertexBuffers == MAX_VERTEX_OBJECTS)
			VID_Error(ERR_DROP, "MAX_VERTEX_OBJECTS");
		vbo.r_numVertexBuffers++;
	}
	strcpy(vb->name, name);

	vb->target = target;
	vb->size = size;
	vb->data = data;
	vb->usage = usage;

	qglGenBuffers(1, &vb->id);
	qglBindBuffer(vb->target, vb->id);
	qglBufferData(vb->target, vb->size, vb->data, vb->usage);
	qglObjectLabel(GL_BUFFER, vb->id, strlen(vb->name),vb->name);

	return vb;
}

void GL_BindVBO(vertexBuffer_t *vb) {

	if (vb->target == GL_ARRAY_BUFFER) {
		if (gl_state.vboId != vb->id) {
			qglBindBuffer(GL_ARRAY_BUFFER, vb->id);
			gl_state.vboId = vb->id;
		}
	}
	if (vb->target == GL_ELEMENT_ARRAY_BUFFER) {
//		if (gl_state.iboId != vb->id) {
			qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vb->id);
//			gl_state.iboId = vb->id;
//		}
	}

//	int i = vb->target == GL_ARRAY_BUFFER ? 0 : 1;

//	if (gl_state.vboId[i] != vb->id) {
//		qglBindBuffer(vb->target, vb->id);
//		gl_state.vboId[i] = vb->id;
//	}
}


void R_DeleteVBO(vertexBuffer_t *in) {
	
	if (in == NULL)
		return;
	
	if (in->target == GL_ARRAY_BUFFER) {
		if (gl_state.vboId == in->id)
			gl_state.vboId = 0;
	}
	else {
		if (gl_state.iboId == in->id)
			gl_state.iboId = 0;
	}
//	int i = in->target == GL_ARRAY_BUFFER ? 0 : 1;

//	if (gl_state.vboId[i] != in->id) {
//		gl_state.vboId[i] = in->id;
//	}

//	Com_Printf("delete vbo %s\n", in->name);
			
	qglDeleteBuffers(1, &in->id);
	memset(in, 0, sizeof(*in));
}


char *q_pretifymem(float value) { // based on valve idea :-)

	float   oneKb = 1024.0f;
	float   oneMb = oneKb * oneKb;
	static	int  current;
	static	char  out[24];

	if (value > oneMb) {
		value /= oneMb;
		sprintf(out, va("" S_COLOR_GREEN "%.3f", value));
		strcat(out, "" S_COLOR_WHITE " Mb");
	}
	else if (value > oneKb)	{
		value /= oneKb;
		sprintf(out, va("" S_COLOR_GREEN "%.3f", value));
		strcat(out, "" S_COLOR_WHITE " Kb");
	}
	else {
		sprintf(out, va("" S_COLOR_GREEN "%i", (int)value));
		strcat(out, "" S_COLOR_WHITE " bytes");
	}
	return out;
}

void R_VboListing_f(void) {
	vertexBuffer_t *vb;
	int i, total = 0;
	char S1[24], S2[21];

	Com_Printf(S_COLOR_YELLOW "VBO List:\n");

	for (i = 0, vb = vbo.r_vertexBuffer; i < vbo.r_numVertexBuffers; i++, vb++) {
		
		if(vb->target == GL_ARRAY_BUFFER)
			strcpy(S1, "GL_ARRAY_BUFFER");
		else
			strcpy(S1, "GL_ELEMENT_ARRAY_BUFFER");
		
		if (vb->usage == GL_STREAM_DRAW)
			strcpy(S2, "" S_COLOR_WHITE "|" S_COLOR_MAGENTA "GL_STREAM_DRAW");
		else
			if (vb->usage == GL_STATIC_DRAW)
				strcpy(S2, "" S_COLOR_WHITE "|" S_COLOR_MAGENTA "GL_STATIC_DRAW");
		else
			if (vb->usage == GL_DYNAMIC_DRAW)
			strcpy(S2, "" S_COLOR_WHITE "|" S_COLOR_MAGENTA "GL_DYNAMIC_DRAW");

		Com_Printf(">" S_COLOR_GREEN "%s" S_COLOR_WHITE "|" S_COLOR_YELLOW "%s%s %s\n", vb->name, S1, S2, q_pretifymem((float)vb->size) );
		total += vb->size;
	}
	Com_Printf("\nTotal: " S_COLOR_GREEN "%s of VBO data\n", q_pretifymem((float)total));
}

void R_ShotdownVBO(void) {
	int    i;
	vertexBuffer_t *vb;

	for (i = 0, vb = vbo.r_vertexBuffer; i < vbo.r_numVertexBuffers; i++, vb++) {
		qglDeleteBuffers(1, &vb->id);
		memset(vb, 0, sizeof(*vb));
	}
}