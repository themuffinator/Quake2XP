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
  return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
const mat3 ACESInputMat =
{
    {0.59719, 0.35458, 0.04823},
    {0.07600, 0.90834, 0.01566},
    {0.02840, 0.13383, 0.83777}
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
const mat3 ACESOutputMat =
{
    { 1.60475, -0.53108, -0.07367},
    {-0.10208,  1.10813, -0.00605},
    {-0.00327, -0.07276,  1.07602}
};

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

vec3 ACESFitted(vec3 color)
{
    color = color * ACESInputMat;

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = color * ACESOutputMat;

    // Clamp to [0, 1]
    color = saturate(color);

    return color;
}

void main(){
	
    vec3 hdrColor = texture(u_ScreenTex, gl_FragCoord.xy).rgb;

/*    hdrColor = ACESFitted(hdrColor) * 1.68;
    fragData.rgb = pow( hdrColor.rgb, vec3(1.0/u_gamma) );
    fragData.a = 1.0;
    return;
    */
	float hdrMaxLuminance		= u_hdrParams.x;
	float exposure				= u_hdrParams.y;

	vec3 exposedColor = exposure * hdrColor.rgb;

	vec3 curr = uncharted2Tonemap( exposedColor );

	vec3 whiteScale = 1.0 / uncharted2Tonemap( vec3( hdrMaxLuminance ) );
	hdrColor.rgb = curr * whiteScale;

    float gamma = 1.0 / u_gamma; // hdr gamma 2.2
	fragData.rgb = pow( hdrColor.rgb, vec3(gamma) );
    // store luma for fxaa
    float lum = dot(vec3(0.2125, 0.7154, 0.0721), fragData.rgb);
    fragData.a = lum;
}
