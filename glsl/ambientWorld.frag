//!#include "include/global.inc"

layout (bindless_sampler, location = U_TMU0) uniform sampler2D		u_Diffuse;
layout (bindless_sampler, location = U_TMU1) uniform sampler2D		u_Add;
layout (bindless_sampler, location = U_TMU2) uniform sampler2D		u_NormalMap;
layout (bindless_sampler, location = U_TMU3) uniform sampler2D		u_LightMap0;
layout (bindless_sampler, location = U_TMU4) uniform sampler2D		u_LightMap1;
layout (bindless_sampler, location = U_TMU5) uniform sampler2D		u_LightMap2;
layout (bindless_sampler, location = U_TMU6) uniform sampler2DRect	u_ssaoMap;

layout (location = U_LM_TYPE)			uniform int		u_LightMapType;
layout (location = U_USE_SSAO)			uniform int		u_ssao;
layout (location = U_AMBIENT_LEVEL)		uniform float	u_ambientScale;    
layout (location = U_SPECULAR_SCALE)	uniform float	u_specularScale;
layout (location = U_LAVA_PASS)			uniform int		u_isLava;
layout (location = U_PARAM_INT_0)		uniform int		u_envMapPass;
layout (location = U_PARAM_INT_1)		uniform int		u_bump;	

in vec3	v_positionVS;
in vec3	v_viewVecTS;
in vec2	v_wTexCoord;
in vec2	v_lTexCoord;
in vec3	v_bumpVecs[3];

float	u_specularExp = 16.0;

#include lighting.inc //!#include "include/lighting.inc"
#include parallax.inc //!#include "include/parallax.inc"


void main (void) {
	
	vec3 whiteLM = vec3(1.0, 1.0, 1.0);	

	vec3 V = normalize(v_viewVecTS);
	vec2 P;

	switch (u_parallaxType) {
		case 0: 
		P = v_wTexCoord;
		break;
		case 1: 
		P = parallaxMapping(u_Diffuse, v_wTexCoord, V);
		break;
		case 2: 
		P = ReliefMapping(u_Diffuse, v_wTexCoord, V);
		break;
	}
	
	vec3 diffuseMap = texture(u_Diffuse, P).xyz;
//	diffuseMap.rgb = pow(diffuseMap.rgb, vec3(2.2));

	vec3 glowMap = texture(u_Add, P).xyz;
	vec3 normalMap = normalize(texture(u_NormalMap, P).rgb * 2.0 - 1.0);
	float specular = texture(u_NormalMap, P).a;

	vec3 lm;
		if(u_isLava == 1)
			lm = whiteLM;
		if(u_isLava == 0)
			lm = texture(u_LightMap0, v_lTexCoord.xy).rgb;
	
	if(u_LightMapType == 1){

		vec3 lm0 = lm;
		vec3 lm1 = texture(u_LightMap1, v_lTexCoord.xy).rgb;
		vec3 lm2 = texture(u_LightMap2, v_lTexCoord.xy).rgb;

		// diffuse
		vec3 D = vec3(
			dot(normalMap.xyz, v_bumpVecs[0]),
			dot(normalMap.xyz, v_bumpVecs[1]),
			dot(normalMap.xyz, v_bumpVecs[2]));
		
		// Omit energy-conserving division by PI here.
		D = lm0 * D.x + lm1 * D.y + lm2 * D.z;

		// approximate specular
		// half-angle vector, stable but slower
		vec3 H0 = normalize(V + v_bumpVecs[0]);
		vec3 H1 = normalize(V + v_bumpVecs[1]);
		vec3 H2 = normalize(V + v_bumpVecs[2]);

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
		if(u_bump == 1)
			fragData.xyz = diffuseMap * mix(D, S, specular * u_specularScale);
		else
			fragData.xyz = diffuseMap * D;

	} else
		fragData.xyz = diffuseMap * lm;

	if(u_envMapPass == 1){  
  
  		vec3 reflectionVector = normalMap * dot( V, normalMap );
		reflectionVector = ( reflectionVector * 2.0f ) - V;
  
		vec3 envMap = texture(u_Diffuse, reflectionVector.st).rgb;
		envMap *= 0.5;
  		float lum = dot(vec3(0.2125, 0.7154, 0.0721), envMap);
		envMap = envMap * lum;    
		fragData.xyz +=	envMap;	
	}

	if (u_ssao == 1)
		fragData.xyz *= texture(u_ssaoMap, gl_FragCoord.xy * 0.5).xyz;

	// fake AO/cavity
	fragData.xyz *= normalMap.z * 0.5 + 0.5;
	fragData.xyz *= u_ambientScale;
	fragData += vec4(glowMap * 2.0, 1.0);
	fragData.w = 1.0;

// DEBUG
//	if (u_ssao == 1)
//		fragData.xyz = texture(u_ssaoMap, gl_FragCoord.xy * 0.5).xyz;
}
