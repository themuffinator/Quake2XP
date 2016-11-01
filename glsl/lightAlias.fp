layout (binding = 0) uniform sampler2D		u_bumpMap;
layout (binding = 1) uniform sampler2D		u_diffuseMap;
layout (binding = 2) uniform sampler2D		u_causticMap;
layout (binding = 3) uniform samplerCube	u_CubeFilterMap;
layout (binding = 4) uniform sampler2D		u_rghMap;
layout (binding = 5) uniform sampler2D		u_bumpBlend;

uniform float	u_LightRadius;
uniform float	u_specularScale;
uniform float	u_CausticsModulate;
uniform float	u_ColorModulate;
uniform vec4	u_LightColor;
uniform int		u_fog;
uniform float	u_fogDensity;
uniform int		u_isCaustics;
uniform int		u_isAmbient;
uniform int		u_isRgh;
uniform int		u_spotLight;
uniform vec3	u_spotParams;

in vec2			v_texCoord;
in vec3			v_viewVec;
in vec3			v_lightVec;
in vec4			v_CubeCoord;
in vec4			v_positionVS;
in vec3			v_lightAtten;
in vec3			v_lightSpot;
in vec3			v_lightDirection;

#include lighting.inc

void main (void) {

	float attenMap = PointAttenuation(v_lightAtten, 2.0);

	if(u_spotLight == 1)
		attenMap *= ConeAttenuation(v_lightSpot, u_spotParams);

	if(attenMap <= CUTOFF_EPSILON){
		discard;
		return;
	}

	vec3 L = normalize(v_lightVec);
	vec3 V = normalize(v_viewVec);
	
	vec4 cubeFilter = texture(u_CubeFilterMap, v_CubeCoord.xyz) * 2.0;
	vec4 diffuseMap  = texture(u_diffuseMap, v_texCoord);
	vec4 normalMap =  texture(u_bumpMap, v_texCoord);
	normalMap.xyz *= 2.0;
	normalMap.xyz -= 1.0;
	normalMap.xyz = normalize(normalMap).xyz;

	float specular = normalMap.a * u_specularScale;
			
	float SSS = diffuseMap.a;
	vec3 nm = texture(u_bumpMap,   v_texCoord).xyz * vec3( 2.0,  2.0, 2.0) + vec3(-1.0, -1.0,  0.0);
	vec3 dt = texture(u_bumpBlend, v_texCoord).xyz * vec3(-2.0, -2.0, 2.0) + vec3( 1.0,  1.0, -1.0);
	vec3 r = normalize(nm * dot(nm, dt) - dt * nm.z);
	vec3 blendNormal =  r * 0.5 + 0.5;
	vec4 skin_color = SkinLighting(V, L, blendNormal, u_LightColor.rgb, diffuseMap * 0.5, attenMap, specular);


	if (u_isCaustics == 1){
		vec4 causticsMap = texture(u_causticMap, v_texCoord);
		vec4 tmp = causticsMap * diffuseMap * u_CausticsModulate;
		diffuseMap += tmp;
	}

	if (u_isAmbient == 1) {
		vec3 curNormal = mix(blendNormal, normalMap.rgb, SSS);
		fragData = diffuseMap * u_ColorModulate * LambertLighting(curNormal, L) * u_LightColor * attenMap;
		return;
	}
	
	if (u_isAmbient == 0) {
	
	float roughness;
	
	if(u_isRgh == 1){
		vec4 rghMap = texture(u_rghMap, v_texCoord);
		roughness = rghMap.r * 1.5;
    roughness = clamp(roughness, 0.1, 1.0);
	}

	if(u_isRgh == 0){
		roughness = 1.0 - diffuseMap.r * 1.35;
    roughness = clamp(roughness, 0.1, 1.0);
    }

	vec3 brdf =  Lighting_BRDF(diffuseMap.rgb, vec3(specular), roughness, normalMap.xyz, L, V);
	vec3 brdfColor = brdf * u_LightColor.rgb;

		if(u_fog == 1) {  
			float fogCoord = abs(gl_FragCoord.z / gl_FragCoord.w);
			float fogFactor = exp(-u_fogDensity * fogCoord); //exp1
			//float fogFactor = exp(-pow(u_fogDensity * fogCoord, 2.0)); //exp2

			vec3 tmp =  (Diffuse_Lambert(diffuseMap.rgb) * u_LightColor.rgb) * (normalMap.z * 0.5 + 0.5); //  multiplied by fake AO

	//		vec4 tmp = mix(skin_color, vec4(brdfColor, 1.0), SSS);
			fragData = mix(u_LightColor, vec4(tmp, 1.0), fogFactor) * attenMap; // u_LightColor == fogColor
			return;
		}

			skin_color *= cubeFilter;
			vec3 metall_color = brdfColor * cubeFilter.rgb * attenMap; 
			fragData = mix(skin_color, vec4(metall_color, 1.0), SSS);			
	}
}