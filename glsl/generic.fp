layout (binding = 0) uniform sampler2D	u_map;
layout (binding = 1) uniform sampler2D	u_normalMap;

layout(location = U_COLOR)			uniform vec4		u_color;
layout(location = U_COLOR_MUL)		uniform float		u_colorScale;
layout(location = U_PARAM_VEC3_0)	uniform vec3		u_lightShift;

layout(location = U_CONSOLE_BACK)	uniform int	u_console;
layout(location = U_2D_PICS)		uniform int	u_2dPics;
layout(location = U_FRAG_COLOR)		uniform int	u_fragColor;


in vec2		v_texCoord;
in vec4		v_color;


#include lighting.inc

void main(void) 
{

vec4 diffuse = texture(u_map, v_texCoord.xy);

if(u_console == 1){
	
	vec3 normal = normalize(texture(u_normalMap, v_texCoord).rgb * 2.0 - 1.0);
	float specular = texture(u_normalMap, v_texCoord).a;	

	vec3 L = normalize(vec3(u_lightShift.x, u_lightShift.y, u_lightShift.z));
	vec3 V  = normalize(vec3(0.0, 0.0, 1.0));

	vec4 lighting = vec4(Lighting_BRDF(diffuse.rgb, vec3(specular), 0.4, normal.xyz, L, V), 1.0)  * vec4(1.0);

	fragData = diffuse * 0.25 + lighting;
	
	fragData -= mod(gl_FragCoord.y, 3.0) < 1.0 ? 0.5 : 0.0; // add scanline
	fragData.a = 1.0;
	return;
}

if(u_2dPics == 1){
	fragData =  vec4(diffuse.rgb * v_color.rgb, diffuse.a);
	return;
}

if(u_fragColor == 1){
	fragData =  u_color;
	return;
	}
}
