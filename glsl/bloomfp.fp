layout (binding = 0) uniform sampler2DRect		u_map0; // screen
layout (binding = 1) uniform sampler2DRect		u_map1; // bloom

uniform vec3	u_bloomParams; //0 - multipler 1 - Brightness threshold 2 - exposure level

void main(void) { 

	vec4 screen = texture2DRect(u_map0, gl_FragCoord.xy); 
	vec4 bloom = texture2DRect(u_map1, gl_FragCoord.xy * 0.25);
  
	screen += bloom * u_bloomParams.x;
  
	float TM = u_bloomParams.z * (u_bloomParams.z / u_bloomParams.y + 1.0) / (u_bloomParams.z + 1.0);
	screen *= TM;     
	fragData = screen;
}
