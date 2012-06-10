uniform sampler2DRect u_map; 
uniform float threshold;   

void main()   
{  
vec2 tc = gl_FragCoord.xy * 4.0; 

vec4 b = texture2DRect( u_map, tc );  

b += texture2DRect( u_map, tc + vec2(1.0, 0.0)); 
b += texture2DRect( u_map, tc + vec2(0.0, 1.0)); 
b += texture2DRect( u_map, tc + vec2(1.0, 1.0)); 
b *= 0.25;

// Cut off?  
gl_FragColor = max( b - threshold, 0.0 ) * ( 1.0 / threshold );  
}
