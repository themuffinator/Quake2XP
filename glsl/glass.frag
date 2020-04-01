/*
layout (binding = 0) uniform sampler2D		u_deformMap;
layout (binding = 1) uniform sampler2D		u_colorMap;
layout (binding = 2) uniform sampler2DRect	g_colorBufferMap;
layout (binding = 3) uniform sampler2DRect	g_depthBufferMap;
*/

layout (bindless_sampler, location  = U_TMU0) uniform sampler2D		u_deformMap;
layout (bindless_sampler, location  = U_TMU1) uniform sampler2D		u_colorMap;
layout (bindless_sampler, location  = U_TMU2) uniform sampler2DRect	g_colorBufferMap;
layout (bindless_sampler, location  = U_TMU3) uniform sampler2DRect	g_depthBufferMap;

layout(location = U_REFR_ALPHA)			uniform float	u_alpha;
layout(location = U_REFR_THICKNESS0)	uniform float	u_thickness0; //depth feather
layout(location = U_REFR_THICKNESS1)	uniform float	u_thickness1; //sprite softeness
layout(location = U_SCREEN_SIZE)		uniform vec2	u_viewport;
layout(location = U_DEPTH_PARAMS)		uniform vec2	u_depthParms;
layout(location = U_COLOR_MUL)			uniform float	u_ambientScale;
layout(location = U_REFR_MASK)			uniform vec2	u_mask;			//softeness
layout(location = U_REFR_ALPHA_MASK)	uniform int		u_ALPHAMASK;	//is sprite
layout(location = U_PARAM_FLOAT_0)		uniform float	u_blurScale;


in float	v_depth;
in float	v_depthS;
in vec2		v_deformMul;
in vec2		v_deformTexCoord;

#include depth.inc


vec4 boxBlur(sampler2DRect blurTex, float  blurSamples, vec2 N){

	float numSamples = (1.0 / (blurSamples * 4.0 + 1.0));
	vec4 sum = texture2DRect( blurTex, gl_FragCoord.xy + N);	// central point

	for ( float i = 1.0; i <= blurSamples; i += 1.0 ){

		sum += texture2DRect(blurTex, gl_FragCoord.xy + N + vec2(i, 0.0));
		sum += texture2DRect(blurTex, gl_FragCoord.xy + N + vec2(-i, 0.0));
		sum += texture2DRect(blurTex, gl_FragCoord.xy + N + vec2(0.0, i));
		sum += texture2DRect(blurTex, gl_FragCoord.xy + N + vec2(0.0, -i));
	}
	return sum * numSamples;
}

void main (void) {

	vec2 N = texture(u_deformMap, v_deformTexCoord).xy * 2.0 - 1.0;
	vec4 diffuse  = texture(u_colorMap,  v_deformTexCoord.xy);

	// Z-feather
	float depth = DecodeDepth(texture2DRect(g_depthBufferMap, gl_FragCoord.xy).x, u_depthParms);
	N *= clamp((depth - v_depth) / u_thickness0, 0.0, 1.0);
	// scale by the deform multiplier and the viewport size
	N *= v_deformMul * u_viewport.xy;
	
	// world refracted surfaces
	// chromatic aberration approximation
    vec4 clearGlass;
    clearGlass.r = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + N * 0.85).r;
	clearGlass.g = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + N * 1.00).g;
	clearGlass.b = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + N * 1.15).b;

    vec4 bluredGlass = boxBlur(g_colorBufferMap, max(8.0, diffuse.a * u_blurScale), N);

    fragData = mix (bluredGlass, clearGlass, diffuse.a);

    // blend glass texture
	diffuse.rgb *= u_ambientScale;
	fragData.xyz += diffuse.xyz * u_alpha;

    fragData.w = 1.0;
}