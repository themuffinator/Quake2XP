layout (binding = 0) uniform sampler2D	u_map;
layout (binding = 1) uniform sampler2D	u_normalMap;

layout(location = U_COLOR)			uniform vec4		u_color;
layout(location = U_COLOR_MUL)		uniform float		u_colorScale;
layout(location = U_PARAM_VEC3_0)	uniform vec3		u_lightShift;

layout(location = U_CONSOLE_BACK)	uniform int	    u_console;
layout(location = U_2D_PICS)		uniform int	    u_2dPics;
layout(location = U_FRAG_COLOR)		uniform int     u_fragColor;
layout (location = U_SCREEN_SIZE)	uniform vec2	u_screenSize;

in vec2		v_texCoord;
in vec4		v_color;


#include lighting.inc

#define RGB_MASK_SIZE 3.0
#define SMOOTHING 1.0 / 16.0

vec4 catmullRom(in vec4 A, in vec4 B, in vec4 C, in vec4 D, in float s)
{
	mat4 CatmullRom = mat4(
		vec4(-1.0, 2.0,-1.0, 0.0), 
		vec4( 3.0,-5.0, 0.0, 2.0),
		vec4(-3.0, 4.0, 1.0, 0.0),
		vec4( 1.0,-1.0, 0.0, 0.0));

	vec4 Expo = vec4(s * s * s, s * s, s, 1);

	return 0.5 * Expo * CatmullRom * mat4(
		A[0], B[0], C[0], D[0],
		A[1], B[1], C[1], D[1],
		A[2], B[2], C[2], D[2],
		A[3], B[3], C[3], D[3]);
}

vec4 textureCatmullrom(in sampler2D Sampler, in vec2 Texcoord)
{
	ivec2 TextureSize = textureSize(Sampler, 0) - ivec2(1);
	ivec2 TexelCoord = ivec2(TextureSize * Texcoord);

	vec4 Texel00 = texelFetchOffset(Sampler, TexelCoord, 0, ivec2(-1,-1));
	vec4 Texel10 = texelFetchOffset(Sampler, TexelCoord, 0, ivec2( 0,-1));
	vec4 Texel20 = texelFetchOffset(Sampler, TexelCoord, 0, ivec2( 1,-1));
	vec4 Texel30 = texelFetchOffset(Sampler, TexelCoord, 0, ivec2( 2,-1));

	vec4 Texel01 = texelFetchOffset(Sampler, TexelCoord, 0, ivec2(-1, 0));
	vec4 Texel11 = texelFetchOffset(Sampler, TexelCoord, 0, ivec2( 0, 0));
	vec4 Texel21 = texelFetchOffset(Sampler, TexelCoord, 0, ivec2( 1, 0));
	vec4 Texel31 = texelFetchOffset(Sampler, TexelCoord, 0, ivec2( 2, 0));

	vec4 Texel02 = texelFetchOffset(Sampler, TexelCoord, 0, ivec2(-1, 1));
	vec4 Texel12 = texelFetchOffset(Sampler, TexelCoord, 0, ivec2( 0, 1));
	vec4 Texel22 = texelFetchOffset(Sampler, TexelCoord, 0, ivec2( 1, 1));
	vec4 Texel32 = texelFetchOffset(Sampler, TexelCoord, 0, ivec2( 2, 1));

	vec4 Texel03 = texelFetchOffset(Sampler, TexelCoord, 0, ivec2(-1, 2));
	vec4 Texel13 = texelFetchOffset(Sampler, TexelCoord, 0, ivec2( 0, 2));
	vec4 Texel23 = texelFetchOffset(Sampler, TexelCoord, 0, ivec2( 1, 2));
	vec4 Texel33 = texelFetchOffset(Sampler, TexelCoord, 0, ivec2( 2, 2));

	vec2 SplineCoord = fract(TextureSize * Texcoord);

	vec4 Row0 = catmullRom(Texel00, Texel10, Texel20, Texel30, SplineCoord.x);
	vec4 Row1 = catmullRom(Texel01, Texel11, Texel21, Texel31, SplineCoord.x);
	vec4 Row2 = catmullRom(Texel02, Texel12, Texel22, Texel32, SplineCoord.x);
	vec4 Row3 = catmullRom(Texel03, Texel13, Texel23, Texel33, SplineCoord.x);

	return catmullRom(Row0, Row1, Row2, Row3, SplineCoord.y);
}

void main(void) 
{

vec4 diffuse;

if(u_console == 1){
	
  diffuse = texture(u_map, v_texCoord.xy);
	vec3 normal = normalize(texture(u_normalMap, v_texCoord).rgb * 2.0 - 1.0);
	float specular = texture(u_normalMap, v_texCoord).a;	

	vec3 L = normalize(vec3(u_lightShift.x, u_lightShift.y, u_lightShift.z));
	vec3 V  = normalize(vec3(0.0, 0.0, 1.0));

	vec4 lighting = vec4(Lighting_BRDF(diffuse.rgb, vec3(specular), 0.4, normal.xyz, L, V), 1.0)  * vec4(1.0);

	fragData = diffuse + lighting;
	fragData -= mod(gl_FragCoord.y, 3.0) < 1.0 ? 0.5 : 0.0;
	fragData.a = 1.0;
	return;
}

if(u_2dPics == 1){
  
  diffuse  = textureCatmullrom(u_map, v_texCoord.xy);
  
  // signed distance field
  float distance = diffuse.a;
  float alpha = smoothstep(0.5 - SMOOTHING, 0.5 + SMOOTHING, distance);
	fragData =  vec4(diffuse.rgb * v_color.rgb, diffuse.a * alpha);
	return;
}

if(u_fragColor == 1){
	fragData =  u_color;
	return;
	}
}
