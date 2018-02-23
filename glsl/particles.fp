layout (binding = 0) uniform sampler2D		u_map0;
layout (binding = 1) uniform sampler2DRect	u_depthBufferMap;

in float		v_depth;
in vec4			v_color;
in vec4			v_texCoord0;

uniform vec2			u_depthParms;
uniform vec2			u_mask;
uniform float			u_thickness;
uniform float			u_colorScale;

#include depth.inc

void main (void) {
	vec4 color = texture(u_map0, v_texCoord0.st);
	
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
