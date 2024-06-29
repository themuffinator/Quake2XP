//!#include "include/global.inc"
layout(location = 0) in vec4 att_position;
layout(location = 4) in vec4 att_color;
layout(location = 5) in vec2 att_texCoord;

layout(location = U_ORTHO_MATRIX) uniform mat4	u_orthoMatrix;

out vec2		v_texCoord;
out vec4		v_color;

void main (void) 
{
	v_texCoord	= att_texCoord;
	v_color		= att_color;

	gl_Position = u_orthoMatrix * vec4(att_position.xyz, 1.0);
}

