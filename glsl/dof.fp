layout (binding = 0) uniform sampler2DRect	u_ScreenTex;
layout (binding = 1) uniform sampler2DRect	u_DepthTex;

uniform vec2	u_screenSize;
uniform vec4	u_dofParams;

#include depth.inc

const	vec2 dofOffsets[16] = vec2[]( 
		vec2(0.0,	0.4),
        vec2(0.15,	0.37),
        vec2(0.29,	0.29),
        vec2(-0.37,	0.15),       
        vec2(0.4,	0.0), 
        vec2(0.37,	-0.15),       
        vec2(0.29,	-0.29),       
        vec2(-0.15,	-0.37),
        vec2(0.0,	-0.4), 
        vec2(-0.15,	0.37),
        vec2(-0.29,	0.29),
        vec2(0.37,	0.15), 
        vec2(-0.4,	0.0),  
        vec2(-0.37,	-0.15),      
        vec2(-0.29,	-0.29),     
        vec2(0.15,	-0.37)
		);

#define DOF_SAMPLES 17
#define	ASPECTRATIO u_screenSize.x / u_screenSize.y

void main(void){

	vec2 aspectcorrect = vec2(1.0, ASPECTRATIO);
   
	// Z-feather
	float depth = DecodeDepth(texture2DRect(u_DepthTex, gl_FragCoord.xy).x, u_dofParams.zw);
	float factor = depth - u_dofParams.x;

	vec2 dofblur = vec2 (clamp(factor * u_dofParams.y, -3.0, 3.0));

	vec4 col = vec4(0.0);
    col += texture2DRect(u_ScreenTex, gl_FragCoord.xy);

	for (int i = 0; i< DOF_SAMPLES; i++)
		col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (dofOffsets[i] * aspectcorrect) * dofblur);
	
	fragData = col / DOF_SAMPLES;
}

