//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0) uniform sampler2DRect     u_map; // thermal

void main(void) { 

fragData = texture(u_map, gl_FragCoord.xy * 0.5); 
fragData -= mod(gl_FragCoord.y, 3.0) < 1.0 ? 0.5 : 0.0;
fragData.a = 1.0;
}