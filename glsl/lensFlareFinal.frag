//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0) uniform sampler2DRect		u_map0; // screen
layout (bindless_sampler, location  = U_TMU1) uniform sampler2D			u_map1; // lens dirt
layout (bindless_sampler, location  = U_TMU2) uniform sampler2D			u_map2; // lens flare
layout (bindless_sampler, location  = U_TMU3) uniform sampler2D			u_map3; // lens star burst

layout(location = U_SCREEN_SIZE)		uniform vec2	u_screenSize; 
layout(location = U_TEXTURE0_MATRIX)	uniform mat4	u_flareRotateMatrix;
layout(location = U_PARAM_FLOAT_0)		uniform float	u_intens;

void main(void) { 
	vec2 uv = gl_FragCoord.xy / u_screenSize;
	vec2 uv2 = gl_FragCoord.xy / u_screenSize - vec2(0.5);
	vec2 lensStarUv = (u_flareRotateMatrix * vec4(uv2.xy, 0.0, 0.0)).xy + 0.5;

	vec4 screen	= texture(u_map0, gl_FragCoord.xy); 
	vec4 dirt	= texture(u_map1, uv);
	vec4 flare	= texture(u_map2, uv);
	vec4 burst	= texture(u_map3, lensStarUv);
	flare *= dirt * burst;
	flare *= u_intens;

	float blend_factor = 0.002;
	fragData = screen + (screen * blend_factor + flare * (vec4(1.0) - blend_factor));
	fragData.a = 1.0;
}
