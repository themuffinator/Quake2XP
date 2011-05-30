varying float			v_depth;
varying float			v_depthS;
varying vec2			v_deformMul;
varying vec2			v_deformTexCoord;

uniform float			u_thickness;
uniform float			u_thickness2;
uniform float			u_alpha;
uniform vec2			u_mask;
uniform vec2			u_viewport;

uniform sampler2D		u_deformMap;
uniform sampler2D		u_colorMap;

uniform	sampler2DRect	g_depthBufferMap;
uniform	sampler2DRect	g_colorBufferMap;
uniform vec2			u_depthParms;

float DecodeDepth (const float d, const in vec2 parms) {
	return parms.x / (parms.y - d);
}

void main (void) {

	vec2 N = texture2D(u_deformMap, v_deformTexCoord).xy * 2.0 - 1.0;
	vec4 diffuse  = texture2D(u_colorMap,  v_deformTexCoord.xy);

	// Z-feather
	float depth = DecodeDepth(texture2DRect(g_depthBufferMap, gl_FragCoord.xy).x, u_depthParms);
	N *= clamp((depth - v_depth) / u_thickness, 0.0, 1.0);
	float softness = clamp((depth - v_depthS) / u_thickness2, 0.0, 1.0);

	// scale by the deform multiplier and the viewport size
	N *= v_deformMul * u_viewport.xy;

	// output the refracted color
	#ifdef ALPHAMASK
	// apply the alpha mask
	float A = texture2D(u_deformMap, v_deformTexCoord).a;

	if (A == 0.004) {
		discard;
		return;
	}
	N *= A;

	vec3 deform = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + N).xyz;
	gl_FragColor.xyz = deform;
	
	#else

	vec3 deform = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + N).xyz;

	diffuse *= gl_Color;
	gl_FragColor.xyz = deform * (1.0 - u_alpha) + diffuse.xyz * u_alpha;
	gl_FragColor *= mix(vec4(1.0), vec4(softness), u_mask.xxxy);
	#endif
}
