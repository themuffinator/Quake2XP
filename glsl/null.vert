//!#include "include/global.inc"
layout(location = 0) in vec3	att_position;

layout(location = U_MVP_MATRIX)	uniform mat4	u_modelViewProjectionMatrix;
out vec3 color;

void main (void) {
	color = att_position;
	gl_Position = u_modelViewProjectionMatrix * vec4(att_position, 1.0);
}
