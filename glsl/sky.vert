//!#include "include/global.inc"
layout(location = 0)					in vec3			att_position;
layout(location = U_MVP_MATRIX)			uniform mat4	u_modelViewProjectionMatrix;
layout(location = U_TEXTURE0_MATRIX)	uniform mat4	u_rotMatrix;

out vec3	v_texCoord; 

void main (void) {

	vec4 tmp = u_rotMatrix * vec4( att_position, 1.0 );
	v_texCoord = vec3(tmp);

	gl_Position = u_modelViewProjectionMatrix * vec4( att_position, 1.0 );
}
