uniform sampler2D	u_Diffuse;
uniform sampler2D	u_LightMap;
uniform sampler2D	u_Add;
uniform sampler2D	u_NormalMap;
uniform float       u_ColorModulate;
uniform float       u_ambientScale;    
uniform int			u_bumpMap;
uniform vec2		u_bumpScale;
uniform int			u_parallaxType;
uniform int			u_numSteps;
 
uniform sampler2D	u_Caustics;
uniform float       u_CausticsModulate; 

varying vec3		v_viewVecTS;
varying vec3		v_lightVec;

vec2 CalcParallaxOffset (in sampler2D hiMap, in vec2 texCoord, in vec3 viewVec) {
	
	if (u_parallaxType == 2)
	{

	//Parallax Occlusion Maping
	//High quality, uses Crysis(tm) shader.

	float	step = 1.0 / float(u_numSteps);
	vec2	delta = 2.0 * u_bumpScale * viewVec.xy / (-viewVec.z * float(u_numSteps));
	float	NB0 = texture2D(hiMap, texCoord).a;
	float	height = 1.0 - step;
	vec2	offset = texCoord + delta;
	float	NB1 = texture2D(hiMap, offset).a;


	for (int i = 0; i < u_numSteps; i++) {
		
		if (NB1 >= height)
			break;
			
		NB0 = NB1;

		height -= step;
		offset += delta;

		NB1 = texture2D(hiMap, offset).a;
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
		
		float NB = texture2D(hiMap, offsetBest).a;

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
	E.y *= (E.x < 0.01 ? 0.0 : 1.0);

	return E;
}

void main ()
{
vec3 V = normalize(v_viewVecTS);
vec3 L = normalize(v_lightVec);

#ifdef PARALLAX
vec2 P = CalcParallaxOffset(u_Diffuse, gl_TexCoord[0].xy, V);
vec4 diffuseMap = texture2D(u_Diffuse,  P);
vec4 lightMap = texture2D(u_LightMap, gl_TexCoord[1].xy); 
vec4 glowMap = texture2D(u_Add,      P);
vec4 causticsMap = texture2D(u_Caustics, P);

vec3 normalMap =  normalize(texture2D(u_NormalMap, P.xy).rgb * 2.0 - 1.0);
float specTmp = texture2D(u_NormalMap,   P.xy).a;

#else

vec4 diffuseMap = texture2D(u_Diffuse,  gl_TexCoord[0].xy);
vec4 lightMap = texture2D(u_LightMap, gl_TexCoord[1].xy); 
vec4 glowMap = texture2D(u_Add,      gl_TexCoord[0].xy);
vec4 causticsMap = texture2D(u_Caustics, gl_TexCoord[0].xy);

vec3 normalMap =  normalize(texture2D(u_NormalMap, gl_TexCoord[0].xy).rgb * 2.0 - 1.0);
float specTmp = texture2D(u_NormalMap,   gl_TexCoord[0].xy).a;
#endif 

#ifdef BUMP
vec4 bumpLight;
vec4 specular = vec4(specTmp, specTmp, specTmp, specTmp);
vec2 E = PhongLighting(normalMap, L, V, 16.0);

#ifdef LIGHTMAP
bumpLight = (E.x * diffuseMap) + (E.y * specular * lightMap);
diffuseMap *= u_ambientScale;
#endif

#ifdef VERTEXLIGHT
bumpLight = (E.x * diffuseMap * gl_Color) + (E.y * specular * gl_Color); //via lava surfaces 
diffuseMap *= u_ambientScale;
#endif

diffuseMap += bumpLight;
#endif

#ifdef LIGHTMAP
diffuseMap *= lightMap;
#endif

vec4 finalColor = diffuseMap + glowMap;

vec4 tmp;

#ifdef CAUSTICS
tmp = causticsMap * finalColor;
tmp *= u_CausticsModulate;
finalColor = tmp + finalColor;
#endif

gl_FragColor = finalColor * u_ColorModulate;

}
