//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0) uniform sampler2D		u_deformMap;
layout (bindless_sampler, location  = U_TMU1) uniform sampler2D		u_colorMap;
layout (bindless_sampler, location  = U_TMU2) uniform sampler2DRect	g_colorBufferMap;
layout (bindless_sampler, location  = U_TMU3) uniform sampler2DRect	g_depthBufferMap;

layout(location = U_REFR_THICKNESS0)	uniform float	u_thickness0; //depth feather
layout(location = U_SCREEN_SIZE)		uniform vec2	u_viewport;
layout(location = U_DEPTH_PARAMS)		uniform vec2	u_depthParms;
layout(location = U_AMBIENT_LEVEL)		uniform float	u_ambientScale;
layout(location = U_PARAM_FLOAT_0)		uniform float	u_blurScale;


in float	v_depth;
in float	v_depthS;
in vec2		v_deformMul;
in vec2		v_deformTexCoord;
in vec4		v_color;

#include depth.inc //!#include "include/depth.inc"
#include blur.inc //!#include "include/blur.inc"

void main (void) {

	vec2 N = texture(u_deformMap, v_deformTexCoord).xy * 2.0 - 1.0;
	vec4 diffuse  = texture(u_colorMap,  v_deformTexCoord.xy);

	// Z-feather
	float depth = DecodeDepth(texture(g_depthBufferMap, gl_FragCoord.xy).x, u_depthParms);
	N *= clamp((depth - v_depth) / u_thickness0, 0.0, 1.0);
	// scale by the deform multiplier and the viewport size
	N *= v_deformMul * u_viewport.xy;
	
	// world refracted surfaces
	// chromatic aberration approximation
    vec4 clearGlass;
    clearGlass.r = texture(g_colorBufferMap, gl_FragCoord.xy + N * 0.85).r;
	clearGlass.g = texture(g_colorBufferMap, gl_FragCoord.xy + N * 1.00).g;
	clearGlass.b = texture(g_colorBufferMap, gl_FragCoord.xy + N * 1.15).b;

    vec4 bluredGlass = boxBlur2(g_colorBufferMap, max(8.0, diffuse.a * u_blurScale), N);

    fragData = mix (bluredGlass, clearGlass, diffuse.a);

    // blend glass texture
	diffuse.rgb *= u_ambientScale;
	fragData.xyz += diffuse.xyz * ((v_color.rgb * 2.0) * v_color.a);

    fragData.w = 1.0;
}