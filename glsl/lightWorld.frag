//!#include "include/global.inc"

layout (bindless_sampler, location  = U_TMU0) uniform sampler2D		u_Diffuse;
layout (bindless_sampler, location  = U_TMU1) uniform sampler2D		u_NormalMap;
layout (bindless_sampler, location  = U_TMU2) uniform samplerCube	u_CubeFilterMap;
layout (bindless_sampler, location  = U_TMU3) uniform sampler2D		u_Caustics;
layout (bindless_sampler, location  = U_TMU4) uniform sampler2D		u_RghMap;
layout (bindless_sampler, location  = U_TMU5) uniform sampler2DRect	u_SSAOMap;
layout (bindless_sampler, location  = U_TMU6) uniform samplerCube	u_shadowMap;
layout (bindless_sampler, location  = U_TMU7) uniform sampler2DRect	u_DepthBuffer;

layout(location = U_COLOR)				uniform vec4 	u_LightColor;
layout(location = U_USE_FOG)			uniform int		u_fog;
layout(location = U_FOG_DENSITY)		uniform float	u_fogDensity;
layout(location = U_CAUSTICS_SCALE)		uniform float	u_CausticsModulate; 
layout(location = U_USE_CAUSTICS)		uniform int		u_isCaustics;
layout(location = U_AMBIENT_LIGHT)		uniform int		u_isAmbient;
layout(location = U_USE_RGH_MAP)		uniform int		u_isRgh;
layout(location = U_USE_AUTOBUMP)		uniform int		u_autoBump;
layout(location = U_SPOT_LIGHT)			uniform int		u_spotLight;
layout(location = U_SPOT_PARAMS)		uniform vec3	u_spotParams;
layout(location = U_AUTOBUMP_PARAMS)	uniform vec2	u_autoBumpParams; // x - bump scale y - specular scale
layout(location = U_PARAM_INT_0)		uniform int		u_sss;
layout(location = U_PARAM_INT_1)		uniform int		u_selfShadow; // self shadow parallax
layout(location = U_PARAM_INT_2)		uniform int		u_blinnPhong; // use old lighting model
layout(location = U_PARAM_INT_3)		uniform int		u_useShadowMap;
layout(location = U_USE_SSAO)			uniform int		u_ssao;
layout(location = U_PARAM_VEC2_0)		uniform vec2	u_shadowBiasScale;
layout(location = U_PARAM_VEC2_1)		uniform vec2	u_jitterOffset;
layout(location = U_TEXTURE0_MATRIX)	uniform mat4	u_UnprojectMatrix;

in vec3		v_positionVS;
in vec3		v_viewVecTS;
in vec3		v_lightVec;
in vec2		v_texCoord;
in vec4		v_CubeCoord;
in vec4		v_lightCoord;
in vec3		v_lightAtten;
in vec3		v_lightSpot;
in vec3		v_ViewOrg;
in vec3		v_LightOrg;

#include lighting.inc   //!#include "include/lighting.inc"
#include parallax.inc   //!#include "include/parallax.inc"


void MakeNormalVectors(const vec3 forward, inout vec3 right, inout vec3 up){
	// this rotate and negate guarantees a vector not colinear with the original
	right = vec3(forward.z, -forward.x, forward.y);
	right -= forward * dot(right, forward);
	up = cross(normalize(right), forward);
}

float shadowCube(const vec3 I, in float vertexDistance, const float l){
	vec3 forward, right, up;
	forward = normalize(I);
	MakeNormalVectors(forward, right, up);

	vertexDistance -= u_shadowBiasScale.x + l * (1.0 / 512.0);	// bias
	mat2 rmat = randomRotation(pow(gl_FragCoord.xy, u_jitterOffset)) * u_shadowBiasScale.y;
	float shadow = 0.0;
	for(int i = 0; i < NUM_OF_TAPS; i++){
			vec2 offset = rmat * poissonDisk[i];
			vec3 jitter = forward + right * offset.x + up * offset.y;
			shadow += clamp(texture(u_shadowMap, jitter).r - vertexDistance, 0.0, 1.0);
	}
	return shadow * ONE_OVER_NUM_OF_TAPS;
}

