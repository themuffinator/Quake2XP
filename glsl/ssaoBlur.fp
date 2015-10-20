#include depth.inc

uniform	sampler2DRect	u_colorMiniMap;
uniform	sampler2DRect		u_DNMiniMap;

#if 1

// two-pass blur

uniform vec2			u_axisMask;
uniform int			u_numSamples;		// to each side, without central one

void main (void) {
	vec2 centerTC = gl_FragCoord.xy;
	float centerDepth = texture2DRect(u_DNMiniMap, centerTC).x;
	float sum = float(u_numSamples) + 1.0;
	vec4 color = texture2DRect(u_colorMiniMap, centerTC) * sum;

	for (int i = 1; i <= u_numSamples; i++) {
		float f = float(i);
		float scale = 1.0 + float(u_numSamples) - f;

		vec2 sampleTC0 = centerTC + u_axisMask * f;
		vec2 sampleTC1 = centerTC - u_axisMask * f;

		float depth0 = texture2DRect(u_DNMiniMap, sampleTC0).x;
		float depth1 = texture2DRect(u_DNMiniMap, sampleTC1).x;

		float diff0 = 8.0 * (1.0 - depth0 / centerDepth);
		float diff1 = 8.0 * (1.0 - depth1 / centerDepth);

		float w0 = max(0.5 - 0.75 * abs(diff0) - 0.25 * diff0, 0.0) * scale;
		float w1 = max(0.5 - 0.75 * abs(diff1) - 0.25 * diff1, 0.0) * scale;

		color += texture2DRect(u_colorMiniMap, sampleTC0) * w0;
		color += texture2DRect(u_colorMiniMap, sampleTC1) * w1;

		sum += w0 + w1;
	}

	fragData = color / sum;
}

#else

// single-pass blur

void main (void) {
	// low weight center sample, will be used on edges
	float sum = 0.0125;
	vec4 color = texture2DRect(u_colorMiniMap, gl_FragCoord.xy) * sum;

	float centerDepth = texture2DRect(u_DNMiniMap, gl_FragCoord.xy).x;

	vec2 arrOffsets[4] = {
		vec2( 1.0,-1.0),
		vec2(-1.0,-1.0),
		vec2( 1.0, 1.0),
		vec2(-1.0, 1.0)
	};

	for (int i = 0; i < 4; i++) {
		vec2 sampleTC = gl_FragCoord.xy + arrOffsets[i];

		float depth = texture2DRect(u_DNMiniMap, sampleTC).x;
		float diff = 8.0 * (1.0 - depth / centerDepth);
		float w = max(0.5 - 0.75 * abs(diff) - 0.25 * diff, 0.0);

		color += texture2DRect(u_colorMiniMap, sampleTC) * w;
		sum += w;
	}

	fragData = color / sum;
}

#endif