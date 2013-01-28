
vec2 PhongLighting (const in vec3 N, const in vec3 L, const in vec3 V, const float sExp) {
	vec2 E;
	E.x = max(dot(N, L), 0.0);
	vec3 R = reflect(-L, N);
	E.y = pow(max(dot(R, V), 0.0), sExp);
//	E.y *= (E.x < 0.01 ? 0.0 : 1.0);
	E.y *= E.x;
	return E;
}

float AmbientLighting (const in vec3 N, const in vec3 L, const in vec3 V) {
	vec2 E;
	E.x = max(dot(N, L), 0.0);
	vec3 R = reflect(-L, N);
	return E;
}