uniform sampler2D		u_Diffuse;
uniform sampler2D		u_NormalMap;
uniform sampler2D		u_Add;

uniform sampler2D		u_LightMap0;
uniform sampler2D		u_LightMap1;
uniform sampler2D		u_LightMap2;
uniform int				u_LightMapType;

uniform int				u_ssao;
uniform sampler2DRect	u_ssaoMap;

uniform float			u_ColorModulate;
uniform float			u_ambientScale;    

uniform float			u_specularScale;
uniform float			u_specularExp;

in vec3			v_positionVS;
in vec3			v_viewVecTS;
in vec2			v_wTexCoord;
in vec2			v_lTexCoord;
in vec2			v_envCoord;
 
//
// 3-vector radiosity basis for normal mapping
//

const vec3 s_basisVecs[3] = vec3[](
vec3(0.81649658092772603273242802490196,  0.0                                , 0.57735026918962576450914878050195),
vec3(-0.40824829046386301636621401245098,  0.70710678118654752440084436210485, 0.70710678118654752440084436210485),
vec3(-0.40824829046386301636621401245098, -0.70710678118654752440084436210485, 0.57735026918962576450914878050195)
); 

#include lighting.inc
#include parallax.inc

void main (void) {

	
	vec3 V = normalize(v_viewVecTS);
	vec2 P = CalcParallaxOffset(u_Diffuse, v_wTexCoord.xy, V);

	vec3 diffuseMap = texture(u_Diffuse, P).xyz;
	vec3 glowMap = texture(u_Add, P).xyz;
	vec4 normalMap = texture(u_NormalMap, P);
	normalMap.xyz *= 2.0;
	normalMap.xyz -= 1.0;
	diffuseMap += glowMap;

	fragData = vec4(0.0, 0.0, 0.0, 1.0);

	if (u_LightMapType == 0)
		fragData.xyz = diffuseMap * texture(u_LightMap0, v_lTexCoord.xy).rgb;

	if (u_LightMapType == 1) {
	
		normalMap.xyz = normalize(normalMap.xyz);

		vec3 lm0 = texture(u_LightMap0, v_lTexCoord.xy).rgb;
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
#if 0
		// reflection vector, unstable highlight, tends to jump in & out
		vec3 R = reflect(-V, normalMap.xyz);

		vec3 S = vec3(
			dot(R, s_basisVecs[0]),
			dot(R, s_basisVecs[1]),
			dot(R, s_basisVecs[2]));
#else
		// half-angle vector, stable but slower
		vec3 H0 = normalize(V + s_basisVecs[0]);
		vec3 H1 = normalize(V + s_basisVecs[1]);
		vec3 H2 = normalize(V + s_basisVecs[2]);

		vec3 S = vec3(
			dot(normalMap.xyz, H0),
			dot(normalMap.xyz, H1),
			dot(normalMap.xyz, H2));
#endif
//		S = scale * pow(max(S, 0.0), fts);
		S = pow(max(S, 0.0), vec3(u_specularExp));
		S = (lm0 * S.x + lm1 * S.y + lm2 * S.z);

#if 1
		// Approximate energy conservation.
		// Omit division by PI on both diffuse & specular,
		// dividing the latter by 8 instead of (8 * PI).
		// After this, scale by PI because after multiplying by lightmap
		// we actually get N.L * N.H here, not N.H.
		S *= (u_specularExp + 8.0) / (8.0 / PI);

		// The more material is specular, the less it is diffuse.
		// Assume all shiny materials are metals of the same moderate roughness in Q2,
		// treat diffuse map as combined albedo & normal map alpha channel as a rough-to-shiny ratio.
		fragData.xyz = diffuseMap * mix(D, S, normalMap.w * u_specularScale);
#else
		// Non-conserving sum, with normal map alpha channel holding specular brightness.
		fragData.xyz = diffuseMap * D + normalMap.w * u_specularScale * S;
#endif
	}

	if (u_ssao == 1)
		fragData.xyz *= texture2DRect(u_ssaoMap, gl_FragCoord.xy * 0.5).x;

	// fake AO/cavity
	fragData.xyz *= normalMap.z * 0.5 + 0.5;
	fragData.xyz *= u_ColorModulate * u_ambientScale;

// DEBUG
//	if (u_ssao == 1)
//		fragData.xyz = texture2DRect(u_ssaoMap, gl_FragCoord.xy * 0.5).xyz;
}
