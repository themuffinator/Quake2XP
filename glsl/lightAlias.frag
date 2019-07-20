/*
layout (binding = 0) uniform sampler2D		u_bumpMap;
layout (binding = 1) uniform sampler2D		u_diffuseMap;
layout (binding = 2) uniform sampler2D		u_causticMap;
layout (binding = 3) uniform samplerCube	u_CubeFilterMap;
layout (binding = 4) uniform sampler2D		u_rghMap;
layout (binding = 5) uniform sampler2D		u_bumpBlend;
layout (binding = 6) uniform samplerCube	u_skyCube;
*/

layout (bindless_sampler, location  = U_TMU0) uniform sampler2D		u_bumpMap;
layout (bindless_sampler, location  = U_TMU1) uniform sampler2D		u_diffuseMap;
layout (bindless_sampler, location  = U_TMU2) uniform sampler2D		u_causticMap;
layout (bindless_sampler, location  = U_TMU3) uniform samplerCube	u_CubeFilterMap;
layout (bindless_sampler, location  = U_TMU4) uniform sampler2D		u_rghMap;
layout (bindless_sampler, location  = U_TMU5) uniform sampler2D		u_bumpBlend;
layout (bindless_sampler, location  = U_TMU6) uniform samplerCube	u_skyCube;

layout(location = U_SPECULAR_SCALE)		uniform float	u_specularScale;
layout(location = U_CAUSTICS_SCALE)		uniform float	u_CausticsModulate;
layout(location = U_COLOR_MUL)			uniform float	u_ColorModulate;
layout(location = U_COLOR)				uniform vec4	u_LightColor;
layout(location = U_USE_FOG)			uniform int		u_fog;
layout(location = U_FOG_DENSITY)		uniform float	u_fogDensity;
layout(location = U_USE_CAUSTICS)		uniform int		u_isCaustics;
layout(location = U_AMBIENT_LIGHT)		uniform int		u_isAmbient;
layout(location = U_USE_RGH_MAP)		uniform int		u_isRgh;
layout(location = U_SPOT_LIGHT)			uniform int		u_spotLight;
layout(location = U_SPOT_PARAMS)		uniform vec3	u_spotParams;
layout(location = U_USE_AUTOBUMP)		uniform int		u_autoBump;
layout(location = U_AUTOBUMP_PARAMS)	uniform vec2	u_autoBumpParams; // x - bump scale y - specular scale
layout(location = U_PARAM_INT_0)		uniform int		u_blinnPhong; // use old lighting model
layout(location = U_PARAM_INT_1)		uniform int		u_alphaMask;
layout(location = U_PARAM_INT_2)		uniform int		u_useSSS;
layout(location = U_PARAM_INT_3)		uniform int		u_useSkyRefl;

in vec2			v_texCoord;
in vec3			v_viewVec;
in vec3			v_lightVec;
in vec4			v_CubeCoord;
in vec4			v_positionVS;
in vec3			v_lightAtten;
in vec3			v_lightSpot;
in vec3			v_tangent;
in vec3			v_tst;

#include lighting.inc

void main (void) {

	float attenMap = PointAttenuation(v_lightAtten, 2.0);

	if(u_spotLight == 1)
		attenMap *= ConeAttenuation(v_lightSpot, u_spotParams);

	if(attenMap <= CUTOFF_EPSILON){
		discard;
		return;
	}
	
	if(u_alphaMask == 1 && u_isRgh == 1){
		float mask = texture(u_rghMap, v_texCoord).g;
		if (mask <= 0.01) {
			discard;
			return;
		}
	}

	vec3 L = normalize(v_lightVec);
	vec3 V = normalize(v_viewVec);

	vec4 normalMap = vec4(0.0);
	vec3 specular = vec3(0.0);

	vec4 cubeFilter = texture(u_CubeFilterMap, v_CubeCoord.xyz) * 2.0;
	vec4 diffuseMap  = texture(u_diffuseMap, v_texCoord);
	
	if(u_autoBump == 0){
		normalMap.xyz = normalize(texture(u_bumpMap, v_texCoord).rgb * 2.0 - 1.0);
		specular.rgb = texture(u_bumpMap, v_texCoord).aaa * u_specularScale;
	}

	if(u_autoBump == 1){
		normalMap = Height2Normal(v_texCoord, u_diffuseMap, diffuseMap.rgb, u_autoBumpParams.x, u_autoBumpParams.y);
		specular.rgb = normalMap.aaa;
	}		
		
	float SSS = diffuseMap.a;
	vec3 nm = texture(u_bumpMap,   v_texCoord).xyz * vec3( 2.0,  2.0, 2.0) + vec3(-1.0, -1.0,  0.0);
	vec3 dt = texture(u_bumpBlend, v_texCoord).xyz * vec3(-2.0, -2.0, 2.0) + vec3( 1.0,  1.0, -1.0);
	vec3 r = normalize(nm * dot(nm, dt) - dt * nm.z);
	vec3 blendNormal =  r * 0.5 + 0.5;
	vec4 skin_color = SkinLighting(V, L, blendNormal, u_LightColor.rgb, diffuseMap, attenMap, specular.r);


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
	
	float roughness, cd_mask, metalness;
	
	if(u_isRgh == 1){
		vec4 rghMap = texture(u_rghMap, v_texCoord);
		roughness = rghMap.r; 
		metalness =   rghMap.g;
		cd_mask =  rghMap.a;
	}

	if(u_isRgh == 0){
		roughness = 1.0 - diffuseMap.r;
      cd_mask = 1.0; 
      metalness = 0.0;
    }
  
  roughness = clamp(roughness, 0.001, 1.0);
       
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
			vec3 metall_color;
			if(u_useSSS == 1)
				metall_color = SubScateringLighting(V, L, normalMap.xyz, diffuseMap.rgb, specular.r)  * u_LightColor.rgb * cubeFilter.rgb * attenMap;
			else{
			if(u_blinnPhong == 1)
				metall_color = BlinnPhongLighting(diffuseMap.rgb, specular.r, normalMap.rgb, L, V, 128.0)  * u_LightColor.rgb * cubeFilter.rgb * attenMap; 
			if(u_blinnPhong == 0)
				metall_color = Lighting_BRDF(diffuseMap.rgb, specular, roughness, normalMap.xyz, L, V)  * u_LightColor.rgb * cubeFilter.rgb * attenMap; 
			}		

			fragData = mix(skin_color, vec4(metall_color, 1.0), SSS);	

/*		if(u_useSkyRefl == 1 && SSS  > 0.0){
			vec3 reflCoord = v_tst.xyz;
			vec3 envSky = textureLod(u_skyCube, reflCoord.xyz, roughness * 14.0).rgb;
			float frenel = pow(1.0 - abs(dot(V, normalMap.rgb)), 2.0);
			fragData.rgb += mix(envSky * metalness, fragData.rgb,  frenel);
		}
	*/

      if(cd_mask == 0.0){	    
        
      vec2 uv = v_texCoord.xy * 2.0 - 1.0;         
      vec2 uv_orthogonal = normalize(uv);
      vec3 uv_tangent = vec3(-uv_orthogonal.y, 0, uv_orthogonal.x);

        fragData.rgb +=LightingDiffraction(V, L, uv_tangent);
       }
      }	
	  fragData.a = 1.0;
}