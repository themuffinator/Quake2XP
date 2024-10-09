//!#include "include/global.inc"
layout	(bindless_sampler, location  = U_TMU0)	uniform sampler2D		u_map; 
layout	(location = U_SCREEN_SIZE)				uniform vec2			u_screenSize; 

void main(){
	vec2	uv	= gl_FragCoord.xy / u_screenSize;  
	vec4	color = texture( u_map, uv.xy);  
	float	lum = dot(vec4(0.2125, 0.7154, 0.0721, 0.0), color);

	if(lum >= 1.0)
		fragData = vec4(color);
	else
		fragData = vec4(0.0);
	fragData.a = 1.0;
}
