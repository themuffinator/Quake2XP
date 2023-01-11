//!#include "include/global.inc"
layout(location = 0) in vec3 att_position;
layout(location = 4) in vec4 att_color4f;
layout(location = 5) in vec4 att_texCoordDiffuse;

layout(location = U_MVP_MATRIX)			uniform mat4	u_modelViewProjectionMatrix; 
layout(location = U_MODELVIEW_MATRIX)	uniform mat4	u_modelViewMatrix; 
layout(location = U_PROJ_MATRIX)		uniform mat4	u_projectionMatrix; 
layout(location = U_TEXTURE0_MATRIX)	uniform mat4	u_texRotateMatrix;

out vec4	v_texCoord0;
out float	v_depthS;
out float	v_depth;
out	vec4	v_color;
out vec2	v_deformMul;

void main (void) {
	v_texCoord0 = u_texRotateMatrix * att_texCoordDiffuse;
	v_color = att_color4f;
	// soft depth
	v_depthS = -(u_modelViewMatrix * vec4(att_position, 1.0)).z;

	//-------------------------------------------------------------
	// compute view space depth
	vec4 positionVS = u_modelViewMatrix * vec4(att_position, 1.0);
	positionVS = vec4(1.0, 0.0, positionVS.z, 1.0);
	v_depth = -positionVS.z;

	// compute the deform strength
	vec2 d = (u_projectionMatrix * positionVS).xw;
	d.x /= max(d.y, 1.0);
	v_deformMul = min(d.xx, 0.02); // mul by deform mul

	gl_Position = u_modelViewProjectionMatrix * vec4(att_position, 1.0);
}
