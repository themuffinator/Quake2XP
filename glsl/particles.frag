layout (bindless_sampler, location  = U_TMU0) uniform sampler2D		u_colorMap;
layout (bindless_sampler, location  = U_TMU1) uniform sampler2DRect	u_depthBufferMap;

layout(location = U_DEPTH_PARAMS)		uniform vec2	u_depthParms;
layout(location = U_PARTICLE_MASK)		uniform vec2	u_mask;
layout(location = U_PARTICLE_THICKNESS)	uniform float	u_thickness;
layout(location = U_COLOR_MUL)			uniform float	u_colorScale;

in float		v_depth;
in vec4			v_color;
in vec4			v_texCoord0;

#include depth.inc

void main (void) {
	vec4 color = texture(u_colorMap, v_texCoord0.st);
	
	if(u_thickness > 0.0){
	// Z-feather
	float depth = DecodeDepth(texture2DRect(u_depthBufferMap, gl_FragCoord.xy).x, u_depthParms);
	float softness = clamp((depth - v_depth) / u_thickness, 0.0, 1.0);
	
	fragData = color * v_color * u_colorScale;
	fragData *= mix(vec4(1.0), vec4(softness), u_mask.xxxy);
	}
	else
	fragData = color * v_color;
}
