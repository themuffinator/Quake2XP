uniform sampler2D		u_Diffuse;
uniform sampler2D		u_NormalMap;

uniform float       	u_ColorModulate;
 
uniform vec2			u_texSize;
uniform vec2			u_parallaxScale;
uniform int				u_parallaxType;

uniform float			u_specularScale;
uniform float			u_specularExp;

uniform vec4			u_LightColor;
uniform float			u_LightRadius;
varying vec3			v_viewVecTS;
varying vec2			v_wTexCoord;
varying vec4			v_color;
varying vec3			v_lightVec;


float ComputeLOD( vec2 tc, vec2 texSize ) { 

 vec2 dx = dFdx( tc ); 
 vec2 dy = dFdy( tc ); 

 vec2 mag = ( abs( dx )  + abs( dy )  ) * texSize; 
 
 float lod = log2( max( mag.x, mag.y ) ); 

 return lod; 

} 

vec2 CalcParallaxOffset (in sampler2D hiMap, in vec2 texCoord, in vec3 viewVec) {
	
	if (u_parallaxType == 2){
	// 14 sample relief mapping: linear search and then binary search, 
	// improved qfusion shader
	float lod = ComputeLOD(texCoord, u_texSize);
	vec3 offsetVector = vec3(viewVec.xy * u_parallaxScale * vec2(-1, -1), -1);
	vec3 offsetBest = vec3(texCoord, 1);
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
	else if(u_parallaxType == 1){
	// simple fastest parallax mapping
	float lod = ComputeLOD(texCoord, u_texSize);
	float offset = texture2DLod( hiMap, texCoord.xy, lod).a;
	offset = offset * 0.04 - 0.02;
	vec2 offsetBest = offset * viewVec.xy + texCoord.xy;
	
	return offsetBest;
	}
}

vec2 PhongLighting (const in vec3 N, const in vec3 L, const in vec3 V) {
	vec2 E;
	E.x = max(dot(N, L), 0.0);
	vec3 R = reflect(-L, N);
	E.y = pow(max(dot(R, V), 0.0), u_specularExp);
//	E.y *= (E.x < 0.01 ? 0.0 : 1.0);
	E.y *= E.x;
	return E;
}

void main ()
{
vec3 V = normalize(v_viewVecTS);
vec4 diffuseMap;
vec3 normalMap;
float specTmp;

#ifdef PARALLAX
vec2 P = CalcParallaxOffset(u_Diffuse, v_wTexCoord.xy, V);
diffuseMap = texture2D(u_Diffuse, P);
normalMap =  normalize(texture2D(u_NormalMap, P.xy).rgb - 0.5);
specTmp = texture2D(u_NormalMap,   P.xy).a;

#else

diffuseMap = texture2D(u_Diffuse,  v_wTexCoord.xy);
normalMap =  normalize(texture2D(u_NormalMap, v_wTexCoord.xy).rgb - 0.5);
specTmp = texture2D(u_NormalMap, v_wTexCoord).a;

#endif 

vec4 specular = vec4(specTmp, specTmp, specTmp, specTmp);

specular *=u_specularScale;

vec3 tmp1 = v_lightVec;
tmp1 /= u_LightRadius;
float att = max(1.0 - dot(tmp1, tmp1), 0.0);
vec3 L = normalize(v_lightVec);
vec2 E = PhongLighting(normalMap, L, V);
E *= att;
vec3 Dlighting = (E.x * diffuseMap.rgb + E.y * specular.rgb) * u_LightColor;

gl_FragColor = vec4(Dlighting.rgb, 1.0) * u_ColorModulate;

}
