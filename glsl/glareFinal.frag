//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0) uniform sampler2DRect		u_map0; // screen
layout (bindless_sampler, location  = U_TMU1) uniform sampler2D		    u_glare;

layout (location = U_PARAM_FLOAT_0)	uniform float u_intens;
layout(location = U_SCREEN_SIZE)		uniform vec2	u_screenSize;
void main(void) { 

	vec4 screen = texture(u_map0, gl_FragCoord.xy); 
  vec2 st =  gl_FragCoord.xy / u_screenSize;
	vec4 glare = texture(u_glare, st * 0.25);
	screen += glare * u_intens;
	fragData = screen;
}
