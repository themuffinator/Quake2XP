//!#include "include/global.inc"
layout(location = 0) in vec3	att_position;
layout(location = 1) in vec3	att_normal;
layout(location = 2) in vec3	att_tangent;
layout(location = 3) in vec3	att_binormal;
layout(location = 5) in vec2	att_texCoordDiffuse;
layout(location = 6) in vec2	att_texCoordLightMap;

layout (location = U_VIEW_POS)		uniform vec3		u_viewOrg;
layout (location = U_SCROLL)		uniform float		u_scroll;
layout (location = U_PARALLAX_TYPE)	uniform int			u_parallaxType;
layout (location = U_MVP_MATRIX)	uniform mat4		u_modelViewProjectionMatrix;
layout (location = U_MODELVIEW_MATRIX) uniform mat4		u_mvm;

out vec3		v_viewVecTS;
out vec2		v_wTexCoord;
out vec2		v_lTexCoord;
out vec3		v_bumpVecs[3];

//
// 3-vector radiosity basis for normal mapping
//

const vec3 s_basisVecs[3] = vec3[](
vec3 ( 0.81649658092772603273242802490196f,		0.f,									0.57735026918962576450914878050195f ),
vec3 ( -0.40824829046386301636621401245098f,	0.70710678118654752440084436210485f,	0.57735026918962576450914878050195f ),
vec3 ( -0.40824829046386301636621401245098f,	-0.70710678118654752440084436210485f,	0.57735026918962576450914878050195f )
);  

void main (void) {
	// setup tex coords
	v_wTexCoord = att_texCoordDiffuse;  // diffuse, additive, caustics
	v_wTexCoord.x += u_scroll;

	v_lTexCoord = att_texCoordLightMap; // lightmap only

	// calculate tangent space view vector for parallax
	vec3 tmp = u_viewOrg - att_position;
	
	v_viewVecTS.x = dot(tmp, att_tangent);
	v_viewVecTS.y = dot(tmp, att_binormal);
	v_viewVecTS.z = dot(tmp, att_normal);

	v_bumpVecs[0] = s_basisVecs[0];
	v_bumpVecs[1] = s_basisVecs[1];
	v_bumpVecs[2] = s_basisVecs[2];
	
	gl_Position = u_modelViewProjectionMatrix * vec4(att_position, 1.0);
}
