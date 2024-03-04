//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0) uniform sampler2DRect		u_map0; // screen
layout (bindless_sampler, location  = U_TMU1) uniform sampler2DRect		u_map1; // bloom

layout (location = U_PARAM_FLOAT_0)	uniform float u_intens;
layout (location = U_PARAM_FLOAT_1)	uniform float u_scale;

void main(void) { 

	vec4 src = texture(u_map0, gl_FragCoord.xy); 
	vec4 dst = texture(u_map1, gl_FragCoord.xy * u_scale);
	fragData = src + dst * u_intens;
	fragData.a = 1.0;
}
