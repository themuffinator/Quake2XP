layout (binding = 0) uniform sampler2D		u_colorMap;
layout (binding = 1) uniform sampler2D		u_dstMap;
layout (binding = 2) uniform sampler2DRect	g_colorBufferMap;
layout (binding = 3) uniform sampler2DRect	g_depthBufferMap;

layout(location = U_WATER_DEFORM_MUL)	uniform float	u_deformMul;		// for normal w/o depth falloff
layout(location = U_WATHER_THICKNESS)	uniform float	u_thickness;
layout(location = U_WATER_ALPHA)		uniform float	u_alpha;
layout(location = U_COLOR_MUL)			uniform float	u_ColorModulate;
layout(location = U_AMBIENT_LEVEL)		uniform float	u_ambientScale;
layout(location = U_SCREEN_SIZE)		uniform vec2	u_viewport;
layout(location = U_DEPTH_PARAMS)		uniform vec2	u_depthParms;
layout(location = U_WATER_TRANS)		uniform int		u_TRANS;
layout(location = U_PROJ_MATRIX)		uniform mat4	u_projectionMatrix;
layout(location = U_WATER_MIRROR)		uniform int		u_mirror;

in vec2		v_deformTexCoord;
in vec2		v_diffuseTexCoord;
in vec2		v_deformMul;
in vec3		v_positionVS;
in mat3		v_tangentToView;
in vec4		v_color;
in vec3     v_lightVec;
in vec3		v_viewVecTS;

#define MAX_STEPS			20
#define MAX_STEPS_BINARY	10

#define STEP_SIZE			10.0
#define STEP_SIZE_MUL		1.35

#define Z_THRESHOLD			1.0			// sufficient difference to stop tracing

#define	FOREGROUND_FALLOFF	0.25
#define SCREEN_FALLOFF		6.0			// fall-off to screen edge, the higher the sharper

#define	FRESNEL_MUL			1.0
#define FRESNEL_EXP			1.6

#define NORMAL_MUL			-0.02

#define OPAQUE_OFFSET		4.0
#define OPAQUE_MUL			(-1.0 / 512.0)


#include depth.inc
#include lighting.inc

//
// view space to viewport
//
vec2 VS2UV (const in vec3 p) {
	vec4 v = u_projectionMatrix * vec4(p, 1.0);
	return (v.xy / v.w * 0.5 + 0.5) * u_viewport; 
}

