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
layout(location = U_PARAM_INT_1)		uniform int		u_scaledNormal;
layout(location = U_SCROLL)				uniform float	u_scroll;

in float		v_depthS;
in float		v_depth;
in vec4			v_color;
in vec4			v_texCoord0;
in vec2			v_deformMul;

void main (void) {
	vec4 color = texture(u_colorMap, v_texCoord0.st);

	if(u_thickness > 0.0){
	// Z-feather
		float depth = texture(u_depthBufferMap, gl_FragCoord.xy).x;
		float softness = clamp((depth - v_depthS) / u_thickness, 0.0, 1.0);

		if(u_distort == 1){
			//deform
			vec2 scaledTC = v_texCoord0.st;
			if (u_scaledNormal == 1)
				scaledTC *=vec2(2.0, 8.0)+ vec2(u_scroll, 0.0);

			vec2 N = texture(u_deformMap, scaledTC.st).xy * 2.0 - 1.0;
			float A = texture(u_deformMap, v_texCoord0.st).a;

			N *= clamp((depth - v_depth) / 1.0, 0.0, 1.0);
			N *= v_deformMul * u_viewport.xy;

			N *= A;
			N *= softness;

			vec3 deform;
			deform.r = texture(u_colorBufferMap, gl_FragCoord.xy + N * 0.85).r;
			deform.g = texture(u_colorBufferMap, gl_FragCoord.xy + N * 1.00).g;
			deform.b = texture(u_colorBufferMap, gl_FragCoord.xy + N * 1.15).b;
  
			fragData = vec4(deform, A) + color * v_color * u_colorScale;
			fragData.a = 1.0;
		}else
			fragData = color * v_color * u_colorScale;
	
		if(depth < 0.9999)
			return;

		fragData *= mix(vec4(1.0), vec4(softness), u_mask.xxxy);
	}
	else
		fragData = color * v_color;
}
