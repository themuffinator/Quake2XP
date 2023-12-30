
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

	if (r_numVertexObject == MAX_VERTEX_OBJECTS)
		VID_Error(ERR_DROP, "R_Alloc_VAO: MAX_VERTEX_OBJECTS hit");

	for (i = 0, va = r_vertexObject; i < r_numVertexObject; i++, va++) {
		if (!va->id)
			break;
	}

	if (i == r_numVertexObject) {
		if (r_numVertexObject == MAX_VERTEX_OBJECTS)
			VID_Error(ERR_DROP, "MAX_VERTEX_OBJECTS");
		r_numVertexObject++;
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

void GL_BindVao(vertexObject_t *va) {
	int	id = va->id;

//	if (gl_state.vaoId != id) { // fix me
		glBindVertexArray(id);
//		gl_state.vaoId = id;
//		return;
//	}
}

void GL_BindNullVao(void) {

//	if (gl_state.vaoId) {
		glBindVertexArray(0);
//		gl_state.programId = -1;
//	}
}

void R_DeleteVao(char *name) {
	int    i;
	vertexObject_t *va;

	for (i = 0, va = r_vertexObject; i < r_numVertexObject; i++, va++) {
		
		if (!b_stricmp(name, va->name)) {
		
			if (gl_state.vaoId = va->id)
				gl_state.vaoId = 0;
		
			glDeleteVertexArrays(1, &va->id);
			memset(va, 0, sizeof(*va));
			return;
		}
	}
}

void R_VaoListing_f(void) {
	vertexObject_t *va;
	int i;

	Com_Printf(S_COLOR_YELLOW"VAO List:\n");
	for (i = 0, va = r_vertexObject; i < r_numVertexObject; i++, va++) {
		Com_Printf("*" S_COLOR_GREEN "%s\n", va->name );
	}
}