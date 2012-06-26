uniform sampler2D		u_Diffuse;
uniform sampler2D		u_LightMap;
uniform sampler2D		u_Add;
uniform sampler2D		u_NormalMap;
uniform sampler2D		u_deluxMap;
uniform sampler2D		u_Caustics;
uniform float       	u_ColorModulate;
uniform float       	u_ambientScale;    
uniform float       	u_CausticsModulate; 
uniform vec2			u_texSize;
uniform vec2			u_parallaxScale;
uniform int				u_parallaxType;
uniform int				u_isCaustics;
uniform float			u_specularScale;
uniform float			u_specularExp;

varying vec3			v_viewVecTS;
varying vec3			t, b, n;
varying vec2			v_wTexCoord;
varying vec2			v_lTexCoord;
varying vec4			v_color;
varying vec3			v_lightVec[13];


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
vec4 lightMap = texture2D(u_LightMap, v_lTexCoord.xy); 
vec3 wDelux = normalize(texture2D(u_deluxMap, v_lTexCoord).rgb - 0.5);
vec4 diffuseMap;
vec4 glowMap;
vec4 causticsMap;
vec3 normalMap;
float specTmp;
vec3 tbnDelux;
vec4 bumpLight;

#ifdef PARALLAX
vec2 P = CalcParallaxOffset(u_Diffuse, v_wTexCoord.xy, V);
diffuseMap = texture2D(u_Diffuse, P);
glowMap = texture2D(u_Add, P);
causticsMap = texture2D(u_Caustics, P);
normalMap =  normalize(texture2D(u_NormalMap, P.xy).rgb - 0.5);
specTmp = texture2D(u_NormalMap,   P.xy).a;

#else

diffuseMap = texture2D(u_Diffuse,  v_wTexCoord.xy);
glowMap = texture2D(u_Add,  v_wTexCoord.xy);
causticsMap = texture2D(u_Caustics, v_wTexCoord.xy);
normalMap =  normalize(texture2D(u_NormalMap, v_wTexCoord.xy).rgb - 0.5);
specTmp = texture2D(u_NormalMap, v_wTexCoord).a;

#endif 

vec4 specular = vec4(specTmp, specTmp, specTmp, specTmp);

specular *=u_specularScale;

#ifdef VERTEXLIGHT
diffuseMap *= clamp(v_color, 0.0, 0.666);
#endif

// Bump World 
#ifdef BUMP

#ifdef VERTEXLIGHT
tbnDelux.x = abs(dot(n, t));
tbnDelux.y = abs(dot(n, b));
tbnDelux.z = 1.0;
vec2 Es = PhongLighting(normalMap, tbnDelux, V);

#else

//Put delux into tangent space
tbnDelux.x = dot(wDelux, t);
tbnDelux.y = dot(wDelux, b);
tbnDelux.z = dot(wDelux, n);
tbnDelux = abs(tbnDelux);
tbnDelux = clamp(tbnDelux, 0.35, 1.0);
vec2 Es = PhongLighting(normalMap, tbnDelux, V);
#endif

#ifdef LIGHTMAP
bumpLight = (Es.x * diffuseMap) + (Es.y * specular * lightMap);
diffuseMap *= u_ambientScale;
#endif

#ifdef VERTEXLIGHT
bumpLight = (Es.x * diffuseMap * v_color) + (Es.y * specular * v_color); //via lava surfaces 
diffuseMap *= u_ambientScale;
#endif

diffuseMap += bumpLight;

#ifdef LIGHTMAP
diffuseMap *= lightMap;
#endif

vec4 finalColor = diffuseMap + glowMap;

if (u_isCaustics == 1){
vec4 tmp;
tmp = causticsMap * finalColor;
tmp *= u_CausticsModulate;
finalColor = tmp + finalColor;
}

#else
// Non bump World
#ifdef LIGHTMAP
diffuseMap *= lightMap;
#endif

vec4 finalColor = diffuseMap + glowMap;

if (u_isCaustics == 1){
vec4 tmp;
tmp = causticsMap * finalColor;
tmp *= u_CausticsModulate;
finalColor = tmp + finalColor;
}

#endif

gl_FragColor = vec4(finalColor.rgb, 1.0) * u_ColorModulate;

}
