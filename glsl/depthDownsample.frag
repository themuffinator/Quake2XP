//!#include "include/global.inc"
layout (bindless_sampler, location = U_TMU0) uniform sampler2DRect	u_depthBufferMap;

void main (void) {
	vec2 tc = gl_FragCoord.xy * 2.0;

	float sum = texture(u_depthBufferMap, tc).x;
	sum += texture(u_depthBufferMap, tc + vec2(1.0, 0.0)).x;
	sum += texture(u_depthBufferMap, tc + vec2(1.0, 1.0)).x;
	sum += texture(u_depthBufferMap, tc + vec2(0.0, 1.0)).x;

	// output linear depth
	fragData = vec4(sum * 0.25);
}
