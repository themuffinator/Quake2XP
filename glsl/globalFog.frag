//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0) uniform sampler2DRect	u_screenMap;
layout (bindless_sampler, location  = U_TMU1) uniform sampler2DRect	u_depthMap;	

layout	(location = U_PARAM_INT_0)		uniform int		u_fogType;	//	exp1 and exp2
layout	(location = U_PARAM_VEC4_0)		uniform vec4	u_fogColor;	//	rgb and density
layout	(location = U_PARAM_FLOAT_0)	uniform float	u_fogBias;	//	bias

void main(void){

	vec3	fogColor, screenMap;
	float	fogCoord, fogFactor, density, depth;

	screenMap	= texture(u_screenMap,	gl_FragCoord.xy).rgb;
	depth		= texture(u_depthMap,	gl_FragCoord.xy).x; // read linear depth
	
	if(depth > 8192.0){ // its sky
		fragData = vec4(screenMap, 1.0);
		return;
	}

	fogCoord  = abs(gl_FragCoord.z / gl_FragCoord.w) / (depth * 0.5 + 0.5);
	fogCoord += u_fogBias;
	fogColor  = u_fogColor.rgb;
	density   = 100.0 / u_fogColor.a;

	if(u_fogType == 0)
		fogFactor = exp(-density * fogCoord); //exp1    
	else
		fogFactor = exp(-pow(density * fogCoord, 2.0)); //exp2
	
	fragData.rgb = mix(screenMap, fogColor, fogFactor);
	fragData.w = 1.0;
}
