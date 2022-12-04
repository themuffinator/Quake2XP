//!#include "include/global.inc"
layout (bindless_sampler, location = U_TMU0) uniform sampler2DRect	u_ScreenTex;

layout(location = U_COLOR_PARAMS)	uniform vec3	u_control;			// x - brightens, y - contrast, z - saturation
layout(location = U_COLOR_VIBRANCE)	uniform vec3	u_rgbVibrance;		// pre-multipled values vibrance * rgb

void BrightnesContrastSaturation(inout vec3 color, float brt, float con, float sat){

  // Increase or decrease theese values to adjust r, g and b color channels seperately
  const float AvgLumR = 0.5;
  const float AvgLumG = 0.5;
  const float AvgLumB = 0.5;
  
  const vec3 LumCoeff = vec3(0.2125, 0.7154, 0.0721);
  
	vec3 AvgLumin = vec3(AvgLumR, AvgLumG, AvgLumB);
	color = color * brt;
	vec3 intensity = vec3(dot(color, LumCoeff));
	color = mix(intensity, color, sat);
	color = mix(AvgLumin, color, con);
}

void Vibrance(inout vec3 color){

  float lum = dot(vec3(0.2125, 0.7154, 0.0721), color);

  float minColor	= min(color.r, min(color.g, color.b));
  float maxColor	= max(color.r, max(color.g, color.b));
  float mid			= maxColor - minColor;
  color  = mix(vec3(lum), color.rgb, (1.0 + (u_rgbVibrance * (1.0 - (sign(u_rgbVibrance) * mid)))));
}



void main(void){
	vec3 color = texture(u_ScreenTex, gl_FragCoord.xy).rgb;
	BrightnesContrastSaturation(color, u_control.x, u_control.y, u_control.z);
	Vibrance(color);
	fragData.rgb = color;
	fragData.a = 1.0;
}
