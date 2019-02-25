layout (binding = 0) uniform sampler2DRect	u_screenMap;

layout (location = U_PARAM_INT_0)	uniform int		u_fogType;  // exp1 and exp2
layout (location = U_PARAM_VEC4_0)	uniform vec4	u_fogParams; //rgb and density

void main(void){

	vec3 backBuffer = texture2DRect(u_screenMap, gl_FragCoord.xy).rgb;
	
	float fogCoord = abs(gl_FragCoord.z / gl_FragCoord.w);
    
	float fogFactor;
	
	if(u_fogType == 1)
		fogFactor = exp(-u_fogParams.w * fogCoord); //exp1    
	
	if(u_fogType == 2)
		fogFactor = exp(-pow(u_fogParams.w * fogCoord, 2.0)); //exp2
	
	fragData.rgb = mix(backBuffer, u_fogParams.xyz, fogFactor);
	fragData.w = 1.0;
}
