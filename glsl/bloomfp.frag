//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0) uniform sampler2DRect		u_map0; // screen
layout (bindless_sampler, location  = U_TMU1) uniform sampler2DRect		u_map1; // bloom

layout (location = U_PARAM_FLOAT_0)	uniform float	u_bloomParams; //multipler

void main(void) { 

	vec4 screen = texture2DRect(u_map0, gl_FragCoord.xy); 
	vec4 bloom = texture2DRect(u_map1, gl_FragCoord.xy * 0.25);
	screen += bloom * u_bloomParams;
	fragData = screen;
}
