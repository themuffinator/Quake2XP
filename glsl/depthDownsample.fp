//
// this downsamples the depth buffer
//

#include depth.inc

uniform	sampler2DRect	u_depthBufferMap;
uniform vec2			u_depthParms;

#define OFS		1.0

void main(void) {
	
	vec2 ts = gl_FragCoord.xy * 2;
	float sum = DecodeDepth(texture2DRect(u_depthBufferMap, ts).x, u_depthParms);

	sum += DecodeDepth(texture2DRect(u_depthBufferMap, ts + vec2(-OFS, -OFS)).x, u_depthParms);
	sum += DecodeDepth(texture2DRect(u_depthBufferMap, ts + vec2( OFS, -OFS)).x, u_depthParms);
	sum += DecodeDepth(texture2DRect(u_depthBufferMap, ts + vec2(-OFS,  OFS)).x, u_depthParms);
	sum += DecodeDepth(texture2DRect(u_depthBufferMap, ts + vec2( OFS,  OFS)).x, u_depthParms);

	gl_FragDepth = EncodeDepth(sum * 0.2, u_depthParms);
}
