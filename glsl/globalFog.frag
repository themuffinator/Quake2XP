//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0) uniform sampler2DRect	u_screenMap;
layout (bindless_sampler, location  = U_TMU1) uniform sampler2DRect	u_depthMap;

layout	(location = U_PARAM_INT_0)		uniform int		u_fogType;		// exp1 and exp2
layout	(location = U_PARAM_VEC4_0)		uniform vec4	u_fogParams;	//world rgb and density
layout	(location = U_PARAM_VEC4_1)		uniform vec4	u_fogSkyParams; //sky rgb and density
layout	(location = U_PARAM_VEC2_0)		uniform vec2	u_fogBias;		// x - world, y - sky

void main(void){
	vec3 backBuffer = texture(u_screenMap, gl_FragCoord.xy).rgb;
	float depth = texture(u_depthMap, gl_FragCoord.xy).x;

	bool sky;
	
	if(depth >= 0.9999)
		sky = false;
	else
		sky = true;

	depth = depth * 0.5 + 0.5;

	float fogCoord =  abs(gl_FragCoord.z / gl_FragCoord.w);
	
	if(!sky){
		fogCoord += u_fogBias.x;
		fogCoord /= depth;
	}
	if(sky)
		fogCoord += u_fogBias.y;	

	vec3 fogColor;
	float fogFactor, density;

	if(sky){
		fogColor = u_fogSkyParams.xyz;
		density = 1000.0 * u_fogSkyParams.w; // reverse! w/o depth map value
	 }

	 if(!sky){
		fogColor = u_fogParams.xyz;
		density = 100.0 / u_fogParams.w;
	}

	if(u_fogType == 0)
		fogFactor = exp(-density * fogCoord); //exp1    
	
	if(u_fogType == 1)
		fogFactor = exp(-pow(density * fogCoord, 2.0)); //exp2
	
	fragData.rgb = mix(backBuffer, fogColor, fogFactor);
	fragData.w = 1.0;
}
