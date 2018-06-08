layout (binding = 0) uniform sampler2D		u_Diffuse;
layout (binding = 1) uniform sampler2D		u_LightMap0;
layout (binding = 2) uniform sampler2D		u_Add;
layout (binding = 3) uniform sampler2D		u_NormalMap;
layout (binding = 4) uniform sampler2DRect	u_ssaoMap;
layout (binding = 5) uniform sampler2D		u_LightMap1;
layout (binding = 6) uniform sampler2D		u_LightMap2;


layout (location = U_LM_TYPE)			uniform int		u_LightMapType;
layout (location = U_USE_SSAO)			uniform int		u_ssao;
layout (location = U_COLOR_MUL)			uniform float	u_ColorModulate;
layout (location = U_AMBIENT_LEVEL)		uniform float	u_ambientScale;    
layout (location = U_SPECULAR_SCALE)	uniform float	u_specularScale;
layout (location = U_LAVA_PASS)			uniform int		u_isLava;
layout (location = U_PARAM_INT_0)		uniform int		u_envMapPass;

in vec3	v_positionVS;
in vec3	v_viewVecTS;
in vec2	v_wTexCoord;
in vec2	v_lTexCoord;
in vec2	v_envCoord;

float	u_specularExp = 16.0;
 
//
// 3-vector radiosity basis for normal mapping
//

const vec3 s_basisVecs[3] = vec3[](
vec3 ( 0.81649658092772603273242802490196f,		0.f,									0.57735026918962576450914878050195f ),
vec3 ( -0.40824829046386301636621401245098f,	0.70710678118654752440084436210485f,	0.57735026918962576450914878050195f ),
vec3 ( -0.40824829046386301636621401245098f,	-0.70710678118654752440084436210485f,	0.57735026918962576450914878050195f )
); 

#include lighting.inc
#include parallax.inc

vec3 whiteLM = vec3(1.0, 1.0, 1.0);

void main (void) {

	
	vec3 V = normalize(v_viewVecTS);
	vec2 P = CalcParallaxOffset(u_Diffuse, v_wTexCoord.xy, V);

	vec3 diffuseMap = texture(u_Diffuse, P).xyz;
	vec3 glowMap = texture(u_Add, P).xyz;
	vec3 normalMap = normalize(texture(u_NormalMap, P).rgb * 2.0 - 1.0);
	float specular = texture(u_NormalMap, P).a;

	vec3 lm;
		if(u_isLava == 1)
			lm = whiteLM;
		if(u_isLava == 0)
			lm = texture(u_LightMap0, v_lTexCoord.xy).rgb;

	if (u_LightMapType == 0){

		fragData.xyz = diffuseMap * lm;
		
	}
	if (u_LightMapType == 1) {

		vec3 lm0 = lm;
		vec3 lm1 = texture(u_LightMap1, v_lTexCoord.xy).rgb;
		vec3 lm2 = texture(u_LightMap2, v_lTexCoord.xy).rgb;

		// diffuse
		vec3 D = vec3(
			dot(normalMap.xyz, s_basisVecs[0]),
			dot(normalMap.xyz, s_basisVecs[1]),
			dot(normalMap.xyz, s_basisVecs[2]));

		// Omit energy-conserving division by PI here.
		D = lm0 * D.x + lm1 * D.y + lm2 * D.z;

		// approximate specular
		// half-angle vector, stable but slower
		vec3 H0 = normalize(V + s_basisVecs[0]);
		vec3 H1 = normalize(V + s_basisVecs[1]);
		vec3 H2 = normalize(V + s_basisVecs[2]);

		vec3 S = vec3(
			dot(normalMap.xyz, H0),
			dot(normalMap.xyz, H1),
			dot(normalMap.xyz, H2));

//		S = scale * pow(max(S, 0.0), fts);
		S = pow(max(S, 0.0), vec3(u_specularExp));
		S = (lm0 * S.x + lm1 * S.y + lm2 * S.z);

		// Approximate energy conservation.
		// Omit division by PI on both diffuse & specular,
		// dividing the latter by 8 instead of (8 * PI).
		// After this, scale by PI because after multiplying by lightmap
		// we actually get N.L * N.H here, not N.H.
		S *= (u_specularExp + 8.0) / (8.0 / PI);

		// The more material is specular, the less it is diffuse.
		// Assume all shiny materials are metals of the same moderate roughness in Q2,
		// treat diffuse map as combined albedo & normal map alpha channel as a rough-to-shiny ratio.
		fragData.xyz = diffuseMap * mix(D, S, specular * u_specularScale);
	}
      
	if(u_envMapPass == 1){
	vec3 envMap = texture(u_Diffuse, v_envCoord).rgb;
	float lum = dot(vec3(0.2125, 0.7154, 0.0721), envMap);
	fragData.xyz +=	lum * envMap;	
	}

	if (u_ssao == 1)
		fragData.xyz *= texture2DRect(u_ssaoMap, gl_FragCoord.xy * 0.5).xyz;

	// fake AO/cavity
	fragData.xyz *= normalMap.z * 0.5 + 0.5;
	fragData.xyz *= u_ColorModulate * u_ambientScale;
	fragData += vec4(glowMap, 1.0);
	fragData.w = 1.0;

// DEBUG
//	if (u_ssao == 1)
//		fragData.xyz = texture2DRect(u_ssaoMap, gl_FragCoord.xy * 0.5).xyz;
}
