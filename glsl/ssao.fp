#include depth.inc

uniform	sampler2DRect		u_depthBufferMiniMap;

uniform vec2			u_depthParms;
uniform vec2			u_ssaoParms;	// intensity, scale
uniform vec2			u_viewport;


#define HQ

#ifdef HQ
#define HQ_SCALE		0.5
#endif

#define FARCLIP			4096.0		// Q2 specific

vec3 RandomNormal (const in vec2 p) {
	// We need irrationals for pseudo randomness.
	// Most (all?) known transcendental numbers will (generally) work.
	const vec2 v = vec2(
		23.1406926327792690,	// e^pi (Gelfond's constant)
		2.6651441426902251);	// 2^sqrt(2) (Gelfond-Schneider constant)
	float r = mod(123456789., 1e-7 + 256. * dot(p, v));

	vec3 n;
	n.x = sin(r);
	n.y = cos(r);
	n.z = sqrt(1.0 - dot(n.xy, n.xy));

	return n;
}

void main (void) {
	// define kernel
	const float step = 1.0 - 1.0 / 8.0;
	const float fScale = 0.025; 
	float n = 0.0;

	vec3 kernel[8];
	kernel[0] = normalize(vec3( 1, 1, 1))*fScale*(n+=step);
	kernel[1] = normalize(vec3(-1,-1,-1))*fScale*(n+=step);
	kernel[2] = normalize(vec3(-1,-1, 1))*fScale*(n+=step);
	kernel[3] = normalize(vec3(-1, 1,-1))*fScale*(n+=step);
	kernel[4] = normalize(vec3(-1, 1 ,1))*fScale*(n+=step);
	kernel[5] = normalize(vec3( 1,-1,-1))*fScale*(n+=step);
	kernel[6] = normalize(vec3( 1,-1, 1))*fScale*(n+=step);
	kernel[7] = normalize(vec3( 1, 1,-1))*fScale*(n+=step);

	// create random rotation matrix
	vec3 randomNormal = RandomNormal(mod(gl_FragCoord.xy, vec2(16.0)));

	// get fragment depth
	float centerDepth = DecodeDepth(texture2DRect(u_depthBufferMiniMap, gl_FragCoord.xy).x, u_depthParms);

	// compute sample scale
	vec3 scale = vec3(u_ssaoParms.y *
		min(centerDepth / 212.0, 1.0) *		// make area smaller if distance less than 5 meters
		(1.0 + centerDepth / 320.0));		// make area bigger if distance more than 32 meters

	float depthRangeScale = FARCLIP / scale.z * 0.85;

	// convert from inches to screen-space
	scale.xy *= 1.0 / centerDepth;
	scale.z *= 2.0;
	scale.xy *= u_viewport;

	float depthTestSoftness = 64.0 * FARCLIP / scale.z;

	vec3 bias;
	vec4 depths[2];
	vec4 sum = vec4(0.0);

	for (int i = 0; i < 2; i++) {
		bias = reflect(kernel[i*4+0], randomNormal) * scale;

		depths[0].x = DecodeDepth(texture2DRect(u_depthBufferMiniMap, gl_FragCoord.xy + bias.xy).x, u_depthParms) + bias.z;
		#ifdef HQ
			bias *= HQ_SCALE;
			depths[1].x = DecodeDepth(texture2DRect(u_depthBufferMiniMap, gl_FragCoord.xy + bias.xy).x, u_depthParms) + bias.z;
		#endif

		bias = reflect(kernel[i*4+1], randomNormal) * scale;
		depths[0].y = DecodeDepth(texture2DRect(u_depthBufferMiniMap, gl_FragCoord.xy + bias.xy).x, u_depthParms) + bias.z;
		#ifdef HQ
			bias *= HQ_SCALE;
			depths[1].y = DecodeDepth(texture2DRect(u_depthBufferMiniMap, gl_FragCoord.xy + bias.xy).x, u_depthParms) + bias.z;
		#endif

		bias = reflect(kernel[i*4+2], randomNormal) * scale;
		depths[0].z = DecodeDepth(texture2DRect(u_depthBufferMiniMap, gl_FragCoord.xy + bias.xy).x, u_depthParms) + bias.z;
		#ifdef HQ
			bias *= HQ_SCALE;
			depths[1].z = DecodeDepth(texture2DRect(u_depthBufferMiniMap, gl_FragCoord.xy + bias.xy).x, u_depthParms) + bias.z;
		#endif

		bias = reflect(kernel[i*4+3], randomNormal) * scale;
		depths[0].w = DecodeDepth(texture2DRect(u_depthBufferMiniMap, gl_FragCoord.xy + bias.xy).x, u_depthParms) + bias.z;
		#ifdef HQ
			bias *= HQ_SCALE;
			depths[1].w = DecodeDepth(texture2DRect(u_depthBufferMiniMap, gl_FragCoord.xy + bias.xy).x, u_depthParms) + bias.z;
		#endif

	#ifdef HQ
		for (int j = 0; j < 2; j++) {
	#else
		for (int j = 0; j < 1; j++) {
	#endif
			vec4 diff = (vec4(centerDepth) - depths[j]) * (1.0 / FARCLIP);
			vec4 diffScaled = diff * depthRangeScale;
			vec4 rangeIsInvalid = (min(abs(diffScaled), 1.0) + clamp(diffScaled, 0.0, 1.0)) * 0.5;

			sum += mix(clamp(-diff * depthTestSoftness, 0.0, 1.0), vec4(0.55), rangeIsInvalid);
		}
	}

	#ifdef HQ
		float occ = dot(sum, vec4((1.0 / 16.0) * 2.0)) - 0.075;
	#else
		float occ = dot(sum, vec4((1.0 / 8.0) * 2.0)) - 0.075;
	#endif

	gl_FragColor = vec4(min(mix(1.0, occ, u_ssaoParms.x), 1.0));
}
