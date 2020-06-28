//!#include "include/global.inc"
layout(location = 0) in vec3 att_position;
layout(location = 1) in vec3 att_normal;
layout(location = 2) in vec3 att_tangent;
layout(location = 3) in vec3 att_binormal;
layout(location = 5) in vec2 att_texCoordDiffuse;

layout(location = U_ATTEN_MATRIX)	uniform mat4	u_attenMatrix;
layout(location = U_SPOT_MATRIX)	uniform mat4	u_spotMatrix;
layout(location = U_CUBE_MATRIX)	uniform mat4	u_cubeMatrix;
layout(location = U_MVP_MATRIX)		uniform mat4	u_modelViewProjectionMatrix;
layout(location = U_SCROLL)			uniform float	u_scroll;
layout(location = U_VIEW_POS)		uniform vec3	u_viewOriginES;
layout(location = U_LIGHT_POS)		uniform vec3 	u_LightOrg;

out vec3		v_viewVecTS;
out vec3 		v_lightVec;
out vec2		v_texCoord; 
out vec4		v_CubeCoord;
out vec4		v_AttenCoord;
out vec3		v_positionVS;
out vec3		v_lightAtten;
out vec3		v_lightSpot;

void main (void) {
	// setup tex coords
	v_texCoord		= att_texCoordDiffuse;  // diffuse & bump
	v_texCoord		+= u_scroll;
	v_CubeCoord		= u_cubeMatrix * vec4(att_position, 1.0);
	v_lightAtten	= (u_attenMatrix * vec4(att_position, 1.0)).xyz;
	v_lightSpot		= (u_spotMatrix * vec4(att_position, 1.0)).xyz;

	// calculate tangent space view vector for parallax
	vec3 VV = u_viewOriginES - att_position;
	v_viewVecTS.x = dot(VV, att_tangent);
	v_viewVecTS.y = dot(VV, att_binormal);
	v_viewVecTS.z = dot(VV, att_normal);

	vec3 LV = u_LightOrg - att_position;
	v_lightVec.x = dot(LV, att_tangent);
	v_lightVec.y = dot(LV, att_binormal);
	v_lightVec.z = dot(LV, att_normal); 

	gl_Position = u_modelViewProjectionMatrix * vec4(att_position, 1.0);
}
