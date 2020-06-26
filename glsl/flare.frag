//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0) uniform sampler2D	u_map;
layout (bindless_sampler, location  = U_TMU1) uniform sampler2DRect	u_depthBufferMap;


layout (location = U_COLOR_MUL)			uniform float	u_colorMul;
layout(location = U_DEPTH_PARAMS)		uniform vec2	u_depthParms;
layout(location = U_PARAM_VEC2_0)		uniform vec2	u_mask;
layout(location = U_PARAM_FLOAT_0)		uniform float	u_thickness;

in float	v_depth;
in vec2		v_texCoord; 
in vec4		v_color;

#include depth.inc //!#include "include/depth.inc"

void main(void) 
{
	vec4 tex = texture(u_map, v_texCoord.st);
	
  // Z-feather
	float depth = DecodeDepth(texture2DRect(u_depthBufferMap, gl_FragCoord.xy).x, u_depthParms);
	float softness = clamp((depth - v_depth) / u_thickness, 0.0, 1.0);
	
	fragData = tex * v_color * u_colorMul;
	fragData *= mix(vec4(1.0), vec4(softness), u_mask.xxxy);
}
