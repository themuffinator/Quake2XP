//!#include "include/global.inc"
layout (bindless_sampler, location = U_TMU0) uniform sampler2DRect	u_ScreenTex;

layout(location = U_COLOR_PARAMS)	uniform vec4	u_control;			// x - brightens, y - contrast, z - saturation, w - 1 / gamma
layout(location = U_COLOR_VIBRANCE)	uniform vec3	u_rgbVibrance;		// pre-multipled values vibrance * rgb

vec3 BrightnesContrastSaturation(vec3 color, float brt, float con, float sat)
{
  // Increase or decrease theese values to adjust r, g and b color channels seperately
  const float AvgLumR = 0.5;
  const float AvgLumG = 0.5;
  const float AvgLumB = 0.5;
  
  const vec3 LumCoeff = vec3(0.2125, 0.7154, 0.0721);
  
	vec3 AvgLumin = vec3(AvgLumR, AvgLumG, AvgLumB);
	vec3 brtColor = color * brt;
	vec3 intensity = vec3(dot(brtColor, LumCoeff));
	vec3 satColor = mix(intensity, brtColor, sat);
	vec3 conColor = mix(AvgLumin, satColor, con);
	return conColor;
}

vec3 ColorVibrance(in vec3 color){

  float lum = dot(vec3(0.2125, 0.7154, 0.0721), color);

  float minColor	= min(color.r, min(color.g, color.b));
  float maxColor	= max(color.r, max(color.g, color.b));
  float mid			= maxColor - minColor;

  return color  = mix(vec3(lum), color.rgb, (1.0 + (u_rgbVibrance * (1.0 - (sign(u_rgbVibrance) * mid)))));
}

#ifndef saturate
#define saturate(x)    clamp(x, 0.0, 1.0)
#endif

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
  float exposureBias = 2.0;
  vec3 curr = uncharted2Tonemap(exposureBias * color);
  vec3 whiteScale = 1.0 / uncharted2Tonemap(vec3(W));
  return curr * whiteScale;
}

float uncharted2Tonemap(float x) {
  float A = 0.15;
  float B = 0.50;
  float C = 0.10;
  float D = 0.20;
  float E = 0.02;
  float F = 0.30;
  float W = 11.2;
  return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float uncharted2(float color) {
  const float W = 11.2;
  const float exposureBias = 2.0;
  float curr = uncharted2Tonemap(exposureBias * color);
  float whiteScale = 1.0 / uncharted2Tonemap(W);
  return curr * whiteScale;
}

void main(void){

vec3 color = BrightnesContrastSaturation(texture2DRect(u_ScreenTex, gl_FragCoord.xy).rgb, u_control.x, u_control.y, u_control.z);
color = ColorVibrance(color);
//color = uncharted2(color);
fragData.rgb = pow(color, vec3(u_control.w));
fragData.a = 1.0;
}
