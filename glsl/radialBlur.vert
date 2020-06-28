//!#include "include/global.inc"
layout(location = 0) in vec3		att_position;

layout(location = U_ORTHO_MATRIX) uniform mat4	u_orthoMatrix;

void main (void) 
{
	gl_Position = u_orthoMatrix * vec4(att_position, 1.0);
}
