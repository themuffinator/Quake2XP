#extension GL_ARB_texture_rectangle : enable

#include depth.inc

#define MAX_STEPS			40
#define MAX_STEPS_BACK		20

#define STEP_SIZE			4.0
#define STEP_SIZE_MUL		1.15

#define DEPTH_THRESHOLD		1.0			// sufficient difference to stop tracing

#define	FOREGROUND_FALLOFF	0.25
#define SCREEN_FALLOFF		6.0			// fall-off to screen edge, the higher the sharper

#define	FRESNEL_MUL			1.0
#define FRESNEL_EXP			1.6

varying vec2				v_deformTexCoord;
varying vec2				v_diffuseTexCoord;
varying vec2				v_deformMul;
varying vec4				v_color;
varying vec3				v_positionVS;
varying vec3				v_normalVS;

uniform float				u_thickness;
uniform float				u_alpha;
uniform float				u_ColorModulate;
uniform float				u_ambientScale;
uniform vec2				u_viewport;
uniform vec2				u_depthParms;

uniform sampler2D			u_colorMap;
uniform sampler2D			u_dstMap;
uniform	sampler2DRect		g_depthBufferMap;
uniform	sampler2DRect		g_colorBufferMap;

vec2 VS2UV (const in vec3 p) {
	vec4 v = gl_ProjectionMatrix * vec4(p, 1.0);
	return (v.xy / v.w * 0.5 + 0.5) * u_viewport; 
}

void main (void) {
	// load diffuse map with offset
	vec4 offset = texture2D(u_dstMap, v_deformTexCoord.xy);
	vec3 diffuse = texture2D(u_colorMap, v_diffuseTexCoord.xy + offset.zw).xyz;// * u_ambientScale;  
	vec2 N = vec2(0.0, 0.0);
	
	#ifdef TRANS
		//
		// Z-feather
		// negative view-space Z is the fragment depth
		//

		float depth = DecodeDepth(texture2DRect(g_depthBufferMap, gl_FragCoord.xy).x, u_depthParms);
		N = offset.xy * clamp((depth + v_positionVS.z) / u_thickness, 0.0, 1.0);

		// scale by the deform multiplier & the viewport size
		N *= v_deformMul * u_viewport.xy;
	
		// chromatic aberration approximation coefficients
		vec3 refractColor;
		refractColor.x = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + N * 0.85).x;
		refractColor.y = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + N * 1.0).y;
		refractColor.z = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + N * 1.15).z;

		// blend water texture
		gl_FragColor = vec4(mix(refractColor, diffuse, v_color.a), 1.0);
	#else
		// no refraction
		vec3 color = texture2DRect(g_colorBufferMap, gl_FragCoord.xy).xyz;
		gl_FragColor = vec4(mix(color, diffuse, v_color.a), 1.0);

		// FIXME: use regular GL blend
//		gl_FragColor = vec4(diffuse, 1.0);
	#endif

	//
	// screen-space local reflections
	// TODO: use normal map
	//

	vec3 normal = normalize(v_normalVS);
	vec3 viewDir = normalize(v_positionVS);
	vec3 R = reflect(viewDir, normal);

	// Fresnel
	float scale = FRESNEL_MUL * pow(1.0 - abs(dot(viewDir, normal)), FRESNEL_EXP);
//	float scale = min(0.0 + 2.8 * pow(1.0 + dot(viewDir, normal), 2.0), 1.0);

	// ignore invisible & facing into the camera reflections
	if (scale < 0.05 || dot(R, viewDir) < 0.0)
		return;

	vec3 currentPos = v_positionVS;
	float sceneDepth;

	// follow the reflected ray in increasing steps
	float stepSize = STEP_SIZE;
	int i;

	for (i = 0; i < MAX_STEPS; i++, stepSize *= STEP_SIZE_MUL) {
		currentPos += R * stepSize;
		sceneDepth = DecodeDepth(texture2DRect(g_depthBufferMap, VS2UV(currentPos).xy).x, u_depthParms);

		if (sceneDepth <= -currentPos.z)
			break;	// intersection
	}

	if (i == MAX_STEPS)
		return;

	// TODO: make more compact

	// if something is hit, use binary search to enhance precision
	// go back to the middle
	stepSize *= 0.5;
	currentPos -= R * stepSize;

	vec2 tc = VS2UV(currentPos).xy;
	sceneDepth = DecodeDepth(texture2DRect(g_depthBufferMap, tc).x, u_depthParms);

	for (int j = 0; j < MAX_STEPS_BACK; j++, stepSize *= 0.5) {
		currentPos += R * stepSize * (step(-currentPos.z, sceneDepth) - 0.5);

		tc = VS2UV(currentPos).xy;
		sceneDepth = DecodeDepth(texture2DRect(g_depthBufferMap, tc).x, u_depthParms);

#if true
		float delta = -currentPos.z - sceneDepth;

		if (delta > 0.0 && delta < DEPTH_THRESHOLD)
			break;	// found it
#endif
	}

	// fade depending on the ray length
	scale *= 1.0 - float(i) / float(MAX_STEPS);
		
	// fade on screen border
	// avoids abrupt reflection disappearing
	vec2 a = tc / u_viewport;
	if (a.x > 0.5) a.x = 1.0 - a.x;
	if (a.y > 0.5) a.y = 1.0 - a.y;
		
	scale *= clamp(a.x * SCREEN_FALLOFF, 0.0, 1.0);
	scale *= clamp(a.y * SCREEN_FALLOFF, 0.0, 1.0);
		
	// avoid reflection of objects in foreground 
	// TODO: upgrade as it bugs
	scale /= 1.0 + abs(sceneDepth + currentPos.z) * FOREGROUND_FALLOFF;
//	scale /= 1.0 + max(sceneDepth + currentPos.z, 0.0) * FOREGROUND_FALLOFF;
		
	// combine
	gl_FragColor.xyz = mix(gl_FragColor.xyz, texture2DRect(g_colorBufferMap, tc + N).xyz, scale);
}
