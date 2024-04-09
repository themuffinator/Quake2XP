//!#include "include/global.inc"
layout (bindless_sampler, location = U_TMU0) uniform sampler2DRect	u_ScreenTex;
layout (bindless_sampler, location = U_TMU1) uniform sampler2D  	u_LumTex;

layout(location = U_PARAM_FLOAT_1)	uniform float	u_gamma;	
layout(location = U_PARAM_FLOAT_2)	uniform float	u_EVcomp;

#include pbCamera.inc   //!#include "include/pbCamera.inc"

vec3 uncharted2Tonemap(vec3 x) {
  float A = 0.15;
  float B = 0.50;
  float C = 0.10;
  float D = 0.20;
  float E = 0.02;
  float F = 0.30;
  return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

void main(){
	
    vec3 hdrColor   = texture(u_ScreenTex, gl_FragCoord.xy).rgb;
    vec3 luminance  = exp2(textureLod(u_LumTex, vec2(0.5), 8).rgb);
    float avgLum    = log2(dot(vec3(0.2125, 0.7154, 0.0721), luminance));
    
    float targetEV      = ComputeTargetEV(avgLum);
    targetEV            = targetEV + u_EVcomp;
    float aperture      = 4.0;
    float focalLength   = 70.0;
    float shutterSpeed  = 1.0 / (focalLength * 1000.0);

    // Compute the resulting ISO if we left both shutter and aperture here
    float iso = clamp(ComputeISO(aperture, shutterSpeed, targetEV), MIN_ISO, MAX_ISO);
    // Apply half the difference in EV to the aperture
    float evDiff = targetEV - ComputeEV(aperture, shutterSpeed, iso);
    aperture = clamp(aperture * pow(sqrt(2.0f), evDiff * 0.5f), MIN_APERTURE, MIN_APERTURE);
    // Apply the remaining difference to the shutter speed
    evDiff = targetEV - ComputeEV(aperture, shutterSpeed, iso);
    shutterSpeed = clamp(shutterSpeed * pow(2.0f, -evDiff), MIN_SHUTTER, MAX_SHUTTER);

    float exposure = getStandardOutputBasedExposure(aperture, shutterSpeed, iso, 0.18f);

	vec3 exposedColor   = exposure * hdrColor.rgb;
	vec3 curr           = uncharted2Tonemap( exposedColor );
	vec3 whiteScale     = 1.0 / uncharted2Tonemap( vec3(1.0 - avgLum) );
	hdrColor.rgb        = curr * whiteScale;
    float gamma         = 1.0 / u_gamma; // hdr gamma 2.2
	fragData.rgb        = pow( hdrColor.rgb, vec3(gamma) );

    // store luma for fxaa
    float lum = dot(vec3(0.2125, 0.7154, 0.0721), fragData.rgb);
    fragData.a = lum;
}
