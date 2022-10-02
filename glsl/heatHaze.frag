//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0) uniform sampler2D		u_deformMap;
layout (bindless_sampler, location  = U_TMU1) uniform sampler2DRect	g_colorBufferMap;
layout (bindless_sampler, location  = U_TMU2) uniform sampler2DRect	g_depthBufferMap;

layout(location = U_REFR_THICKNESS0)	uniform float	u_thickness; //depth feather
layout(location = U_SCREEN_SIZE)		uniform vec2	u_viewport;


in float	v_depth;
in vec2		v_deformMul;
in vec2		v_texCoord;

void main (void) {

	vec2 N = texture(u_deformMap, v_texCoord).xy * 2.0 - 1.0;

	// Z-feather
	float depth = texture(g_depthBufferMap, gl_FragCoord.xy).x;
	N *= clamp((depth - v_depth) / u_thickness, 0.0, 1.0);
	// scale by the deform multiplier and the viewport size
	N *= v_deformMul * u_viewport.xy;
    fragData.rgb = texture(g_colorBufferMap, gl_FragCoord.xy + N).rgb;
    fragData.a = 1.0;
}