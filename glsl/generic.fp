in vec2		v_texCoord;
in vec2		v_texCoord1;
in vec4		v_colorArray;

layout (binding = 0) uniform sampler2D	u_map;
layout (binding = 1) uniform sampler2D	u_normalMap;

uniform vec4		u_color;
uniform float		u_colorScale;
uniform vec3		u_lightShift;

uniform int			u_ATTRIB_COLORS, u_ATTRIB_CONSOLE;

#include lighting.inc

void main(void) 
{
vec4 diffuse = texture(u_map, v_texCoord.xy);

if(u_ATTRIB_CONSOLE == 1){
	
	vec3 normal = normalize(texture(u_normalMap, v_texCoord).rgb * 2.0 - 1.0);
	float specular = texture(u_normalMap, v_texCoord).a;	

	vec3 L = normalize(vec3(u_lightShift.x, u_lightShift.y, u_lightShift.z));
	vec3 V  = normalize(vec3(0.0, 0.0, 1.0));

	vec4 lighting = vec4(Lighting_BRDF(diffuse.rgb, vec3(specular), 0.4, normal.xyz, L, V), 1.0)  * vec4(1.0);

	fragData = /*vec4(0.0, 0.0, 0.0, 1.0)*/ diffuse * 0.25 + lighting;
	
	fragData -= mod(gl_FragCoord.y, 3.0) < 1.0 ? 0.5 : 0.0; // add scanline
	fragData.a = 1.0;

/*	
	float OuterVignetting	= 1.4 - 0.35;
	float InnerVignetting	= 1.0 - 0.35;

	vec2 uv = gl_FragCoord.xy / vec2(1920.0, 1080.0);
	float d = distance(vec2(0.5, 0.5), uv) * 1.414213;
	float vignetting = clamp((OuterVignetting - d) / (OuterVignetting - InnerVignetting), 0.0, 1.0);
	fragData *= vignetting;
*/

	return;
}

if(u_ATTRIB_COLORS == 1){
	fragData =  vec4(diffuse.rgb * v_colorArray.rgb * u_colorScale, diffuse.a);
	return;
}

if(u_ATTRIB_COLORS != 1 && u_ATTRIB_CONSOLE != 1){
	fragData =  u_color;
	return;
	}
}
