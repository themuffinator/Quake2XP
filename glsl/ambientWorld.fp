#extension GL_ARB_texture_rectangle : enable

uniform sampler2D		u_Diffuse;
uniform sampler2D		u_NormalMap;
uniform sampler2D		u_Add;
uniform sampler2D		u_csmMap;

uniform sampler2D		u_LightMap0;
uniform sampler2D		u_LightMap1;
uniform sampler2D		u_LightMap2;
uniform int			u_LightMapType;

//uniform int			u_ssao;
//uniform sampler2DRect		u_ssaoMap;

uniform float			u_ColorModulate;
uniform float			u_ambientScale;    

uniform float			u_specularScale;
uniform float			u_specularExp;

varying vec3			v_positionVS;
varying vec3			v_viewVecTS;
varying vec2			v_wTexCoord;
varying vec2			v_lTexCoord;
varying vec2			v_envCoord;
varying vec3			v_tbn[3];
 
//
// 3-vector radiosity basis for normal mapping
//
const vec3 s_basisVecs[3] = {
	vec3( 0.81649658092772603273242802490196,  0.                                , 0.57735026918962576450914878050195),
	vec3(-0.40824829046386301636621401245098,  0.70710678118654752440084436210485, 0.57735026918962576450914878050195),
	vec3(-0.40824829046386301636621401245098, -0.70710678118654752440084436210485, 0.57735026918962576450914878050195)
};

#include lighting.inc
#include parallax.inc

void main (void) {
	vec3 V;

	if (u_parallaxType > 0 || u_LightMapType == 1)
		V = normalize(v_viewVecTS);

	vec2 P = v_wTexCoord.xy;

	if(u_parallaxType == 1) {
		vec3 Vp = normalize(v_positionVS);
		vec3 T = normalize(v_tbn[0]);
		vec3 B = normalize(v_tbn[1]);
		vec3 N = normalize(v_tbn[2]);

		// ray intersection in view direction
		float a = abs(dot(N, Vp));
		a = sin(clamp(a, 0.0, 1.0) * HALF_PI) / a;	// thx Berserker for corner artifact correction

		vec3 dp = vec3(v_wTexCoord, 0.0);
		vec3 ds = vec3(a * u_parallaxParams.x * dot(T, Vp), a * u_parallaxParams.y * dot(B, Vp), 1.0);
		float distFactor = 0.05 * sqrt(length(fwidth(v_wTexCoord)));

		IntersectConeExp(u_csmMap, dp, ds, distFactor);

		P = dp.xy;
	}

	if(u_parallaxType == 2)
		P = CalcParallaxOffset(u_Diffuse, v_wTexCoord.xy, V);

	vec4 diffuseMap = texture2D(u_Diffuse, P);
	vec4 glowMap = texture2D(u_Add, P);
	vec4 normalMap = texture2D(u_NormalMap, P);
	normalMap.xyz *= 2.0;
	normalMap.xyz -= 1.0;

	if (u_LightMapType == 0)
		gl_FragColor.xyz = diffuseMap.rgb * texture2D(u_LightMap0, v_lTexCoord.xy).rgb;

	if (u_LightMapType == 1) {
/*
		// calculate Toksvig factor
		// FIXME: need to precompute
		float len = length(normalMap.xyz);
		float ft = len / mix(u_specularExp, 1.0, len);
		float fts = ft * u_specularExp;
		float scale = (1.0 + fts) / (1.0 + u_specularExp);
		normalMap.xyz /= len;
*/
		normalMap.xyz = normalize(normalMap.xyz);
//		normalMap.xyz = vec3(0.0, 0.0, 1.0);

		vec3 lm0 = texture2D(u_LightMap0, v_lTexCoord.xy).rgb;
		vec3 lm1 = texture2D(u_LightMap1, v_lTexCoord.xy).rgb;
		vec3 lm2 = texture2D(u_LightMap2, v_lTexCoord.xy).rgb;

		// diffuse
		vec3 D = vec3(
			dot(normalMap.xyz, s_basisVecs[0]),
			dot(normalMap.xyz, s_basisVecs[1]),
			dot(normalMap.xyz, s_basisVecs[2]));

		gl_FragColor.xyz = diffuseMap.rgb * (lm0 * D.x + lm1 * D.y + lm2 * D.z);

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
		S = pow (max(S, 0.0), u_specularExp);

		gl_FragColor.xyz += normalMap.a * (lm0 * S.x + lm1 * S.y + lm2 * S.z);
	}

//	if (u_ssao == 1)
//		gl_FragColor.xyz *= texture2DRect(u_ssaoMap, gl_FragCoord.xy).x;

	// fake AO/cavity, don't care about re-normalization
	gl_FragColor.xyz *= normalMap.z * 0.5 + 0.5;

	gl_FragColor.xyz += glowMap.rgb;
	gl_FragColor.xyz *= u_ColorModulate * u_ambientScale;
	gl_FragColor.w = 1.0;
}
