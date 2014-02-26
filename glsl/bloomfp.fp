uniform sampler2DRect		u_map0; // screen
uniform sampler2DRect		u_map1; // bloom
uniform vec3				u_bloomParams; //0 - multipler 1 - Brightness threshold 2 - exposure level

void main(void) { 

	vec4 screen = texture2DRect(u_map0, gl_FragCoord.xy); 
	vec4 bloom = texture2DRect(u_map1, gl_FragCoord.xy*0.25);
  
	screen += bloom * u_bloomParams[0];
  
	float TM = u_bloomParams[2] * (u_bloomParams[2] / u_bloomParams[1] + 1.0) / (u_bloomParams[2] + 1.0);
	screen *= TM;     
	gl_FragColor = screen;
}
