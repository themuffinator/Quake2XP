/*
===========================================
Radial Blur effect, uses Crysis(tm) shader.
===========================================
*/

// xy = radial center screen space position, z = radius attenuation, w = blur strength
uniform vec4		u_radialBlurParams;	
uniform sampler2DRect	u_screenMap;
uniform int u_samples;

void main (void) {
	vec2 screenPos = u_radialBlurParams.xy;
	vec2 blurVec = screenPos.xy - gl_FragCoord.xy;

	float invRadius = u_radialBlurParams.z;
	vec2 dir = blurVec.xy * invRadius;
	float blurDist = clamp(1.0 - dot(dir, dir), 0.0, 1.0);
	float strength = u_radialBlurParams.w * blurDist * blurDist;

	float weight = 1.0 / float(u_samples);
  
	vec4 accum = vec4(0.0);   

	for (int i = 0; i < u_samples; i++)
		accum += texture2DRect(u_screenMap, gl_FragCoord.xy + blurVec.xy * float(i) * strength);

	gl_FragColor = accum * weight;
}
