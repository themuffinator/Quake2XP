uniform sampler2DRect		u_map0; // screen
uniform sampler2DRect		u_map1; // bloom
uniform float				u_bloomAlpha;

void main(void) { 

	vec4 a = texture2DRect(u_map0, gl_FragCoord.xy); 
	vec4 b = texture2DRect(u_map1, gl_FragCoord.xy*0.25); 
	gl_FragColor = a + b * u_bloomAlpha;

}
