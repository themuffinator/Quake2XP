layout (binding = 0) uniform sampler2D		u_Diffuse;
layout (binding = 1) uniform sampler2D		u_NormalMap;
layout (binding = 2) uniform samplerCube	u_CubeFilterMap;
layout (binding = 3) uniform sampler3D	 	u_attenMap;
layout (binding = 4) uniform sampler2D		u_Caustics;

uniform float	u_ColorModulate;
uniform float	u_specularScale;
uniform float	u_specularExp;
uniform vec4 	u_LightColor;
uniform float 	u_LightRadius;
uniform int		u_fog;
uniform float	u_fogDensity;
uniform float	u_CausticsModulate; 
uniform int		u_isCaustics;
uniform int		u_isAmbient;

in vec3			v_positionVS;
in vec3			v_viewVecTS;
in vec3			v_lightVec;
in vec2			v_texCoord;
in vec4			v_CubeCoord;
in vec4			v_lightCoord;
in vec4			v_AttenCoord;

#include lighting.inc
#include parallax.inc

void main (void) {
	float attenMap = texture(u_attenMap, v_AttenCoord.xyz).r;

	if(attenMap <= CUTOFF_EPSILON){
		discard;
		return;
	}
 
	vec3	V = normalize(v_viewVecTS);
	vec3	L = normalize(v_lightVec);
	vec2  P = CalcParallaxOffset(u_Diffuse, v_texCoord, V);

	vec4 diffuseMap = texture(u_Diffuse,  P);
	vec4 normalMap =  texture(u_NormalMap, P);
	normalMap.xyz *= 2.0;
	normalMap.xyz -= 1.0;

	// light filter
	vec4 cubeFilter = texture(u_CubeFilterMap, v_CubeCoord.xyz) * 2.0;

	if (u_isCaustics == 1) {
		vec4 causticsMap = texture(u_Caustics, P);
		vec4 tmp = causticsMap * diffuseMap;
		tmp *= u_CausticsModulate;
		diffuseMap = tmp + diffuseMap;
	}

	if(u_isAmbient == 1) {
		fragData = diffuseMap * LambertLighting(normalize(normalMap.xyz), L) * u_LightColor * attenMap;
		return;
	}

	if(u_isAmbient == 0) {
		float specular = normalMap.a * u_specularScale;
		vec2 Es = PhongLighting(normalize(normalMap.xyz), L, V, u_specularExp);

		if(u_fog == 1) {
			float fogCoord = abs(gl_FragCoord.z / gl_FragCoord.w); // = gl_FragCoord.z / gl_FragCoord.w;
			float fogFactor = exp(-u_fogDensity * fogCoord); //exp1
			//float fogFactor = exp(-pow(u_fogDensity * fogCoord, 2.0)); //exp2

			vec4 color = (Es.x * diffuseMap + Es.y * specular) * u_LightColor * cubeFilter * attenMap;
			fragData = mix(u_LightColor, color, fogFactor) * attenMap;  // u_LightColor == fogColor
			return;
		}

		if(u_fog == 0)
			fragData = (Es.x * diffuseMap + Es.y * specular) * u_LightColor * cubeFilter * attenMap;
	} 
}