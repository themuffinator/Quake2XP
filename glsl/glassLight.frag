//!#include "include/global.inc"

layout (bindless_sampler, location  = U_TMU0) uniform sampler2D		u_Diffuse;
layout (bindless_sampler, location  = U_TMU1) uniform sampler2D		u_NormalMap;
layout (bindless_sampler, location  = U_TMU2) uniform samplerCube	u_CubeFilterMap;
layout (bindless_sampler, location  = U_TMU3) uniform sampler2D		u_Caustics;

layout(location = U_COLOR)				uniform vec4 	u_LightColor;
layout(location = U_USE_FOG)			uniform int		u_fog;
layout(location = U_FOG_DENSITY)		uniform float	u_fogDensity;
layout(location = U_CAUSTICS_SCALE)		uniform float	u_CausticsModulate; 
layout(location = U_USE_CAUSTICS)		uniform int		u_isCaustics;
layout(location = U_AMBIENT_LIGHT)		uniform int		u_isAmbient;
layout(location = U_SPOT_LIGHT)			uniform int		u_spotLight;
layout(location = U_SPOT_PARAMS)		uniform vec3	u_spotParams;
layout(location = U_PARAM_FLOAT_3)		uniform float	u_alpha;

in vec3			v_positionVS;
in vec3			v_viewVecTS;
in vec3			v_lightVec;
in vec2			v_texCoord;
in vec4			v_CubeCoord;
in vec4			v_lightCoord;
in vec3			v_lightAtten;
in vec3			v_lightSpot;

#include lighting.inc   //!#include "include/lighting.inc"
#include parallax.inc   //!#include "include/parallax.inc"

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
	vec2 tc = ReliefMapping(u_Diffuse, v_texCoord, V);

	vec4 diffuseMap = texture(u_Diffuse,  tc);
	vec3 N =  normalize(texture(u_NormalMap, tc).rgb * 2.0 - 1.0);
	float specular = texture(u_NormalMap, tc).a;
	vec4 cubeFilter = texture(u_CubeFilterMap, v_CubeCoord.xyz) * 2.0;

	if (u_isCaustics == 1) {
		vec4 causticsMap = texture(u_Caustics, tc);
		vec4 tmp = causticsMap * diffuseMap;
		tmp *= u_CausticsModulate;
		diffuseMap = tmp + diffuseMap;
	}

	if(u_isAmbient == 1) {
		fragData = diffuseMap * LambertLighting(N, L) * u_LightColor * attenMap;
		return;
	}

	if(u_isAmbient == 0) {
		vec2 Es = PhongLighting (N, L, V, 32.0);
		vec3 glassColor =  (Es.x * diffuseMap.rgb + Es.y * specular) * u_LightColor.rgb;
		
		if(u_fog == 1) {  
			float fogCoord = abs(gl_FragCoord.z / gl_FragCoord.w); // = gl_FragCoord.z / gl_FragCoord.w;
			float fogFactor = exp(-u_fogDensity * fogCoord); //exp1

			fragData = mix(u_LightColor, vec4(glassColor, 1.0), fogFactor) * attenMap;
			fragData.rgb *= u_alpha;
			fragData.a = 1.0;
			return;
		}
     
		if(u_fog == 0) {         
		fragData.rgb =  glassColor  * attenMap * cubeFilter.rgb; 
		fragData.rgb *= u_alpha;
		fragData.a = 1.0;
     }
  }	
}