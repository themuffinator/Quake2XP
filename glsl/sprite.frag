//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0) uniform sampler2D		u_deformMap;
layout (bindless_sampler, location  = U_TMU1) uniform sampler2D		u_colorMap;
layout (bindless_sampler, location  = U_TMU2) uniform sampler2DRect	g_colorBufferMap;
layout (bindless_sampler, location  = U_TMU3) uniform sampler2DRect	g_depthBufferMap;

layout(location = U_REFR_ALPHA)			uniform float	u_alpha;
layout(location = U_REFR_THICKNESS0)	uniform float	u_thickness0; //depth feather
layout(location = U_REFR_THICKNESS1)	uniform float	u_thickness1; //sprite softeness
layout(location = U_SCREEN_SIZE)		uniform vec2	u_viewport;
layout(location = U_DEPTH_PARAMS)		uniform vec2	u_depthParms;
layout(location = U_COLOR_MUL)			uniform float	u_ambientScale;
layout(location = U_REFR_MASK)			uniform vec2	u_mask;			//softeness
layout(location = U_REFR_ALPHA_MASK)	uniform int		u_ALPHAMASK;	//is sprite
layout(location = U_PARAM_FLOAT_0)		uniform float	u_blurScale;


in float	v_depth;
in float	v_depthS;
in vec2		v_deformMul;
in vec2		v_deformTexCoord;

#include depth.inc  //!#include "include/depth.inc"

void main (void) {

	vec2 N = texture(u_deformMap, v_deformTexCoord).xy * 2.0 - 1.0;
	vec4 diffuse  = texture(u_colorMap,  v_deformTexCoord.xy);

	// Z-feather
	float depth = DecodeDepth(texture2DRect(g_depthBufferMap, gl_FragCoord.xy).x, u_depthParms);
	N *= clamp((depth - v_depth) / u_thickness0, 0.0, 1.0);
	// scale by the deform multiplier and the viewport size
	N *= v_deformMul * u_viewport.xy;

	float A = texture(u_deformMap, v_deformTexCoord).a;
	float softness = clamp((depth - v_depthS) / u_thickness1, 0.0, 1.0);
	// refracted sprites with soft edges
	if (A <= 0.01) {
		discard;
		return;
	}
	N *= A;
	N *= softness;
	vec3 deform = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + N).xyz;
	diffuse *= A;
	fragData = vec4(deform, 1.0) + diffuse * u_alpha;

	if(depth < 0.9999)
		return;

	fragData *= mix(vec4(1.0), vec4(softness), u_mask.xxxy);
    fragData.w = 1.0;
}