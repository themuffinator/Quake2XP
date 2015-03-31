//
// this downsamples the depth buffer
//

#include depth.inc

uniform	sampler2DRect	g_depthBufferMap;
uniform vec2		g_depthParms;

#define OFS		1.0

void main(void) {
	float sum = DecodeDepth(texture2DRect(g_depthBufferMap, gl_FragCoord.xy).x);

	sum += DecodeDepth(texture2DRect(g_depthBufferMap, gl_FragCoord.xy + vec2(-OFS, -OFS)).x);
	sum += DecodeDepth(texture2DRect(g_depthBufferMap, gl_FragCoord.xy + vec2( OFS, -OFS)).x);
	sum += DecodeDepth(texture2DRect(g_depthBufferMap, gl_FragCoord.xy + vec2(-OFS,  OFS)).x);
	sum += DecodeDepth(texture2DRect(g_depthBufferMap, gl_FragCoord.xy + vec2( OFS,  OFS)).x);

	gl_FragDepth = EncodeDepth(sum * 0.2);
/*
	float sum = texture2DRect(g_depthBufferMap, gl_FragCoord.xy).x;

	sum += texture2DRect(g_depthBufferMap, gl_FragCoord.xy + vec2(-OFS, -OFS)).x;
	sum += texture2DRect(g_depthBufferMap, gl_FragCoord.xy + vec2( OFS, -OFS)).x;
	sum += texture2DRect(g_depthBufferMap, gl_FragCoord.xy + vec2(-OFS,  OFS)).x;
	sum += texture2DRect(g_depthBufferMap, gl_FragCoord.xy + vec2( OFS,  OFS)).x;
	gl_FragDepth = sum * 0.2;
*/
}
