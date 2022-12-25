//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0) uniform sampler2D		u_colorMap;
layout (bindless_sampler, location  = U_TMU1) uniform sampler2DRect	u_depthBufferMap;
layout (bindless_sampler, location  = U_TMU2) uniform sampler2DRect	u_colorBufferMap;
layout (bindless_sampler, location  = U_TMU3) uniform sampler2D		u_deformMap;

layout(location = U_PARTICLE_MASK)		uniform vec2	u_mask;
layout(location = U_PARTICLE_THICKNESS)	uniform float	u_thickness;
layout(location = U_COLOR_MUL)			uniform float	u_colorScale;
layout(location = U_SCREEN_SIZE)		uniform vec2	u_viewport;
layout(location = U_PARAM_INT_0)		uniform int		u_distort;

in float		v_depthS;
in float		v_depth;
in vec4			v_color;
in vec4			v_texCoord0;
in vec2			v_deformMul;

//#include depth.inc //!#include "include/depth.inc"

void main (void) {
	vec4 color = texture(u_colorMap, v_texCoord0.st);

	if(u_thickness > 0.0){
	// Z-feather
		float depth = texture(u_depthBufferMap, gl_FragCoord.xy).x;
		float softness = clamp((depth - v_depthS) / u_thickness, 0.0, 1.0);

		if(u_distort == 1){
			//deform
			vec2 N = texture(u_deformMap, v_texCoord0.st).xy * 2.0 - 1.0;
			vec4 normal = texture(u_deformMap, v_texCoord0.st);
			float A = texture(u_deformMap, v_texCoord0.st).a;

			N *= clamp((depth - v_depth) / 0.01, 0.0, 1.0);
			N *= v_deformMul * u_viewport.xy;
	
			if (A <= 0.01) {
				discard;
				return;
			}
			N *= A;
			//N *= color.a;
			N *= softness;

			vec3 deform = texture(u_colorBufferMap, gl_FragCoord.xy + N).xyz;
			fragData = vec4(deform * color.a, 1.0) + color * v_color * u_colorScale;;
			return;
		}

		fragData = color * v_color * u_colorScale;
	
		if(depth < 0.9999)
			return;

		fragData *= mix(vec4(1.0), vec4(softness), u_mask.xxxy);
	}
	else
		fragData = color * v_color;
}
