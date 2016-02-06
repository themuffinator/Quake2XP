/*
===========================================
Radial Blur effect, uses Crysis(tm) shader.
===========================================
*/

// xy = radial center screen space position, z = radius attenuation, w = blur strength
uniform vec4			u_radialBlurParams;	
layout (binding = 0) uniform sampler2DRect	u_screenMap;
uniform int				u_cont;

void main (void) {
	vec2 screenPos = u_radialBlurParams.xy;
	vec2 blurVec = screenPos.xy - gl_FragCoord.xy;
	vec4 color;

	if(u_cont == 32)
		color = vec4(0.3, 0.3, 0.5, 1.0);

	if(u_cont == 16)
		color = vec4(0.3, 0.85, 0.3, 1.0);
	
	if(u_cont == 8)
		color = vec4(1.0, 0.3, 0.3, 1.0);

	float invRadius = u_radialBlurParams.z;
	vec2 dir = blurVec.xy * invRadius;
	float blurDist = clamp(1.0 - dot(dir, dir), 0.0, 1.0);
	float strength = u_radialBlurParams.w * blurDist * blurDist;
	float weight = 0.125;
  
	vec4 accum = vec4(0.0);
	vec2 add = blurVec.xy * strength;
	vec2 st = gl_FragCoord.xy;
	accum += texture2DRect(u_screenMap, st);
	st += add;
	accum += texture2DRect(u_screenMap, st);
	st += add;
	accum += texture2DRect(u_screenMap, st);
	st += add;
	accum += texture2DRect(u_screenMap, st);
	st += add;
	accum += texture2DRect(u_screenMap, st);
	st += add;
	accum += texture2DRect(u_screenMap, st);
	st += add;
	accum += texture2DRect(u_screenMap, st);
	st += add;
	accum += texture2DRect(u_screenMap, st);
	
	float opaque;
		
		if( u_cont == 8)
			opaque = 0.666;
		else
			opaque = 0.2;

	if( u_cont == 8 || u_cont == 16 || u_cont == 32)
		fragData = mix(accum * weight, color, opaque);
	else
		fragData = accum * weight;
}
