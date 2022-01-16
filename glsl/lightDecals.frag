//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0) uniform sampler2D		u_map;
layout (bindless_sampler, location  = U_TMU1) uniform samplerCube	u_CubeFilterMap;

layout(location = U_COLOR)			uniform vec4 	u_LightColor;
layout(location = U_SPOT_LIGHT)		uniform int		u_spotLight;
layout(location = U_SPOT_PARAMS)	uniform vec3	u_spotParams;
layout(location = U_PARAM_iVEC2_0)	uniform ivec2	u_blendParams;

in vec2	v_texCoord; 
in vec4	v_color;

in vec4	v_CubeCoord;
in vec3	v_lightAtten;
in vec3	v_lightSpot;

#include lighting.inc   //!#include "include/lighting.inc"

void main(void) 
{
	float attenMap = PointAttenuation(v_lightAtten, 2.0);

	if(u_spotLight == 1)
		attenMap *= ConeAttenuation(v_lightSpot, u_spotParams);
	
	if(attenMap <= CUTOFF_EPSILON){
		discard;
		return;
	}
	vec3 col = u_LightColor.rgb;
	if(u_blendParams.x == 0 && u_blendParams.y ==  769) //src 0, dst 0x0301
		col = vec3(1.0);

	vec4 color = texture(u_map, v_texCoord.xy) * v_color;
	vec4 cubeFilter = texture(u_CubeFilterMap, v_CubeCoord.xyz) * 2.0;
	fragData.rgb = color.rgb * col * attenMap * cubeFilter.rgb;
	fragData.a = color.a;
	
}