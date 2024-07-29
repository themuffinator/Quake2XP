//!#include "include/global.inc"

layout (bindless_sampler, location = U_TMU0) uniform sampler2DRect	u_ScreenTex;
layout (bindless_sampler, location = U_TMU1) uniform sampler2D  	u_LumTex;

layout(location = U_PARAM_FLOAT_0)	uniform float	u_gamma;	
layout(location = U_PARAM_FLOAT_1)	uniform float	u_EVcomp;

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
	
    vec3    hdrColor    = texture(u_ScreenTex, gl_FragCoord.xy).rgb;
    vec3    currLum     = exp2(textureLod(u_LumTex, vec2(0.5), 8).rgb);
    float   lum         = log2(dot(vec3(0.2125, 0.7154, 0.0721), currLum));
    float   exposure    = ComputeExposure(lum, u_EVcomp);

    if(u_hdrOutput == 1){
	// Set gamut mapping matrix 
	// 0 = sRGB - D65
	// 1 = P3 - D65b
	// 2 = Rec.2020 - D65
	// 3 = ACES AP0 - D60
	// 4 = ACES AP1 - D60
    const mat3 AP1_2_Output  = OuputGamutMappingMatrix( u_colorSpace );
    vec3 ODTColor = ACESOutputTransforms1000(hdrColor * exposure);
    // Convert from AP1 to specified output gamut
	fragData.rgb = ODTColor * AP1_2_Output;
}else{
	vec3 currColor      = uncharted2Tonemap( hdrColor.rgb * 1.2);
	vec3 whiteScale     = 1.0 / uncharted2Tonemap(vec3(W));
    fragData.rgb        = currColor * whiteScale;
    fragData.rgb        = pow( fragData.rgb, vec3(1.0/u_gamma)); //vid menu controled
}
    // store luma for fxaa
    fragData.a = dot(vec3(0.2125, 0.7154, 0.0721), fragData.rgb);
}
