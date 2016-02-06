layout (binding = 0) uniform sampler2DRect u_ScreenTex;
uniform float u_scroll; 

void main()
{    
    vec2 const_1 = vec2( 142.0/11.0, 1017.0/13.0 );    
    float const_2 = 481344.0/11.0;                               

    vec4 color = texture2DRect(u_ScreenTex, gl_FragCoord.xy);             
    float noise = fract( sin( dot( gl_FragCoord.xy + vec2(0, u_scroll), const_1 )) * const_2);     
    vec4 ns = vec4(noise, noise, noise, 1.0);
        
    // Color correction   
	color = color * 1.16438356 - 0.03305936;    

    fragData = mix(color, ns, 0.04);             
     
}
