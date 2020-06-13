//!#include "include/global.inc"

layout (bindless_sampler, location = U_TMU0) uniform sampler2DRect	u_ScreenTex;
layout (bindless_sampler, location = U_TMU1) uniform sampler2DRect	u_DepthTex;

layout (location = U_SCREEN_SIZE)	uniform vec2	u_screenSize;
layout (location = U_PARAM_VEC4_0)	uniform vec4	u_dofParams;

#include depth.inc //!#include "include/depth.inc"

#define DOF_SAMPLES 17

const	vec2 dofOffsets[DOF_SAMPLES] = vec2[]( 
		vec2(0.0,	0.0),
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

#define	ASPECTRATIO u_screenSize.x / u_screenSize.y

void main(void){

	vec2 aspectCorrect = vec2(1.0, ASPECTRATIO);
   
	// Z-feather
	float depth = DecodeDepth(texture2DRect(u_DepthTex, gl_FragCoord.xy).x, u_dofParams.zw);
	float factor = depth - u_dofParams.x;

	vec2 dofblur = vec2 (clamp(factor * u_dofParams.y, -3.0, 3.0));

	vec4 col = vec4(0.0);
	for (int i = 0; i< DOF_SAMPLES; i++)
		col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (dofOffsets[i] * aspectCorrect) * dofblur);
	
	fragData = col / DOF_SAMPLES;
}

