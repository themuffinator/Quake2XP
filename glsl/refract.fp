varying float			v_depth;
varying float			v_depthS;
varying vec2			v_deformMul;
varying vec2			v_deformTexCoord;

uniform float			u_thickness;
uniform float			u_thickness2;
uniform float			u_alpha;
uniform float			u_ambientScale;

uniform vec2			u_viewport;
uniform vec2			u_mask;
uniform sampler2D		u_deformMap;
uniform sampler2D		u_colorMap;

uniform	sampler2DRect	g_depthBufferMap;
uniform	sampler2DRect	g_colorBufferMap;
uniform sampler2DRect	g_weaponHackMap;

uniform vec2			u_depthParms;

#include depth.inc

void main (void) {

	vec2 N = texture2D(u_deformMap, v_deformTexCoord).xy * 2.0 - 1.0;
	vec4 diffuse  = texture2D(u_colorMap,  v_deformTexCoord.xy);

	// Z-feather
	float depth = DecodeDepth(texture2DRect(g_depthBufferMap, gl_FragCoord.xy).x, u_depthParms);
	N *= clamp((depth - v_depth) / u_thickness, 0.0, 1.0);
	// scale by the deform multiplier and the viewport size
	N *= v_deformMul * u_viewport.xy;
	
	#ifdef ALPHAMASK
	float A = texture2D(u_deformMap, v_deformTexCoord).a;
	float softness = clamp((depth - v_depthS) / u_thickness2, 0.0, 1.0);
	// refracted sprites with soft edges
	if (A == 0.004) {
		discard;
		return;
	}
	N *= A;
	N *= softness;
	vec3 deform = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + N).xyz;
	diffuse *= A;
	gl_FragColor = vec4(deform, 1.0) + diffuse;
	gl_FragColor *= mix(vec4(1.0), vec4(softness), u_mask.xxxy);

	#else
	// world refracted surfaces
	// chromatic aberration approximation

	gl_FragColor.r = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + N * 0.85).r;
	gl_FragColor.g = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + N * 1.00).g;
	gl_FragColor.b = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + N * 1.15).b;
	// blend glass texture
//	diffuse *= u_ambientScale;
	gl_FragColor.xyz += diffuse.xyz * u_alpha;

	#endif
}
