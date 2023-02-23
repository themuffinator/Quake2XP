//!#include "include/global.inc"
layout (bindless_sampler, location = U_TMU0) uniform sampler2DRect	u_ScreenTex;

layout(location = U_PARAM_FLOAT_1)	uniform float	u_gamma;	
layout(location = U_PARAM_VEC2_0)	uniform vec2	u_hdrParams;

vec3 uncharted2Tonemap(vec3 x) {
  float A = 0.15;
  float B = 0.50;
  float C = 0.10;
  float D = 0.20;
  float E = 0.02;
  float F = 0.30;
  float W = 11.2;
  return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

void main(){
	
    vec3 hdrColor = texture(u_ScreenTex, gl_FragCoord.xy).rgb;

	float hdrMaxLuminance		= u_hdrParams.x;
	float exposure				= u_hdrParams.y;

	vec3 exposedColor = exposure * hdrColor.rgb;

	vec3 curr = uncharted2Tonemap( exposedColor );

	vec3 whiteScale = 1.0 / uncharted2Tonemap( vec3( hdrMaxLuminance ) );
	hdrColor.rgb = curr * whiteScale;

    float gamma = 1.0 / u_gamma; // hdr gamma 2.2
	fragData.rgb = pow( hdrColor.rgb, vec3(gamma) );
    fragData.a = 1.0;
}