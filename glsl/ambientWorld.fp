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
in vec3			v_tbn[3];
 
//
// 3-vector radiosity basis for normal mapping
//
const vec3 s_basisVecs[3]=vec3[](
vec3(0.81649658092772603273242802490196,  0.                                , 0.57735026918962576450914878050195),
vec3(-0.40824829046386301636621401245098,  0.70710678118654752440084436210485, 0.70710678118654752440084436210485),
vec3(-0.40824829046386301636621401245098, -0.70710678118654752440084436210485, 0.57735026918962576450914878050195)
); 

#include lighting.inc
#include parallax.inc

void main (void) {

	vec3 V = normalize(v_viewVecTS);
	vec2 P = CalcParallaxOffset(u_Diffuse, v_wTexCoord.xy, V);

	vec3 diffuseMap = texture2D(u_Diffuse, P).xyz;
	vec3 glowMap = texture2D(u_Add, P).xyz;
	vec4 normalMap = texture2D(u_NormalMap, P);
	normalMap.xyz *= 2.0;
	normalMap.xyz -= 1.0;
	diffuseMap += glowMap;

	if (u_LightMapType == 0)
		gl_FragColor.xyz = diffuseMap * texture2D(u_LightMap0, v_lTexCoord.xy).rgb;

	if (u_LightMapType == 1) {

		normalMap.xyz = normalize(normalMap.xyz);

		vec3 lm0 = texture2D(u_LightMap0, v_lTexCoord.xy).rgb;
		vec3 lm1 = texture2D(u_LightMap1, v_lTexCoord.xy).rgb;
		vec3 lm2 = texture2D(u_LightMap2, v_lTexCoord.xy).rgb;

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
		S = pow(max(S, 0.0), vec3(u_specularExp));
		S = (lm0 * S.x + lm1 * S.y + lm2 * S.z);

		gl_FragColor.xyz = diffuseMap * mix(D, S, normalMap.w * u_specularScale);

	}

	if (u_ssao == 1)
		gl_FragColor.xyz *= texture2DRect(u_ssaoMap, gl_FragCoord.xy * 0.5).x;

	// fake AO/cavity
	gl_FragColor.xyz *= normalMap.z * 0.5 + 0.5;
	gl_FragColor.xyz *= u_ColorModulate * u_ambientScale;
	gl_FragColor.w = 1.0;

// DEBUG
//	if (u_ssao == 1)
//		gl_FragColor.xyz = texture2DRect(u_ssaoMap, gl_FragCoord.xy * 0.5).xyz;
}
