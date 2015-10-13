//
// this quarters the depth buffer
//

#include depth.inc

uniform	sampler2DRect	u_depthBufferMap;
uniform vec2			u_depthParms;

void main (void) {
	vec2 tc = gl_FragCoord.xy * 2.0;

	float sum = DecodeDepth(texture2DRect(u_depthBufferMap, tc).x, u_depthParms);
	sum += DecodeDepth(texture2DRect(u_depthBufferMap, tc + vec2(1.0, 0.0)).x, u_depthParms);
	sum += DecodeDepth(texture2DRect(u_depthBufferMap, tc + vec2(1.0, 1.0)).x, u_depthParms);
	sum += DecodeDepth(texture2DRect(u_depthBufferMap, tc + vec2(0.0, 1.0)).x, u_depthParms);

	// output linear depth
	gl_FragColor = vec4(sum * 0.25);
}