void main (void) {

	float attenMap = PointAttenuation(v_lightAtten, 2.0);

	if(u_spotLight == 1)
		attenMap *= ConeAttenuation(v_lightSpot, u_spotParams);
  
	if(attenMap <= CUTOFF_EPSILON){
		discard;
		return;
	}

	vec3	V = normalize(v_viewVecTS);
	vec3	L = normalize(v_lightVec);

	float shadowMap = 1.0;
	if(u_useShadowMap == 1){
		// reconstruct vertex position in world space
		float depth = texture(u_DepthBuffer, gl_FragCoord.xy).r;

		vec4 P = u_UnprojectMatrix * vec4(gl_FragCoord.xy, depth, 1.0);
		P.xyz /= P.w;
		// compute incident ray
		vec3 I = v_LightOrg - P.xyz;
		float dist = length(I);

		// compute view direction in world space
		vec3 Vv = v_ViewOrg - P.xyz;
		float l = length(Vv);
		shadowMap = shadowCube(-I, dist, l);
		if (shadowMap < 0.004){
			discard;
			return;
		}
	}
	vec4 diffuseMap;
	vec4 normalMap;
	vec2 texCoord;
	
	if(u_autoBump == 0){

		switch (u_parallaxType) {
			case 0:
			texCoord = v_texCoord;
			break;
			case 1: 
			texCoord = parallaxMapping(u_Diffuse, v_texCoord, V);
			break;
			case 2:
			texCoord = ReliefMapping(u_Diffuse, v_texCoord, V);
			break;
		}
		diffuseMap = texture(u_Diffuse,  texCoord);
		normalMap.rgb =  normalize(texture(u_NormalMap, texCoord).rgb * 2.0 - 1.0);
	}

	if(u_autoBump == 1){
		diffuseMap = texture(u_Diffuse,  v_texCoord);	
		normalMap = Height2Normal(v_texCoord, u_Diffuse, diffuseMap.rgb, u_autoBumpParams.x, u_autoBumpParams.y);
	}

	// light filter
	vec4 cubeFilter = texture(u_CubeFilterMap, v_CubeCoord.xyz) * 2.0;

	if (u_isCaustics == 1) {
		vec4 causticsMap = texture(u_Caustics, texCoord);
		vec4 tmp = causticsMap * diffuseMap;
		tmp *= u_CausticsModulate;
		diffuseMap = tmp + diffuseMap;
	}

	if(u_isAmbient == 1) {
		fragData = diffuseMap * LambertLighting(normalMap.xyz, L) * u_LightColor * attenMap;
		return;
	}

	if(u_isAmbient == 0) {
		
		float roughness;
		float specular = texture(u_NormalMap, texCoord).a;
    
		if(u_isRgh == 1){
			roughness = texture(u_RghMap, texCoord).r;
		}
    if(u_isRgh != 1)
		{
		roughness = 1.0 - diffuseMap.r;
		roughness = clamp(roughness, 0.1, 1.0);
		}

		vec3 brdf;
		
		if(u_sss == 1)
			brdf =  SubScateringLighting(V, L, normalMap.xyz, diffuseMap.rgb, specular);

		if(u_sss == 0){

		if(u_blinnPhong == 0)	
			brdf =  Lighting_BRDF(diffuseMap.rgb, vec3(specular), roughness, normalMap.xyz, L, V);

		if(u_blinnPhong == 1)
			brdf = BlinnPhongLighting(diffuseMap.rgb, specular, normalMap.rgb, L, V, 128.0);
		}

		vec3 brdfColor = brdf * u_LightColor.rgb;
          
		if(u_fog == 1) {  
			float fogCoord = abs(gl_FragCoord.z / gl_FragCoord.w); // = gl_FragCoord.z / gl_FragCoord.w;
			float fogFactor = exp(-u_fogDensity * fogCoord); //exp1

			fragData = mix(u_LightColor, vec4(brdfColor, 1.0), fogFactor) * attenMap * shadowMap;  // u_LightColor == fogColor
			return;
		}
     
		if(u_fog == 0) { 
		  
		float shadow = 1.0;
		if(u_selfShadow == 1)
			shadow = selfShadow(u_Diffuse, L, texCoord);
        
		fragData.rgb =  brdfColor  * attenMap * shadow * cubeFilter.rgb * shadowMap; 
		if(u_ssao == 1)
			fragData.rgb *= texture(u_SSAOMap, gl_FragCoord.xy * 0.5).rgb;

		fragData.a = 1.0;
     }
  }	
}