void main (void) {

	vec3 V = normalize(v_viewVecTS);
	vec3 L = normalize(v_lightVec);

	// load diffuse map with offset
	vec3 offset = normalize(texture(u_dstMap, v_deformTexCoord.xy).rgb * 2.0 - 1.0); // use scaled tex coord
//	float noise = texture(u_dstMap, v_deformTexCoord.xy).a;

	vec3 diffuse;
	if (u_TRANS == 1){
	diffuse.r = texture(u_colorMap, v_diffuseTexCoord.xy + offset.xy * 0.85).r * u_ambientScale; 
	diffuse.g = texture(u_colorMap, v_diffuseTexCoord.xy + offset.xy * 1.0).g * u_ambientScale;
	diffuse.b = texture(u_colorMap, v_diffuseTexCoord.xy + offset.xy * 1.15).b * u_ambientScale;	
//	diffuse *= noise;
	diffuse *= v_color.a;
	}

	if (u_TRANS == 0){
	diffuse.r = texture(u_colorMap, v_diffuseTexCoord.xy + offset.xy * 0.425).r * u_ambientScale; 
	diffuse.g = texture(u_colorMap, v_diffuseTexCoord.xy + offset.xy * 0.5).g * u_ambientScale;
	diffuse.b = texture(u_colorMap, v_diffuseTexCoord.xy + offset.xy * 0.575).b * u_ambientScale;	
	diffuse *= 0.5;
	}

	vec3 N = vec3(0.0, 0.0, 1.0);  // shutup compiler
	vec2 tc;
	float sceneDepth;

	vec2 Es = PhongLighting(offset, L, V, 6.0);
	vec3 lighting = Es.x * diffuse + Es.y * diffuse.rrr;

	if (u_TRANS == 1) {
		sceneDepth = DecodeDepth(texture2DRect(g_depthBufferMap, gl_FragCoord.xy).x, u_depthParms);
		N.xy = offset.xy * clamp((sceneDepth + v_positionVS.z) / u_thickness, 0.0, 1.0);

		// scale by the deform multiplier & viewport size
		tc = N.xy * v_deformMul * u_viewport;

		// chromatic aberration approximation
		vec3 refractColor;
		refractColor.r = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + tc.xy * 0.85).r;
		refractColor.g = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + tc.xy * 1.0).g;
		refractColor.b = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + tc.xy * 1.15).b;

		// blend water texture
		fragData = vec4(mix(refractColor + lighting, diffuse, v_color.a), 1.0);
	}
	
	if (u_TRANS != 1) {
		// non-transparent
		N.xy = offset.xy;
		fragData = vec4(diffuse + lighting, 1.0);
	}
 
	if (u_mirror == 0)
		return;

	//
	// screen-space local reflections
	//

	N.xy *= u_deformMul * NORMAL_MUL;
	N.z = 1.0;

	N = normalize(v_tangentToView * N);

	V = normalize(v_positionVS);
	vec3 R = reflect(V, N);

	// Fresnel
	float scale = FRESNEL_MUL * pow(1.0 - abs(dot(V, N)), FRESNEL_EXP);

	// ignore invisible & facing into the camera reflections
	if (scale < 0.05 || dot(R, V) < 0.0)
		return;

	// follow the reflected ray in increasing steps
	vec3 rayPos = v_positionVS;

	// FIXME: only temporary solution to self-intersection issue
	// NOTE: offset must be done using undistorted surface normal,
	// but since the distortion is weak, this works too
	if (u_TRANS == 0)
		rayPos += N * v_positionVS.z * OPAQUE_MUL * OPAQUE_OFFSET;

	float stepSize = STEP_SIZE;
	int i;

	for (i = 0; i < MAX_STEPS; i++, stepSize *= STEP_SIZE_MUL) {
		rayPos += R * stepSize;

		tc = VS2UV(rayPos).xy;
		sceneDepth = DecodeDepth(texture2DRect(g_depthBufferMap, tc).x, u_depthParms);

		if (sceneDepth <= -rayPos.z)
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
	rayPos -= R * stepSize;
	tc = VS2UV(rayPos).xy;
	sceneDepth = DecodeDepth(texture2DRect(g_depthBufferMap, tc).x, u_depthParms);

	for (int j = 0; j < MAX_STEPS_BINARY; j++, stepSize *= 0.5) {
		rayPos += R * stepSize * (step(-rayPos.z, sceneDepth) - 0.5);

		tc = VS2UV(rayPos).xy;
		sceneDepth = DecodeDepth(texture2DRect(g_depthBufferMap, tc).x, u_depthParms);

		float delta = -rayPos.z - sceneDepth;

		if (abs(delta) < Z_THRESHOLD)
			break;	// found it
	}

	// fade depending on the ray length
	float f = float(i) / float(MAX_STEPS);
	f *= f;
	scale *= 1.0 - f * f;
	
	// fade on screen border
	// avoids abrupt reflection disappearing
	vec2 a = tc / u_viewport;
	if (a.x > 0.5) a.x = 1.0 - a.x;
	if (a.y > 0.5) a.y = 1.0 - a.y;
		
	scale *= clamp(a.x * SCREEN_FALLOFF, 0.0, 1.0);
	scale *= clamp(a.y * SCREEN_FALLOFF, 0.0, 1.0);

	// avoid reflection of objects in foreground 
	// TODO: upgrade
	scale /= 1.0 + abs(sceneDepth + rayPos.z) * FOREGROUND_FALLOFF;

	// combine
	fragData.xyz = mix(fragData.xyz, texture2DRect(g_colorBufferMap, tc).xyz, scale);
	fragData.w = 1.0;
}
