//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0) uniform sampler2DRect u_map;  
layout (location = U_PARAM_INT_0)	uniform int pass;

 void main(void){

 vec4 outp = vec4(0.0, 0.0, 0.0, 1.0);

    if(pass == 0){ // horizontal

    outp  = 0.015625 *	texture(u_map, gl_FragCoord.xy - vec2(6.0,0.0) );
    outp += 0.0596875 * texture(u_map, gl_FragCoord.xy - vec2(5.0,0.0) );
    outp += 0.09375 *	texture(u_map, gl_FragCoord.xy - vec2(4.0,0.0) );
    outp += 0.1640625 * texture(u_map, gl_FragCoord.xy - vec2(3.0,0.0) );
    outp += 0.234375 *	texture(u_map, gl_FragCoord.xy - vec2(2.0,0.0) );
    outp += 0.2734375 * texture(u_map, gl_FragCoord.xy - vec2(1.0,0.0) );
    outp += 0.3125 *	texture(u_map, gl_FragCoord.xy );
    outp += 0.2734375 * texture(u_map, gl_FragCoord.xy + vec2(1.0,0.0) );
    outp += 0.234375 *	texture(u_map, gl_FragCoord.xy + vec2(2.0,0.0) );
    outp += 0.1640625 * texture(u_map, gl_FragCoord.xy + vec2(3.0,0.0) );
    outp += 0.09375 *	texture(u_map, gl_FragCoord.xy + vec2(4.0,0.0) );
    outp += 0.0596875 * texture(u_map, gl_FragCoord.xy + vec2(5.0,0.0) );
    outp += 0.015625 *	texture(u_map, gl_FragCoord.xy + vec2(6.0,0.0) );
 
    fragData = outp * 0.5;
    return;
    }
    
    if(pass == 1){ // vertical

    outp  = 0.015625    *   texture(u_map, gl_FragCoord.xy - vec2(0.0,6.0) );
    outp += 0.0596875   *	texture(u_map, gl_FragCoord.xy - vec2(0.0,5.0) );
    outp += 0.09375     *	texture(u_map, gl_FragCoord.xy - vec2(0.0,4.0) );
    outp += 0.1640625   *	texture(u_map, gl_FragCoord.xy - vec2(0.0,3.0) );
    outp += 0.234375    *	texture(u_map, gl_FragCoord.xy - vec2(0.0,2.0) );
    outp += 0.2734375   *	texture(u_map, gl_FragCoord.xy - vec2(0.0,1.0) );
    outp += 0.3125      *	texture(u_map, gl_FragCoord.xy );
    outp += 0.2734375   *	texture(u_map, gl_FragCoord.xy + vec2(0.0,1.0) );
    outp += 0.234375    *	texture(u_map, gl_FragCoord.xy + vec2(0.0,2.0) );
    outp += 0.1640625   *	texture(u_map, gl_FragCoord.xy + vec2(0.0,3.0) );
    outp += 0.09375     *	texture(u_map, gl_FragCoord.xy + vec2(0.0,4.0) );
    outp += 0.0596875   *	texture(u_map, gl_FragCoord.xy + vec2(0.0,5.0) );
    outp += 0.015625    *	texture(u_map, gl_FragCoord.xy + vec2(0.0,6.0) );
    
	fragData = outp * 0.5;
    return;
    }
} 