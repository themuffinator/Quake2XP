#extension GL_ARB_texture_rectangle : enable

#include depth.inc

varying vec2			v_texCoord;
uniform vec2			u_screenSize;
uniform	sampler2DRect	u_colorMiniMap;

uniform	sampler2DRect	u_depthMiniMap;
uniform vec2			u_depthParms;

void main(void) {
	vec2 centerTC = gl_FragCoord.xy;

	// low weight center sample, will be used on edges
	float sum = 0.0125;
	vec4 color = texture2DRect(u_colorMiniMap, centerTC) * sum;

	float centerDepth = DecodeDepth(texture2DRect(u_depthMiniMap, centerTC).r, u_depthParms);

	vec2 arrOffsets[4] = {
		vec2( 1.0,-1.0),
		vec2(-1.0,-1.0),
		vec2( 1.0, 1.0),
		vec2(-1.0, 1.0)
	};

	for (int i = 0; i < 4; i++) {
		vec2 sampleTC = centerTC + arrOffsets[i];

		float depth = DecodeDepth(texture2DRect(u_depthMiniMap, sampleTC).r, u_depthParms);

		float diff = 8.0 * (1.0 - depth / centerDepth);
		float w = max(0.5 - 0.75 * abs(diff) - 0.25 * diff, 0.0);

		color += texture2DRect(u_colorMiniMap, sampleTC) * w;
		sum += w;
	}

	gl_FragColor = color / sum;
}
