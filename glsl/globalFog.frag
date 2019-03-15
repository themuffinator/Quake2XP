layout (binding = 0) uniform sampler2DRect	u_screenMap;
layout (binding = 1) uniform sampler2DRect	u_depthMap;
layout (binding = 2) uniform sampler2DRect	u_skyMaskMap;

layout	(location = U_PARAM_INT_0)		uniform int		u_fogType;  // exp1 and exp2
layout	(location = U_PARAM_VEC4_0)		uniform vec4	u_fogParams; //rgb and density
layout	(location = U_PARAM_VEC4_1)		uniform vec4	u_fogSkyParams; //sky rgb and density
layout	(location = U_PARAM_FLOAT_0)	uniform float	u_fogBias;
layout	(location = U_DEPTH_PARAMS)		uniform vec2	u_depthParms;

#include depth.inc

void main(void){

	float mask = texture2DRect(u_skyMaskMap, gl_FragCoord.xy).r;
	
	vec3 backBuffer = texture2DRect(u_screenMap, gl_FragCoord.xy).rgb;
	float depth = DecodeDepth(texture2DRect(u_depthMap, gl_FragCoord.xy).x, u_depthParms);
	depth = depth * 0.5 + 0.5;

	float fogCoord =  abs(gl_FragCoord.z / gl_FragCoord.w);
	fogCoord += u_fogBias;
	fogCoord /= depth;

	vec3 fogColor;
	float fogFactor, density;

	if(mask == 1.0){
		fogColor = u_fogSkyParams.xyz;
		density = 100.0 / u_fogSkyParams.w;
	 }

	 if(mask == 0.0){
		fogColor = u_fogParams.xyz;
		density = 100.0 / u_fogParams.w;
	}

	if(u_fogType == 1)
		fogFactor = exp(-density * fogCoord); //exp1    
	
	if(u_fogType == 2)
		fogFactor = exp(-pow(density * fogCoord, 2.0)); //exp2
	
	fragData.rgb = mix(backBuffer, fogColor, fogFactor);
	fragData.w = 1.0;
}
