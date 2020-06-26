//!#include "include/global.inc"
/*
===========================================
Radial Blur effect, uses Crysis(tm) shader.
===========================================
*/

// xy = radial center screen space position, z = radius attenuation, w = blur strength
layout (location = U_PARAM_VEC4_0)	uniform vec4			u_radialBlurParams;	
layout (bindless_sampler, location = U_TMU0)	uniform sampler2DRect	u_r_screenTex;

void main (void) {
	vec2 screenPos = u_radialBlurParams.xy;
	vec2 blurVec = screenPos.xy - gl_FragCoord.xy;

	float invRadius = u_radialBlurParams.z;
	vec2 dir = blurVec.xy * invRadius;
	float blurDist = clamp(1.0 - dot(dir, dir), 0.0, 1.0);
	float strength = u_radialBlurParams.w * blurDist * blurDist;
	float weight = 0.125;
  
	vec4 accum = vec4(0.0);
	vec2 add = blurVec.xy * strength;
	vec2 st = gl_FragCoord.xy;
	accum += texture2DRect(u_r_screenTex, st);
	st += add;
	accum += texture2DRect(u_r_screenTex, st);
	st += add;
	accum += texture2DRect(u_r_screenTex, st);
	st += add;
	accum += texture2DRect(u_r_screenTex, st);
	st += add;
	accum += texture2DRect(u_r_screenTex, st);
	st += add;
	accum += texture2DRect(u_r_screenTex, st);
	st += add;
	accum += texture2DRect(u_r_screenTex, st);
	st += add;
	accum += texture2DRect(u_r_screenTex, st);
	
	fragData = accum * weight;
}
