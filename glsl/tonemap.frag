//!#include "include/global.inc"
layout (bindless_sampler, location = U_TMU0) uniform sampler2DRect	u_ScreenTex;
//layout (bindless_sampler, location = U_TMU1) uniform sampler2D  	u_LevelTex;

layout(location = U_PARAM_FLOAT_0)	uniform float	u_exposure;	
layout(location = U_PARAM_FLOAT_1)	uniform float	u_gamma;	
//layout(location = U_PARAM_INT_0)	uniform int	    u_maxMip;	

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
  const float W = u_exposure;
  vec3 curr = uncharted2Tonemap(color);
  vec3 whiteScale = 1.0 / uncharted2Tonemap(vec3(W));
  return curr * whiteScale;
}

void main(){

    vec3 hdrColor = texture(u_ScreenTex, gl_FragCoord.xy).rgb;
 // vec3 lumKey = textureLod(u_LevelTex, gl_FragCoord.xy, u_maxMip).rgb;
 // float lum =  dot(vec3(0.2125, 0.7154, 0.0721), lumKey);

    fragData.rgb = uncharted2(hdrColor);

    float gamma = 1.0 / u_gamma; // hdr gamma 2.2
    fragData.rgb = pow( fragData.rgb, vec3(gamma) );
    fragData.a = 1.0;
}