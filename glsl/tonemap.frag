//!#include "include/global.inc"
layout (bindless_sampler, location = U_TMU0) uniform sampler2DRect	u_ScreenTex;
layout (bindless_sampler, location = U_TMU1) uniform sampler2D  	u_LumTex;
layout (bindless_sampler, location = U_TMU2) uniform sampler2D  	u_prevLumTex;

layout(location = U_PARAM_FLOAT_1)	uniform float	u_gamma;	
layout(location = U_PARAM_FLOAT_2)	uniform float	u_EVcomp;
layout(location = U_PARAM_FLOAT_3)	uniform float	u_backLerp;

#include pbCamera.inc   //!#include "include/pbCamera.inc"

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
mat3 ACESInputMat =
{
    {0.59719, 0.35458, 0.04823},
    {0.07600, 0.90834, 0.01566},
    {0.02840, 0.13383, 0.83777}
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
mat3 ACESOutputMat =
{
    { 1.60475, -0.53108, -0.07367},
    {-0.10208,  1.10813, -0.00605},
    {-0.00327, -0.07276,  1.07602}
};

vec3 RRTAndODTFit(vec3 v){

    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

vec3 ACESFitted(vec3 color){

    color = color * ACESInputMat;
    // Apply RRT and ODT
    color = RRTAndODTFit(color);
    color = color * ACESOutputMat;
    // Clamp to [0, 1]
    color = saturate(color);

    return color;
}

float A = 0.15;
float B = 0.50;
float C = 0.10;
float D = 0.20;
float E = 0.02;
float F = 0.30;
float W = 0.88;

vec3 uncharted2Tonemap(vec3 x){
   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

#define UNCHARTED
//#define HDR_OUTPUT

void main(){
	
    vec3    hdrColor    = texture(u_ScreenTex, gl_FragCoord.xy).rgb;

    vec3    currLum     = exp2(textureLod(u_LumTex, vec2(0.5), 8).rgb);
    float   avgLum      = log2(dot(vec3(0.2125, 0.7154, 0.0721), currLum));

    vec3    prevLum     = exp2(textureLod(u_prevLumTex, vec2(0.5), 8).rgb);
    float   avgPrevLum  = log2(dot(vec3(0.2125, 0.7154, 0.0721), prevLum));
    float   lum         = mix(avgLum, avgPrevLum, u_backLerp);

    float targetEV      = ComputeTargetEV(lum);
    float aperture      = 4.0;
    float focalLength   = 70.0;
    float shutterSpeed  = 1.0 / (focalLength * 1000.0);
    
    targetEV            = targetEV + u_EVcomp;

    // Compute the resulting ISO if we left both shutter and aperture here
    float iso = clamp(ComputeISO(aperture, shutterSpeed, targetEV), MIN_ISO, MAX_ISO);
    // Apply half the difference in EV to the aperture
    float evDiff = targetEV - ComputeEV(aperture, shutterSpeed, iso);
    aperture = clamp(aperture * pow(sqrt(2.0f), evDiff * 0.5f), MIN_APERTURE, MIN_APERTURE);
    // Apply the remaining difference to the shutter speed
    evDiff = targetEV - ComputeEV(aperture, shutterSpeed, iso);
    shutterSpeed = clamp(shutterSpeed * pow(2.0f, -evDiff), MIN_SHUTTER, MAX_SHUTTER);

    float exposure = getStandardOutputBasedExposure(aperture, shutterSpeed, iso, 0.18f);

#ifdef UNCHARTED
	vec3 currColor      = uncharted2Tonemap( exposure * hdrColor.rgb );
	vec3 whiteScale     = 1.0 / uncharted2Tonemap( vec3(W - lum) );
	
    #ifdef HDR_OUTPUT
        fragData.rgb        = currColor * whiteScale;
    #else
        hdrColor.rgb        = currColor * whiteScale;
        fragData.rgb        = pow( hdrColor.rgb, vec3(1.0 / u_gamma) ); //vid menu controled
    #endif
#else
    #ifdef HDR_OUTPUT
        fragData.rgb        = ACESFitted( hdrColor.rgb * exposure);
    #else
        hdrColor.rgb        = ACESFitted( hdrColor.rgb * exposure);
	    fragData.rgb        = pow( hdrColor.rgb, vec3(1.0 / 2.2) ); // force 2.2 gamma
#endif
#endif
    // store luma for fxaa
    fragData.a = dot(vec3(0.2125, 0.7154, 0.0721), fragData.rgb);
}
