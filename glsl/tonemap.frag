//!#include "include/global.inc"

layout (bindless_sampler, location = U_TMU0) uniform sampler2DRect	u_ScreenTex;
layout (bindless_sampler, location = U_TMU1) uniform sampler2D  	u_LumTex;

layout(location = U_PARAM_FLOAT_0)	uniform float	u_gamma;	
layout(location = U_PARAM_FLOAT_1)	uniform float	u_EVcomp;
layout(location = U_PARAM_FLOAT_2)	uniform float	u_hdrMaxIso;
layout(location = U_PARAM_FLOAT_3)	uniform float	u_WhiteTemp;
layout(location = U_PARAM_FLOAT_4)	uniform float	u_WhiteTint;
layout(location = U_PARAM_FLOAT_5)	uniform float	u_blueCorrection;

layout(location = U_PARAM_INT_0)	uniform int	    u_colorSpace;
layout(location = U_PARAM_INT_1)	uniform int	    u_hdrOutput;

#include aces.inc   //!#include "include/aces.inc"
#include pbCamera.inc   //!#include "include/pbCamera.inc"

float A = 0.15;
float B = 0.50;
float C = 0.10;
float D = 0.20;
float E = 0.02;
float F = 0.30;
float W = 1.0;

vec3 uncharted2Tonemap(vec3 x){
   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

void main(){
	
    vec3    hdrColor        = texture(u_ScreenTex, gl_FragCoord.xy).rgb;
    vec3    currLum         = exp2(textureLod(u_LumTex, vec2(0.5), 7).rgb);
    float   avgLuminance    = log2(dot(vec3(0.2125, 0.7154, 0.0721), currLum));

    if(u_hdrOutput == 1){
	// Set gamut mapping matrix 
	// 0 = sRGB - D65
	// 1 = P3 - D65b
	// 2 = Rec.2020 - D65
	// 3 = ACES AP0 - D60
	// 4 = ACES AP1 - D60
	
	const mat3 BlueCorrect = {
		{0.9404372683, -0.0183068787, 0.0778696104},
		{0.0083786969,  0.8286599939, 0.1629613092},
		{0.0005471261, -0.0008833746, 1.0003362486}
	};
	
	const mat3 AP0_2_AP1 = AP0_2_XYZ_MAT * XYZ_2_AP1_MAT;
	const mat3 AP1_2_AP0 = AP1_2_XYZ_MAT * XYZ_2_AP0_MAT;

	mat3 BlueCorrectAP1  = AP1_2_AP0 * BlueCorrect;
	BlueCorrectAP1 = BlueCorrectAP1 * AP0_2_AP1;
	hdrColor = WhiteBalance(hdrColor * BlueCorrectAP1);

    const mat3  AP1_2_Output    = OuputGamutMappingMatrix(u_colorSpace);
    float       exposure        = ComputeExposure(avgLuminance, u_EVcomp, u_hdrMaxIso);
    vec3        ODTColor        = ACESOutputTransforms1000(hdrColor * exposure);
    // Convert from AP1 to specified output gamut
	fragData.rgb	=	ODTColor * AP1_2_Output;
  fragData.b *= u_blueCorrection;	
}
else{    
    float exposure    = ComputeExposure(avgLuminance, u_EVcomp, MAX_ISO);
    vec3  currColor   = uncharted2Tonemap( hdrColor.rgb * exposure );
    vec3  whiteScale  = 1.0 / uncharted2Tonemap(vec3(W));
    fragData.rgb      = currColor * whiteScale;    
}
    fragData.rgb      = pow(fragData.rgb, vec3(1.0/u_gamma));
    // store luma for fxaa
    fragData.a = log2(dot(vec3(0.2125, 0.7154, 0.0721), fragData.rgb));
}
