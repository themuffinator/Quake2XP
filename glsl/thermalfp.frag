//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0) uniform sampler2DRect     u_map; // screen

void main(void) { 
 fragData = texture2DRect(u_map, gl_FragCoord.xy * 0.5); 
}