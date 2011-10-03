uniform sampler2D	u_Diffuse;
uniform sampler2D	u_LightMap;
uniform sampler2D	u_Add;
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_deluxMap;
uniform float       u_ColorModulate;
uniform float       u_ambientScale;    
uniform int			u_bumpMap;
uniform vec2		u_bumpScale;
uniform vec2		u_texSize;
uniform int			u_parallaxType;
uniform int			u_numSteps;
uniform sampler2D	u_Caustics;
uniform float       u_CausticsModulate; 
uniform float		u_lightScale; 

varying vec3		v_viewVecTS;
varying vec3		t, b, n;
varying vec2		v_wTexCoord;
varying vec2		v_lTexCoord;
varying vec4		v_color;

float ComputeLOD( vec2 tc, vec2 texSize ) { 

 vec2 dx = dFdx( tc ); 
 vec2 dy = dFdy( tc ); 

 vec2 mag = ( abs( dx )  + abs( dy )  ) * texSize; 
 
 float lod = log2( max( mag.x, mag.y ) ); 

 return lod; 

} 

vec2 CalcParallaxOffset (in sampler2D hiMap, in vec2 texCoord, in vec3 viewVec) {
	
	
	if (u_parallaxType == 2)
	{

	/*===================================
	Parallax Occlusion Maping
	High quality, uses Crysis(tm) shader.
	===================================*/

	// clamp z value - fix (but not full) smooth tbn bug 
	if (viewVec.z < 0.0)
		viewVec.z = clamp(viewVec.z, -1.0, -0.1);
	else
		viewVec.z = clamp(viewVec.z, 0.1, 1.0);

	float lod = ComputeLOD(texCoord, u_texSize);

	float	step = 1.0 / float(u_numSteps);
	vec2	delta = 2.0 * u_bumpScale * viewVec.xy / (-viewVec.z * float(u_numSteps));
	float	NB0 = texture2DLod(hiMap, texCoord, lod).a;
	float	height = 1.0 - step;
	vec2	offset = texCoord + delta;
	float	NB1 = texture2DLod(hiMap, offset, lod).a;

	for (int i = 0; i < u_numSteps; i++) {
		
		if (NB1 >= height)
			break;
	
		NB0 = NB1;

		height -= step;
		offset += delta;

		NB1 = texture2DLod(hiMap, offset, lod).a;
	}

	vec2 offsetBest = offset;
	float error = 1.0;

	float t1 = height;
	float t0 = t1 + step;
	
	float delta1 = t1 - NB1;
	float delta0 = t0 - NB0;

	vec4 intersect = vec4(delta * float(u_numSteps), delta * float(u_numSteps) + texCoord);
   
	for (int i = 0; i < 10; i++) {
		if (abs(error) <= 0.01)
			break;
	
		float denom = delta1 - delta0;
		float t = (t0 * delta1 - t1 * delta0) / denom;
		offsetBest = -t * intersect.xy + intersect.zw;
		
		float NB = texture2DLod(hiMap, offsetBest, lod).a;

		error = t - NB;
		if (error < 0.0) {
			delta1 = error;
			t1 = t;
		}
		else {
			delta0 = error;
			t0 = t;
		}
	}
	
	return offsetBest;

	} else if(u_parallaxType == 1){
	
	// simple fastest parallax mapping
	
	float offset = texture2D( hiMap, texCoord.xy ).a;
	offset = offset * 0.04 - 0.02;
	vec2 offsetBest = offset * viewVec.xy + texCoord.xy;
	
	return offsetBest;
	}
}

vec2 PhongLighting (const in vec3 N, const in vec3 L, const in vec3 V, const float sExp) {
	vec2 E;
	E.x = max(dot(N, L), 0.0);
	vec3 R = reflect(-L, N);
	E.y = pow(max(dot(R, V), 0.0), sExp);
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

#ifdef VERTEXLIGHT
diffuseMap *= clamp(v_color, 0.0, 0.666);
#endif

#ifdef BUMP

#ifdef VERTEXLIGHT
tbnDelux.x = abs(dot(n, t));
tbnDelux.y = abs(dot(n, b));
tbnDelux.z = 1.0;
vec4 specular = vec4(specTmp, specTmp, specTmp, specTmp);
vec2 E = PhongLighting(normalMap, tbnDelux, V, 16.0);

#else

//Put delux into tangent space
tbnDelux.x = dot(wDelux, t);
tbnDelux.y = dot(wDelux, b);
tbnDelux.z = dot(wDelux, n);
tbnDelux = abs(tbnDelux);

vec4 specular = vec4(specTmp, specTmp, specTmp, specTmp);
vec2 E = PhongLighting(normalMap, tbnDelux, V, 16.0);
#endif

#ifdef LIGHTMAP
bumpLight = (E.x * diffuseMap) + (E.y * specular * lightMap);
diffuseMap *= u_ambientScale;
#endif

#ifdef VERTEXLIGHT
bumpLight = (E.x * diffuseMap * v_color) + (E.y * specular * v_color); //via lava surfaces 
diffuseMap *= u_ambientScale;
#endif

diffuseMap += bumpLight;
#endif

#ifdef LIGHTMAP
diffuseMap *= lightMap;
#endif

vec4 finalColor = diffuseMap + glowMap;

#ifdef CAUSTICS
vec4 tmp;
tmp = causticsMap * finalColor;
tmp *= u_CausticsModulate;
finalColor = tmp + finalColor;
#endif

gl_FragColor = finalColor * u_ColorModulate;

}
