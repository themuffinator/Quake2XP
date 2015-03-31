varying vec2		v_texCoord;

uniform	sampler2D	g_colorMiniMap;
uniform vec2		u_blurOffset;

uniform	sampler2D	g_depthMiniMap;
uniform vec2		g_depthParms;

void main(void) {
	vec2 centerTC = gl_TexCoord[0].st;

	// low weight center sample, will be used on edges
	float sum = 0.0125;
	vec4 color = texture2D(g_colorMiniMap, centerTC) * sum;

	float centerDepth = DecodeDepth(texture2D(g_depthMiniMap, centerTC).r);

	const vec2 arrOffsets[4] = {
		vec2( 1.0,-1.0),
		vec2(-1.0,-1.0),
		vec2( 1.0, 1.0),
		vec2(-1.0, 1.0)
	};

	for (int i = 0; i < 4; i++) {
		vec2 sampleTC = centerTC + u_blurOffset.xy * arrOffsets[i];

		float depth = DecodeDepth(texture2D(g_depthMiniMap, sampleTC).r);

		float diff = 8.0 * (1.0 - depth / centerDepth);
		float w = max(0.5 - 0.75 * abs(diff) - 0.25 * diff, 0.0);

		color += texture2D(g_colorMiniMap, sampleTC) * w;
		sum += w;
	}

	gl_FragColor = color / sum;
}
