layout (binding = 0) uniform sampler2DRect		u_map0; // screen
layout (binding = 1) uniform sampler2DRect		u_map1; // bloom

uniform float	u_bloomParams; //multipler

void main(void) { 

	vec4 screen = texture2DRect(u_map0, gl_FragCoord.xy); 
	vec4 bloom = texture2DRect(u_map1, gl_FragCoord.xy * 0.25);
  
	screen += bloom * u_bloomParams;
	fragData = screen;
}
