//!#include "include/global.inc"
#include depth.inc //!#include "include/depth.inc"

layout (bindless_sampler, location = U_TMU0) uniform	sampler2DRect	u_colorMiniMap;
layout (bindless_sampler, location = U_TMU1) uniform	sampler2DRect	u_DNMiniMap;


layout(location = U_PARAM_VEC2_0)	uniform vec2	u_axisMask;
layout(location = U_PARAM_INT_0)	uniform int		u_numSamples;		// to each side, without central one

// two-pass blur

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