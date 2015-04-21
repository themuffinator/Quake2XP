//
// this downsamples the depth buffer
//

#include depth.inc

uniform	sampler2DRect	u_depthBufferMap;
uniform vec2			u_depthParms;

#define OFS		1.0

void main(void) {
	float sum = DecodeDepth(texture2DRect(u_depthBufferMap, gl_FragCoord.xy).x, u_depthParms);

	sum += DecodeDepth(texture2DRect(u_depthBufferMap, gl_FragCoord.xy + vec2(-OFS, -OFS)).x, u_depthParms);
	sum += DecodeDepth(texture2DRect(u_depthBufferMap, gl_FragCoord.xy + vec2( OFS, -OFS)).x, u_depthParms);
	sum += DecodeDepth(texture2DRect(u_depthBufferMap, gl_FragCoord.xy + vec2(-OFS,  OFS)).x, u_depthParms);
	sum += DecodeDepth(texture2DRect(u_depthBufferMap, gl_FragCoord.xy + vec2( OFS,  OFS)).x, u_depthParms);

	gl_FragDepth = EncodeDepth(sum * 0.2, u_depthParms);
}
