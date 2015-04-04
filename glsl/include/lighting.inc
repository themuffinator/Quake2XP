#define	CUTOFF_EPSILON		1.0/255.0		// light intensity below which fragment is discarded
#define PI			3.14159265358979323846

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

/*
==============
PointAttenuation

'p' - position in unit light space.
==============
*/
float PointAttenuation (const in vec3 p, const in float e) {
	return pow(max(1.0 - dot(p, p), 0.0), e);
}