#define PI			3.14159265358979323846
#define HALF_PI			1.57079632679489661923

uniform vec4			u_parallaxParams; // x = scale[0] y = scale[1] z = tex_w[2] w = tex_h
uniform int				u_parallaxType;

float ComputeLOD( vec2 tc, vec2 texSize ) { 
 vec2 dx = dFdx( tc ); 
 vec2 dy = dFdy( tc ); 
 vec2 mag = ( abs( dx )  + abs( dy )  ) * texSize; 
 float lod = log2( max( mag.x, mag.y ) ); 
 return lod; 
} 

void IntersectConeExp (in sampler2D csmMap, inout vec3 dp, in vec3 ds, in float distFactor) {
	
	// conestep parallax mapping
	// improved fastest shader
	float lod = ComputeLOD(dp, u_parallaxParams.zw);
	float iz = length(ds.xy);
	float ht, old_ht, cr = 0.0;
	int limit = 16;
	vec2 t = texture2DLod(csmMap, dp.xy, lod).rg;

	while (t.r > dp.z && limit > 0) {     
		cr = t.g * t.g;
		dp += ds * (distFactor + (t.r - dp.z) * cr) / (iz + cr);
		t = texture2DLod(csmMap, dp.xy, lod).rg;
		limit--;
	}

	ht = t.r - dp.z;
	distFactor /= (iz + cr);
	dp -= ds * distFactor;

	t = texture2DLod(csmMap, dp.xy, lod).rg;
	old_ht = t.r - dp.z;

	dp += ds * distFactor * (1.0 - clamp(ht / (ht - old_ht), 0.0, 1.0));
}

vec2 CalcParallaxOffset (in sampler2D hiMap, in vec2 texCoord, in vec3 viewVec) {
	
	// 14 sample relief mapping: linear search and then binary search, 
	// improved qfusion shader
	float lod = ComputeLOD(texCoord, u_parallaxParams.zw);
	vec3 offsetVector = vec3(viewVec.xy * u_parallaxParams.xy * vec2(-1.0, -1.0), -1.0);
	vec3 offsetBest = vec3(texCoord, 1.0);
	offsetVector *= 0.1;

	offsetBest += offsetVector *  step(texture2DLod(hiMap, offsetBest.xy, lod).a, offsetBest.z);
	offsetBest += offsetVector *  step(texture2DLod(hiMap, offsetBest.xy, lod).a, offsetBest.z);
	offsetBest += offsetVector *  step(texture2DLod(hiMap, offsetBest.xy, lod).a, offsetBest.z);
	offsetBest += offsetVector *  step(texture2DLod(hiMap, offsetBest.xy, lod).a, offsetBest.z);
	offsetBest += offsetVector *  step(texture2DLod(hiMap, offsetBest.xy, lod).a, offsetBest.z);
	offsetBest += offsetVector *  step(texture2DLod(hiMap, offsetBest.xy, lod).a, offsetBest.z);
	offsetBest += offsetVector *  step(texture2DLod(hiMap, offsetBest.xy, lod).a, offsetBest.z);
	offsetBest += offsetVector *  step(texture2DLod(hiMap, offsetBest.xy, lod).a, offsetBest.z);
	offsetBest += offsetVector *  step(texture2DLod(hiMap, offsetBest.xy, lod).a, offsetBest.z);
	offsetBest += offsetVector * (step(texture2DLod(hiMap, offsetBest.xy, lod).a, offsetBest.z)          - 0.5);
	offsetBest += offsetVector * (step(texture2DLod(hiMap, offsetBest.xy, lod).a, offsetBest.z) * 0.5    - 0.25);
	offsetBest += offsetVector * (step(texture2DLod(hiMap, offsetBest.xy, lod).a, offsetBest.z) * 0.25   - 0.125);
	offsetBest += offsetVector * (step(texture2DLod(hiMap, offsetBest.xy, lod).a, offsetBest.z) * 0.125  - 0.0625);
	offsetBest += offsetVector * (step(texture2DLod(hiMap, offsetBest.xy, lod).a, offsetBest.z) * 0.0625 - 0.03125);
	
	return offsetBest.xy;
}