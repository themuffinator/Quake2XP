layout (binding = 0) uniform sampler2DRect u_map;

void main(void)
{
  
    vec4 
    outp  = 0.015625 *	    	texture2DRect(u_map, gl_FragCoord.xy - vec2(0.0,6.0) );
    outp += 0.0596875 *		texture2DRect(u_map, gl_FragCoord.xy - vec2(0.0,5.0) );
    outp += 0.09375 *		texture2DRect(u_map, gl_FragCoord.xy - vec2(0.0,4.0) );
    outp += 0.1640625 *		texture2DRect(u_map, gl_FragCoord.xy - vec2(0.0,3.0) );
    outp += 0.234375 *		texture2DRect(u_map, gl_FragCoord.xy - vec2(0.0,2.0) );
    outp += 0.2734375 *		texture2DRect(u_map, gl_FragCoord.xy - vec2(0.0,1.0) );
    outp += 0.3125 *		texture2DRect(u_map, gl_FragCoord.xy );
    outp += 0.2734375 *		texture2DRect(u_map, gl_FragCoord.xy + vec2(0.0,1.0) );
    outp += 0.234375 *		texture2DRect(u_map, gl_FragCoord.xy + vec2(0.0,2.0) );
    outp += 0.1640625 *		texture2DRect(u_map, gl_FragCoord.xy + vec2(0.0,3.0) );
    outp += 0.09375 *		texture2DRect(u_map, gl_FragCoord.xy + vec2(0.0,4.0) );
    outp += 0.0596875 *		texture2DRect(u_map, gl_FragCoord.xy + vec2(0.0,5.0) );
    outp += 0.015625 *		texture2DRect(u_map, gl_FragCoord.xy + vec2(0.0,6.0) );
    
	fragData = outp * 0.5;
} 
 