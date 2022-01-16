//!#include "include/global.inc"
layout(location = 0) in vec3	att_position;
layout(location = 1) in vec3	att_normal;
layout(location = 4) in vec4	att_color4f;
layout(location = 5) in vec2	att_texCoordDiffuse;

layout(location = U_MVP_MATRIX)		uniform mat4	u_modelViewProjectionMatrix;
layout(location = U_ATTEN_MATRIX)	uniform mat4	u_attenMatrix;
layout(location = U_SPOT_MATRIX)	uniform mat4	u_spotMatrix;
layout(location = U_CUBE_MATRIX)	uniform mat4	u_cubeMatrix;

out vec2		v_texCoord; 
out	vec4		v_color;

out vec4		v_CubeCoord;
out vec4		v_AttenCoord;
out vec3		v_lightAtten;
out vec3		v_lightSpot;

void main (void) {
	v_texCoord		= att_texCoordDiffuse;	
	v_color			= att_color4f;

	v_CubeCoord		= u_cubeMatrix * vec4(att_position, 1.0);
	v_lightAtten	= (u_attenMatrix * vec4(att_position, 1.0)).xyz;
	v_lightSpot		= (u_spotMatrix * vec4(att_position, 1.0)).xyz;

	gl_Position		= u_modelViewProjectionMatrix * vec4(att_position, 1.0);
}
