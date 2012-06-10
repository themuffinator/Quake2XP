uniform sampler2DRect u_map;
 
 void main(void)
 {
    vec4 	
    outp  = 0.015625 *	texture2DRect(u_map, gl_FragCoord.xy + vec2(-6.0,0.0) );
    outp += 0.0596875 * texture2DRect(u_map, gl_FragCoord.xy + vec2(-5.0,0.0) );
    outp += 0.09375 *	texture2DRect(u_map, gl_FragCoord.xy + vec2(-4.0,0.0) );
    outp += 0.1640625 * texture2DRect(u_map, gl_FragCoord.xy + vec2(-3.0,0.0) );
    outp += 0.234375 *	texture2DRect(u_map, gl_FragCoord.xy + vec2(-2.0,0.0) );
    outp += 0.2734375 * texture2DRect(u_map, gl_FragCoord.xy + vec2(-1.0,0.0) );
    outp += 0.3125 *	texture2DRect(u_map, gl_FragCoord.xy );
    outp += 0.2734375 * texture2DRect(u_map, gl_FragCoord.xy + vec2(1.0,0.0) );
    outp += 0.234375 *	texture2DRect(u_map, gl_FragCoord.xy + vec2(2.0,0.0) );
    outp += 0.1640625 * texture2DRect(u_map, gl_FragCoord.xy + vec2(3.0,0.0) );
    outp += 0.09375 *	texture2DRect(u_map, gl_FragCoord.xy + vec2(4.0,0.0) );
    outp += 0.0596875 * texture2DRect(u_map, gl_FragCoord.xy + vec2(5.0,0.0) );
    outp += 0.015625 *	texture2DRect(u_map, gl_FragCoord.xy + vec2(6.0,0.0) );
 
    gl_FragColor = outp;
} 