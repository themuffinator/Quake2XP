layout (binding = 0) uniform sampler2D		u_deformMap;
layout (binding = 1) uniform sampler2D		u_colorMap;
layout (binding = 2) uniform sampler2DRect	g_colorBufferMap;
layout (binding = 3) uniform sampler2DRect	g_depthBufferMap;

layout (location = 4) uniform float	u_alpha;
layout (location = 5) uniform float	u_thickness;
layout (location = 6) uniform float	u_thickness2;
layout (location = 7) uniform vec2	u_viewport;
layout (location = 8) uniform vec2	u_depthParms;
layout (location = 9) uniform float	u_ambientScale;
layout (location = 10) uniform vec2	u_mask;
layout (location = 11) uniform int	u_ALPHAMASK;

in float	v_depth;
in float	v_depthS;
in vec2		v_deformMul;
in vec2		v_deformTexCoord;

#include depth.inc

void main (void) {

	vec2 N = texture(u_deformMap, v_deformTexCoord).xy * 2.0 - 1.0;
	vec4 diffuse  = texture(u_colorMap,  v_deformTexCoord.xy);

	// Z-feather
	float depth = DecodeDepth(texture2DRect(g_depthBufferMap, gl_FragCoord.xy).x, u_depthParms);
	N *= clamp((depth - v_depth) / u_thickness, 0.0, 1.0);
	// scale by the deform multiplier and the viewport size
	N *= v_deformMul * u_viewport.xy;
	
	if(u_ALPHAMASK == 1){
		float A = texture(u_deformMap, v_deformTexCoord).a;
		float softness = clamp((depth - v_depthS) / u_thickness2, 0.0, 1.0);
		// refracted sprites with soft edges
		if (A == 0.01) {
			discard;
				return;
			}
		N *= A;
		N *= softness;
		vec3 deform = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + N).xyz;
		diffuse *= A;
		fragData = (vec4(deform, 1.0) + diffuse) * A;
		fragData *= mix(vec4(1.0), vec4(softness), u_mask.xxxy);
			return;
	}

	// world refracted surfaces
	// chromatic aberration approximation

	fragData.r = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + N * 0.85).r;
	fragData.g = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + N * 1.00).g;
	fragData.b = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + N * 1.15).b;
	// blend glass texture
	diffuse *= u_ambientScale;
	fragData.xyz += diffuse.xyz * u_alpha;
  fragData.w = 1;
}
