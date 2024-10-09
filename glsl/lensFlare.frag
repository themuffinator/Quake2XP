//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0)	uniform sampler2D	u_map0;
layout(location = U_SCREEN_SIZE)				uniform vec2		u_screenSize; 

// based on https://john-chapman-graphics.blogspot.com/2013/02/pseudo-lens-flare.html

	// chromatic distortion:
vec4 textureDistorted(in sampler2D u_map0, in vec2 tc, in vec2 dir, in vec3 distortion){
	return vec4(texture(u_map0, tc + dir * distortion.r).r,
				texture(u_map0, tc + dir * distortion.g).g,
				texture(u_map0, tc + dir * distortion.b).b, 1.0);
}
#define MAX_GHOSTS 8

void main(void){
	vec2 uv = gl_FragCoord.xy / u_screenSize;
	vec2 tc = -uv + vec2(1.0);
	vec2 texelSize = 1.0 / u_screenSize;

	float dispersal = 0.3;
	vec2 ghostVec = (vec2(0.5) - tc) * dispersal;
	float haloWidth = 0.4;
	vec2 haloVec = normalize(ghostVec) * haloWidth;
	
	float strength = 3.0; // was 1
	vec3 distortion = vec3(-texelSize.x * strength, 0.0, texelSize.x * strength);
	
	vec4 outColor = vec4(0.0);
	for (int i = 0; i < MAX_GHOSTS; ++i){
		vec2 offset = fract(tc + ghostVec * float(i));
		float weight = length(vec2(0.5) - offset) / length(vec2(0.5));
		weight = pow(1.0 - weight, 10.0);
		outColor += textureDistorted(u_map0, offset, normalize(ghostVec), distortion) * weight;
	}

	float weight = length(vec2(0.5) - fract(tc + haloVec)) / length(vec2(0.5));
	weight = pow(1.0 - weight, 10.0);
	outColor += textureDistorted(u_map0, fract(tc + haloVec), normalize(ghostVec), distortion) * weight;
	fragData = vec4(outColor.xyz, 1.0);
}