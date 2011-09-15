varying float			v_depth;
varying float			v_depthS;
varying vec2			v_deformMul;
varying vec2			v_deformTexCoord;
varying	vec4			v_color;

uniform float			u_thickness;
uniform float			u_thickness2;
uniform float			u_alpha;
uniform vec2			u_viewport;
uniform vec2			u_mask;
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
	vec3 deform = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + N).xyz;
	diffuse *= v_color;
	gl_FragColor.xyz = deform * (1.0 - u_alpha) + diffuse.xyz * u_alpha;

	#endif
}
