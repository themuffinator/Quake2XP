//!#include "include/global.inc"
layout(location = 0) in vec3	att_position;

layout(location = U_MVP_MATRIX)	uniform mat4	u_modelViewProjectionMatrix;
out vec3 pos;

void main (void) {
	vec4 position = u_modelViewProjectionMatrix * vec4(att_position, 1.0);
	gl_Position = position;
	pos = position.xyz;
}
