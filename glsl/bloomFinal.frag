//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0) uniform sampler2DRect		u_map0; // screen
layout (bindless_sampler, location  = U_TMU1) uniform sampler2DRect		u_map1; // bloom

void main(void) { 
	vec4 src	= texture(u_map0, gl_FragCoord.xy); 
	vec4 dst	= texture(u_map1, gl_FragCoord.xy * 0.5);

	float blend_factor = 0.002;
	fragData = src + (src * blend_factor + dst * (vec4(1.0) - blend_factor));
	fragData.a = 1.0;
}
