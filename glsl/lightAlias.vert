//!#include "include/global.inc"
layout(location = 0) in vec3 att_position;
layout(location = 1) in vec3 att_normal;
layout(location = 2) in vec3 att_tangent;
layout(location = 3) in vec3 att_binormal;
layout(location = 5) in vec2 att_texCoordDiffuse;

layout(location = U_LIGHT_POS)		uniform vec3	u_LightOrg;
layout(location = U_VIEW_POS)		uniform vec3	u_ViewOrigin; 		
layout(location = U_ATTEN_MATRIX)	uniform mat4	u_attenMatrix;
layout(location = U_SPOT_MATRIX)	uniform mat4	u_spotMatrix;
layout(location = U_CUBE_MATRIX)	uniform mat4	u_cubeMatrix;
layout(location = U_MVP_MATRIX)		uniform mat4	u_modelViewProjectionMatrix;
layout(location = U_MODELVIEW_MATRIX)	uniform mat4	u_modelViewMatrix; 
layout(location = U_TEXTURE0_MATRIX)	uniform mat4	u_skyMatrix; 
layout(location = U_TEXTURE1_MATRIX)	uniform mat3	u_entAxis;

out vec2		v_texCoord;
out vec4		v_CubeCoord;
out vec4		v_AttenCoord;
out vec3		v_viewVec;
out vec3		v_lightVec;
out vec3		v_lightAtten;
out vec3		v_lightSpot;
out vec3		v_tangent;
out mat3		v_tangentToView;
out vec3		v_tst;
out vec3		v_positionVS;
out mat4		v_mvMatrix;

void main (void) {
	
v_texCoord			= att_texCoordDiffuse; 
v_CubeCoord			= u_cubeMatrix		* vec4(att_position, 1.0);
v_lightAtten		= (u_attenMatrix	* vec4(att_position, 1.0)).xyz;
v_lightSpot			= (u_spotMatrix		* vec4(att_position, 1.0)).xyz;
v_tangent       =  att_tangent;
vec3 LV = u_LightOrg - att_position;
v_lightVec.x = dot(LV, att_tangent);
v_lightVec.y = dot(LV, att_binormal);
v_lightVec.z = dot(LV, att_normal); 

vec3 VV = u_ViewOrigin - att_position;
v_viewVec.x = dot(VV, att_tangent);
v_viewVec.y = dot(VV, att_binormal);
v_viewVec.z = dot(VV, att_normal); 

mat3 m = mat3(u_modelViewProjectionMatrix);

v_mvMatrix = u_modelViewMatrix;

// calculate tangent to view space transform
v_tangentToView[0] = m * att_tangent;
v_tangentToView[1] = m * att_binormal;
v_tangentToView[2] = m * att_normal;

gl_Position = u_modelViewProjectionMatrix * vec4(att_position, 1.0);

v_tst = mat3(u_skyMatrix) * att_normal;

vec4 pos = u_modelViewProjectionMatrix * vec4(att_position, 1.0);
v_positionVS = pos.xyz;
}
