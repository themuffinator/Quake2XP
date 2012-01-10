/*
===========================================
Radial Blur effect, uses Crysis(tm) shader.
===========================================
*/

// xy = radial center screen space position, z = radius attenuation, w = blur strength
uniform vec4		u_radialBlurParams;	
uniform sampler2DRect	u_screenMap;

void main (void) {
	vec2 screenPos = u_radialBlurParams.xy;
	vec2 blurVec = screenPos.xy - gl_FragCoord.xy;

	float invRadius = u_radialBlurParams.z;
	vec2 dir = blurVec.xy * invRadius;
	float blurDist = clamp(1.0 - dot(dir, dir), 0.0, 1.0);
	float strength = u_radialBlurParams.w * blurDist * blurDist;

	float weight = 0.125;
  
	vec4 accum = vec4(0.0);   
	accum += texture2DRect(u_screenMap, gl_FragCoord.xy + blurVec.xy * 0.0 * strength);
	accum += texture2DRect(u_screenMap, gl_FragCoord.xy + blurVec.xy * 1.0 * strength);
	accum += texture2DRect(u_screenMap, gl_FragCoord.xy + blurVec.xy * 2.0 * strength);
	accum += texture2DRect(u_screenMap, gl_FragCoord.xy + blurVec.xy * 3.0 * strength);
	accum += texture2DRect(u_screenMap, gl_FragCoord.xy + blurVec.xy * 4.0 * strength);
	accum += texture2DRect(u_screenMap, gl_FragCoord.xy + blurVec.xy * 5.0 * strength);
	accum += texture2DRect(u_screenMap, gl_FragCoord.xy + blurVec.xy * 6.0 * strength);
	accum += texture2DRect(u_screenMap, gl_FragCoord.xy + blurVec.xy * 7.0 * strength);

	gl_FragColor = accum * weight;
}
