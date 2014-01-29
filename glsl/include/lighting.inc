#define	CUTOFF_EPSILON		0.01		// light intensity below which fragment is discarded

vec2 PhongLighting (const in vec3 N, const in vec3 L, const in vec3 V, const float sExp) {
	vec2 E;
	E.x = max(dot(N, L), 0.0);
	vec3 R = reflect(-L, N);
	E.y = pow(max(dot(R, V), 0.0), sExp);
	E.y *= E.x;
	return E;
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

float InScatter(vec3 start, vec3 dir, vec3 lightPos, float d)
{
	// calculate quadratic coefficients a,b,c
	vec3 q = start - lightPos;

	float b = dot(dir, q);
	float c = dot(q, q);

	// evaluate integral
	float s = 1.0f / sqrt(c - b*b);

	float l = s * (atan( (d + b) * s) - atan( b*s ));

	return l;	
}