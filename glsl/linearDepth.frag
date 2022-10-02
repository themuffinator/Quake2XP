//!#include "include/global.inc"

#include depth.inc //!#include "include/depth.inc"

layout (bindless_sampler, location = U_TMU0) uniform sampler2DRect	u_depthBufferMap;

layout (location = U_DEPTH_PARAMS)	uniform vec2	u_depthParms;

void main (void) {
	float linearDepth = DecodeDepth(texture(u_depthBufferMap, gl_FragCoord.xy).r, u_depthParms);
	fragData = vec4(linearDepth);
}
