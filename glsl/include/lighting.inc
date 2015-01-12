#define	CUTOFF_EPSILON		1.0/255.0		// light intensity below which fragment is discarded

vec2 PhongLighting (const in vec3 N, const in vec3 L, const in vec3 V, const float sExp) {
	vec2 E;
	E.x = max(dot(N, L), 0.0);
	vec3 R = reflect(-L, N);
	E.y = pow(max(dot(R, V), 0.0), sExp);
	E.y *= E.x;
	return E;
}

float halfLambert(in vec3 N, in vec3 L )  
{  
     return dot ( N, L ) * 0.5 + 2.0;
} 

float LambertLighting(const in vec3 N, const in vec3 L) {
	float E;
	return E = max(dot (N, L), 0.0);
}
