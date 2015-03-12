#extension GL_ARB_texture_rectangle : enable

#include depth.inc

#define MAX_STEPS			40
#define MAX_STEPS_BACK		20

#define STEP_SIZE			4.0
#define STEP_SIZE_MUL		1.19

#define DEPTH_THRESHOLD		1.0			// sufficient difference to stop tracing

#define	FOREGROUND_FALLOFF	0.25
#define SCREEN_FALLOFF		6.0			// fall-off to screen edge, the higher the sharper

#define	FRESNEL_MUL			1.0
#define FRESNEL_EXP			1.6

#define NORMAL_MUL			-0.02

varying vec2				v_deformTexCoord;
varying vec2				v_diffuseTexCoord;
varying vec2				v_deformMul;
varying vec3				v_positionVS;
varying mat3				v_tangentToView;
varying vec4				v_color;

uniform float				u_deformMul;		// for normal w/o depth falloff
uniform float				u_thickness;
uniform float				u_alpha;
uniform float				u_ColorModulate;
uniform float				u_ambientScale;
uniform vec2				u_viewport;
uniform vec2				u_depthParms;
uniform int					u_TRANS;

uniform sampler2D			u_colorMap;
uniform sampler2D			u_dstMap;
uniform	sampler2DRect		g_depthBufferMap;
uniform	sampler2DRect		g_colorBufferMap;

//
// view space to viewport
//
vec2 VS2UV (const in vec3 p) {
	vec4 v = gl_ProjectionMatrix * vec4(p, 1.0);
	return (v.xy / v.w * 0.5 + 0.5) * u_viewport; 
}

void main (void) {
	// load diffuse map with offset
	vec4 offset = texture2D(u_dstMap, v_deformTexCoord.xy);
	vec3 diffuse = texture2D(u_colorMap, v_diffuseTexCoord.xy + offset.zw).xyz * u_ambientScale;  
	vec3 N;
	vec2 tc;

	// scale by the deform multiplier
	if(u_TRANS == 1){
		float depth = DecodeDepth(texture2DRect(g_depthBufferMap, gl_FragCoord.xy).x, u_depthParms);
		N.xy = offset.xy * clamp((depth + v_positionVS.z) / u_thickness, 0.0, 1.0);
	}

	if(u_TRANS != 1)
		N.xy = offset.xy;

		// scale by the viewport size
		tc = N.xy * v_deformMul * u_viewport;
	
	if(u_TRANS == 1){
		// chromatic aberration approximation
		vec3 refractColor;
		refractColor.x = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + tc.xy * 0.85).x;
		refractColor.y = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + tc.xy * 1.0).y;
		refractColor.z = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + tc.xy * 1.15).z;

		// blend water texture
		gl_FragColor = vec4(mix(refractColor, diffuse, v_color.a), 1.0);
	}
	
	if(u_TRANS != 1){
		// non-transparent
		gl_FragColor = vec4(diffuse, 1.0);
	//	return;
	}

	//
	// screen-space local reflections
	//

	N.xy *= u_deformMul * NORMAL_MUL;
	N.z = 1.0;

	N = normalize(v_tangentToView * N);

	vec3 V = normalize(v_positionVS);
	vec3 R = reflect(V, N);

	// Fresnel
	float scale = FRESNEL_MUL * pow(1.0 - abs(dot(V, N)), FRESNEL_EXP);

	// ignore invisible & facing into the camera reflections
	if (scale < 0.05 || dot(R, V) < 0.0)
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

	//
	// if something is hit, use binary search to enhance precision
	//

	// go back to the middle
	stepSize *= 0.5;
	currentPos -= R * stepSize;

	tc = VS2UV(currentPos).xy;
	sceneDepth = DecodeDepth(texture2DRect(g_depthBufferMap, tc).x, u_depthParms);

	for (int j = 0; j < MAX_STEPS_BACK; j++, stepSize *= 0.5) {
		currentPos += R * stepSize * (step(-currentPos.z, sceneDepth) - 0.5);

		tc = VS2UV(currentPos).xy;
		sceneDepth = DecodeDepth(texture2DRect(g_depthBufferMap, tc).x, u_depthParms);

#if true
//		float delta = sceneDepth + currentPos.z;
		float delta = -currentPos.z - sceneDepth;

		if (delta > 0.0 && delta < DEPTH_THRESHOLD)
			break;	// found it
//		if (abs(delta) < DEPTH_THRESHOLD)
//			break;	// found it
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
	// TODO: upgrade
	scale /= 1.0 + abs(sceneDepth + currentPos.z) * FOREGROUND_FALLOFF;
//	scale /= 1.0 + max(sceneDepth + currentPos.z, 0.0) * FOREGROUND_FALLOFF;
		
	// combine
	gl_FragColor.xyz = mix(gl_FragColor.xyz, texture2DRect(g_colorBufferMap, tc).xyz, scale);
}
