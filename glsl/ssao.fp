uniform	sampler2DRect		u_DNMiniMap;
uniform	sampler2D		u_randomNormalMap;

uniform vec2			u_ssaoParms;	// intensity, scale
uniform vec2			u_viewport;

#define HQ

#ifdef HQ
#define HQ_SCALE		0.5
#endif

#define FARCLIP			4096.0		// Q2 specific

/*
// reconstructs normal from position
// has ugly artifacts on surface edges
void ViewSpaceNormal(const vec3 p) {
	vec3 b = normalize(ddx(p));
	vec3 t = normalize(ddy(p));

	return cross(t, b);
}
*/
/*
mat2 RandomRotation (const in vec2 p) {
	// We need irrationals for pseudo randomness.
	// Most (all?) known transcendental numbers will (generally) work.
	const vec2 v = vec2(
		23.1406926327792690,	// e^pi (Gelfond's constant)
		2.6651441426902251);	// 2^sqrt(2) (Gelfond-Schneider constant)
	float r = mod(123456789., 1e-7 + 256. * dot(p, v));
	float sinr = sin(r);
	float cosr = cos(r);
	return mat2(cosr, sinr, -sinr, cosr);
}
*/
/*
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
*/
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

/*
	vec3 kernel[8] = {
#if 1
		normalize(vec3( 1, 1, 1))*fScale*(n+=step),
		normalize(vec3(-1,-1,-1))*fScale*(n+=step),
		normalize(vec3(-1,-1, 1))*fScale*(n+=step),
		normalize(vec3(-1, 1,-1))*fScale*(n+=step),

		normalize(vec3(-1, 1 ,1))*fScale*(n+=step),
		normalize(vec3( 1,-1,-1))*fScale*(n+=step),
		normalize(vec3( 1,-1, 1))*fScale*(n+=step),
		normalize(vec3( 1, 1,-1))*fScale*(n+=step)
#elif 1
		vec3( 1, 1, 1),
		vec3(-1,-1,-1),
		vec3(-1,-1, 1),
		vec3(-1, 1,-1),

		vec3(-1, 1 ,1),
		vec3( 1,-1,-1),
		vec3( 1,-1, 1),
		vec3( 1, 1,-1)
#else
		vec3(0.717887, 0.271767, 0.539537),
		vec3(-0.974975, 0.470748, 0.120884),
		vec3(0.125706, -0.505539, -0.508408),
		vec3(-0.56859, 0.0946989, 0.827387),

		vec3(0.949156, -0.837214, -0.829524),
		vec3(-0.956053, -0.794488, -0.0399487),
		vec3(-0.776238, 0.115268, 0.598804),
		vec3(-0.943724, 0.814142, 0.586352)
#endif
	};
*/

	// create random rotation matrix
	vec3 randomNormal = texture2D(u_randomNormalMap, gl_FragCoord.xy * (1.0 / 4.0)).xyz * 2.0 - 1.0;
//	vec3 randomNormal = RandomNormal(mod(gl_FragCoord.xy, vec2(16.0)));

	// get fragment depth
	float centerDepth = texture2DRect(u_DNMiniMap, gl_FragCoord.xy).x;

	// compute sample scale
	vec3 scale = vec3(u_ssaoParms.y *
		min(centerDepth / 212.0, 1.0) *		// make area smaller if distance is less than 5 meters
		(1.0 + centerDepth / 320.0));		// make area bigger if distance is more than 32 meters

	float depthRangeScale = FARCLIP / scale.z * 0.85;

	// convert from inches to pixels
	scale.xy *= 1.0 / centerDepth;
	scale.z *= 2.0;
	scale.xy *= u_viewport;

	float depthTestSoftness = 64.0 * FARCLIP / scale.z;

	vec3 bias;
	vec4 depths[2];
	vec4 sum = vec4(0.0);

	for (int i = 0; i < 2; i++) {
		bias = reflect(kernel[i*4+0], randomNormal) * scale;

		depths[0].x = texture2DRect(u_DNMiniMap, gl_FragCoord.xy + bias.xy).x + bias.z;
		#ifdef HQ
			bias *= HQ_SCALE;
			depths[1].x = texture2DRect(u_DNMiniMap, gl_FragCoord.xy + bias.xy).x + bias.z;
		#endif

		bias = reflect(kernel[i*4+1], randomNormal) * scale;
		depths[0].y = texture2DRect(u_DNMiniMap, gl_FragCoord.xy + bias.xy).x + bias.z;
		#ifdef HQ
			bias *= HQ_SCALE;
			depths[1].y = texture2DRect(u_DNMiniMap, gl_FragCoord.xy + bias.xy).x + bias.z;
		#endif

		bias = reflect(kernel[i*4+2], randomNormal) * scale;
		depths[0].z = texture2DRect(u_DNMiniMap, gl_FragCoord.xy + bias.xy).x + bias.z;
		#ifdef HQ
			bias *= HQ_SCALE;
			depths[1].z = texture2DRect(u_DNMiniMap, gl_FragCoord.xy + bias.xy).x + bias.z;
		#endif

		bias = reflect(kernel[i*4+3], randomNormal) * scale;
		depths[0].w = texture2DRect(u_DNMiniMap, gl_FragCoord.xy + bias.xy).x + bias.z;
		#ifdef HQ
			bias *= HQ_SCALE;
			depths[1].w = texture2DRect(u_DNMiniMap, gl_FragCoord.xy + bias.xy).x + bias.z;
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
		float occ = dot(sum, vec4(2.0 / 16.0)) - 0.075;
	#else
		float occ = dot(sum, vec4(2.0 / 8.0)) - 0.075;
	#endif

	gl_FragColor = vec4(min(mix(1.0, occ, u_ssaoParms.x), 1.0));
}
