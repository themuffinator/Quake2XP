struct SHCoeffs {
	vec4	c;
};

struct SHSpectralCoeffs {
	vec4	r, g, b;
};

SHSpectralCoeffs SH_Add (const in SHSpectralCoeffs a, const in SHSpectralCoeffs b) {
	SHSpectralCoeffs c;

	c.r = a.r + b.r;
	c.g = a.g + b.g;
	c.b = a.b + b.b;

	return c;
}

SHSpectralCoeffs SH_Sub (const in SHSpectralCoeffs a, const in SHSpectralCoeffs b) {
	SHSpectralCoeffs c;

	c.r = a.r - b.r;
	c.g = a.g - b.g;
	c.b = a.b - b.b;

	return c;
}

SHSpectralCoeffs SH_Mul (const in SHCoeffs c, const in vec3 vec) {
	SHSpectralCoeffs sc;

	sc.r = c.c * vec.x;
	sc.g = c.c * vec.y;
	sc.b = c.c * vec.z;

	return sc;
}

SHSpectralCoeffs SH_Mul (in SHSpectralCoeffs sc, const in vec3 vec) {
	sc.r *= vec.x;
	sc.g *= vec.y;
	sc.b *= vec.z;

	return sc;
}

SHCoeffs SH_Lerp(const in SHCoeffs a, const in SHCoeffs b, const in float t) {
	SHCoeffs c;

	c.c = mix(a.c, b.c, t);

	return c;
}

SHSpectralCoeffs SH_Lerp (const in SHSpectralCoeffs a, const in SHCoeffs b, const in float t) {
	SHSpectralCoeffs sc;

	sc.r = mix(a.r, b.c, t);
	sc.g = mix(a.g, b.c, t);
	sc.b = mix(a.b, b.c, t);

	return sc;
}

SHSpectralCoeffs SH_Lerp (const in SHSpectralCoeffs a, const in SHSpectralCoeffs b, const in float t) {
	SHSpectralCoeffs sc;

	sc.r = mix(a.r, b.r, t);
	sc.g = mix(a.g, b.g, t);
	sc.b = mix(a.b, b.b, t);

	return sc;
}

vec3 SH_Dot (const in SHSpectralCoeffs sc, const in SHCoeffs c) {
	return vec3(
		dot(sc.r, c.c),
		dot(sc.g, c.c),
		dot(sc.b, c.c));
}

void SH_NormalizeBands (inout SHCoeffs sc) {
	sc.c.yzw /= max(0.05, length(sc.c.yzw));
}

void SH_NormalizeBands (inout SHSpectralCoeffs sc) {
	sc.r.yzw /= max(0.05, length(sc.r.yzw));
	sc.g.yzw /= max(0.05, length(sc.g.yzw));
	sc.b.yzw /= max(0.05, length(sc.b.yzw));
}

SHSpectralCoeffs SH_NormalizeDir (in SHSpectralCoeffs sc) {
	SH_NormalizeBands(sc);

	sc.r.yzw *= 0.488602512;
	sc.g.yzw *= 0.488602512;
	sc.b.yzw *= 0.488602512;

	sc.r.x = 0.282094792;
	sc.g.x = 0.282094792;
	sc.b.x = 0.282094792;

	return sc;
}

SHSpectralCoeffs SH_NormalizeCone (in SHSpectralCoeffs sc, const in float halfAngle) {
	SHCoeffs c;

	vec2 zhCoeffs = vec2(0.5 * (1.0 - cos(halfAngle)), 0.75 * sin(halfAngle) * sin(halfAngle));

	sc.r.yzw *= zhCoeffs.y;
	sc.g.yzw *= zhCoeffs.y;
	sc.b.yzw *= zhCoeffs.y;

	sc.r.x *= zhCoeffs.x;
	sc.g.x *= zhCoeffs.x;
	sc.b.x *= zhCoeffs.x;

	return sc;
}

SHSpectralCoeffs SH_NormalizeCone90 (in SHSpectralCoeffs sc) {
	SH_NormalizeBands(sc);

	sc.r.yzw *= 0.375;	// 3/8
	sc.g.yzw *= 0.375;
	sc.b.yzw *= 0.375;

	sc.r.x = 0.146447;	// 1/2 (1 - 1/Sqrt[2])
	sc.g.x = 0.146447;
	sc.b.x = 0.146447;

	return sc;
}

SHSpectralCoeffs SH_Normalize (in SHSpectralCoeffs sc) {
	SH_NormalizeBands(sc);

	sc.r.x = 1.0;
	sc.g.x = 1.0;
	sc.b.x = 1.0;

	return sc;
}

vec3 SH_ExtractDir (const in SHCoeffs c) {
	return vec3(-c.c.w, -c.c.y, c.c.z);
}

vec3 SH_ExtractNormalizedDir (const in SHCoeffs c) {
	float l = dot(c.c.yzw, c.c.yzw);
	return l > 0.0001 ? normalize(SH_ExtractDir(c)) : vec3(0.0);
}

SHCoeffs SH_Dir (in vec3 dir) {
	SHCoeffs c;

	c.c = vec4(
		0.282094792,
		-0.488602512 * dir.y,
		0.488602512 * dir.z,
		-0.488602512 * dir.x);

	return c;
}

// Takes a direction and zonal harmonics coefficients as an input and returns
// SH coefficients of this function rotated towards given direction:
SHCoeffs SH_Rotate (const in vec3 dir, const in vec2 zhCoeffs) {
	vec2 theta12_cs;

	if (dot(dir.xy, dir.xy) > 0.0001)
		theta12_cs.xy = normalize(dir.xy);
	else
		theta12_cs.xy = vec2(0.0);

	vec2 phi12_cs;
	phi12_cs.y = dir.z;
	phi12_cs.x = sqrt(1.0 - dir.z * dir.z);

	SHCoeffs c;

	// the first band is rotation-independent
	c.c.x = zhCoeffs.x;

	// rotate the second band of SH
	c.c.y = zhCoeffs.y * phi12_cs.x * theta12_cs.y; 
	c.c.z = -zhCoeffs.y * phi12_cs.y; 
	c.c.w = zhCoeffs.y * phi12_cs.x * theta12_cs.x;
	
	return c;
}

SHCoeffs SH_Cone (const in vec3 dir, const in float halfAngle) {
	SHCoeffs c;

	vec2 zhCoeffs = vec2(0.5 * (1.0 - cos(halfAngle)), 0.75 * sin(halfAngle) * sin(halfAngle));
	c.c = vec4(zhCoeffs.x, zhCoeffs.yyy * vec3(-dir.y, dir.z, -dir.x));

	return c;
}

SHCoeffs SH_Cone60(const in vec3 dir) {
	SHCoeffs c;

	// (1/2 (1 - Sqrt[3]/2), 3 / 16)
	vec2 zhCoeffs = vec2(0.0669873, 0.1875);
	c.c = vec4(zhCoeffs.x, zhCoeffs.yyy * vec3(-dir.y, dir.z, -dir.x));

	return c;
}

SHCoeffs SH_Cone90 (const in vec3 dir) {
	SHCoeffs c;

	// (1/2 (1 - 1/Sqrt[2]), 3 / 8)
	vec2 zhCoeffs = vec2(0.146447, 0.375);
	c.c = vec4(zhCoeffs.x, zhCoeffs.yyy * vec3(-dir.y, dir.z, -dir.x));

	return c;
}

SHCoeffs SH_Cosine (const in vec3 dir) {
	SHCoeffs c;

	c.c.x = 0.886227; 
	c.c.yzw = vec3(-1.02333, 1.02333, -1.02333) * dir.yzx;

	return c;
}
