//!#include "include/global.inc"
layout (bindless_sampler, location = U_TMU0) uniform sampler2D u_ScreenTex;
layout (bindless_sampler, location = U_TMU1) uniform sampler2D u_lensDirtTex;

layout(location = U_PARAM_FLOAT_0)	uniform float	u_vignetSize;
layout(location = U_PARAM_INT_0)	uniform int		u_lensDirt;
layout(location = U_SCREEN_SIZE)	uniform vec2	u_screenSize; 

  
// CHROMATIC ABBERATION

vec2 BarrelDistortion( vec2 xy, float amount )
{
	vec2 cc = xy - 0.5;
	float dist = dot( cc, cc );

	return xy + cc * dist * amount;
}

float Linterp( float t )
{
	return saturate( 1.0 - abs( 2.0 * t - 1.0 ) );
}

float Remap( float t, float a, float b )
{
	return saturate( ( t - a ) / ( b - a ) );
}

vec3 SpectrumOffset( float t )
{
	float lo = step( t, 0.5 );
	float hi = 1.0 - lo;
	float w = Linterp( Remap( t, 1.0 / 6.0, 5.0 / 6.0 ) );
	vec3 ret = vec3( lo, 1.0, hi ) * vec3( 1.0 - w, w, 1.0 - w );

	return pow( ret, vec3( 1.0 / 2.2 ) );
}

#define Chromatic_Power 0.075
#define Chromatic_Samples 12

void main()
{    
	vec2 uv = gl_FragCoord.xy / u_screenSize;
	vec4 color = texture(u_ScreenTex, uv);
	vec3 dirt = vec3(0.0);
	float lum = 0.0;

	lum = dot(vec3(0.2125, 0.7154, 0.0721), color.rgb);
	dirt = pow(texture(u_lensDirtTex, uv).rgb, vec3(12.0));
	dirt *= lum;    

	vec3 sum = vec3( 0.0 );
	vec3 sumColor = vec3( 0.0 );

	for(int i = 0; i < Chromatic_Samples; i++){

		float t = ( float(i) / ( float(Chromatic_Samples) - 1.0 ) );
		vec3 so = SpectrumOffset( t );

		sum += so.xyz;
		sumColor += so * texture(u_ScreenTex, BarrelDistortion( uv, ( 0.5 * Chromatic_Power * t ) ) ).rgb /*+ dirt*/;
	}

	color.rgb = ( sumColor / sum );
	fragData = color;
	fragData.a = 1.0;

	float OuterVignetting	= 1.4 - u_vignetSize;
	float InnerVignetting	= 1.0 - u_vignetSize;

	float d = distance(vec2(0.5, 0.5), uv) * 1.414213;
	float vignetting = clamp((OuterVignetting - d) / (OuterVignetting - InnerVignetting), 0.0, 1.0);
	fragData *= vignetting;
	fragData.a = 1.0;
}
