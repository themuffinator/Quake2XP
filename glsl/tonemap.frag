//!#include "include/global.inc"
layout (bindless_sampler, location = U_TMU0) uniform sampler2DRect	u_ScreenTex;

layout(location = U_PARAM_FLOAT_0)	uniform float	u_exposure;	
layout(location = U_PARAM_FLOAT_1)	uniform float	u_gamma;	

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

vec3 uncharted2(vec3 color) {
  const float W = 11.2;
  vec3 curr = uncharted2Tonemap(u_exposure * color);
  vec3 whiteScale = 1.0 / uncharted2Tonemap(vec3(W));
  return curr * whiteScale;
}


void main()
{
    vec3 hdrColor = texture2DRect(u_ScreenTex, gl_FragCoord.xy).rgb;
    fragData.rgb = uncharted2(hdrColor);
    fragData.rgb = pow(fragData.rgb, vec3(1.0 / u_gamma));
    fragData.a = 1.0;
}