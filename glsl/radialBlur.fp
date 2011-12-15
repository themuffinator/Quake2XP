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
	accum += texture2DRect(u_screenMap, gl_FragCoord.xy + blurVec.xy * float(0) * strength);
	accum += texture2DRect(u_screenMap, gl_FragCoord.xy + blurVec.xy * float(1) * strength);
	accum += texture2DRect(u_screenMap, gl_FragCoord.xy + blurVec.xy * float(2) * strength);
	accum += texture2DRect(u_screenMap, gl_FragCoord.xy + blurVec.xy * float(3) * strength);
	accum += texture2DRect(u_screenMap, gl_FragCoord.xy + blurVec.xy * float(4) * strength);
	accum += texture2DRect(u_screenMap, gl_FragCoord.xy + blurVec.xy * float(5) * strength);
	accum += texture2DRect(u_screenMap, gl_FragCoord.xy + blurVec.xy * float(6) * strength);
	accum += texture2DRect(u_screenMap, gl_FragCoord.xy + blurVec.xy * float(7) * strength);

	gl_FragColor = accum * weight;
}
