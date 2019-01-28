layout(location = 0) in vec3	att_position;
layout(location = 5) in vec2	att_texCoordDiffuse;

layout(location = U_MVP_MATRIX)	uniform mat4	u_modelViewProjectionMatrix;

out vec2		v_texCoord; 

void main (void) {
	v_texCoord		= att_texCoordDiffuse;	
	gl_Position		= u_modelViewProjectionMatrix * vec4(att_position, 1.0);
}
