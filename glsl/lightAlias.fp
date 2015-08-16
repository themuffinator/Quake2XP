uniform sampler2D		u_bumpMap;
uniform sampler2D		u_diffuseMap;
uniform sampler2D		u_causticMap;
uniform samplerCube		u_CubeFilterMap;
uniform sampler3D	 	u_attenMap;

uniform float			u_LightRadius;
uniform float			u_specularScale;
//uniform float			u_specularExp;
uniform float			u_CausticsModulate;
uniform vec4			u_LightColor;
uniform int				u_fog;
uniform float			u_fogDensity;
uniform int				u_isCaustics;
uniform int				u_isAmbient;

varying vec2			v_texCoord;
varying vec3			v_viewVec;
varying vec3			v_lightVec;
varying vec4			v_CubeCoord;
varying vec4			v_AttenCoord;
varying vec4			v_positionVS;

// FIXME: give uniform
#define u_specularExp		16.0

#include lighting.inc

void main (void) {
	float attenMap = texture3D(u_attenMap, v_AttenCoord).r;

	if(attenMap <= CUTOFF_EPSILON){
		discard;
		return;
	}

	vec3 L = normalize(v_lightVec);
	vec3 V = normalize(v_viewVec);

	vec4 diffuseMap  = texture2D(u_diffuseMap, v_texCoord);
	vec4 normalMap =  texture2D(u_bumpMap, v_texCoord);
	normalMap.xyz *= 2.0;
	normalMap.xyz -= 1.0;

  float SSS = diffuseMap.a;

	vec4 cubeFilter = textureCube(u_CubeFilterMap, v_CubeCoord.xyz) * 2.0;

	if (u_isCaustics == 1){
		vec4 causticsMap = texture2D(u_causticMap, v_texCoord);
		vec4 tmp = causticsMap * diffuseMap * u_CausticsModulate;
		diffuseMap += tmp;
	}

	if (u_isAmbient == 1) {
		gl_FragColor = diffuseMap * LambertLighting(normalize(normalMap.xyz), V) * u_LightColor * attenMap;
		return;
	}

	if (u_isAmbient == 0) {

		float specular = normalMap.a * u_specularScale;
		vec2 Es = PhongLighting(normalize(normalMap.xyz), L, V, u_specularExp);

		if(u_fog == 1) {  
			float fogCoord = abs(gl_FragCoord.z / gl_FragCoord.w);
			float fogFactor = exp(-u_fogDensity * fogCoord); //exp1
			//float fogFactor = exp(-pow(u_fogDensity * fogCoord, 2.0)); //exp2

			vec4 color = (Es.x * diffuseMap + Es.y * specular) * u_LightColor * cubeFilter * attenMap;
			gl_FragColor = mix(u_LightColor, color, fogFactor) * attenMap; // u_LightColor == fogColor

			return;
		}

		if(u_fog == 0) {

			if(SSS <= 0.00392){
					gl_FragColor = subScatterFS(V, L, normalize(normalMap.xyz), u_LightColor, diffuseMap, attenMap, specular) * cubeFilter;
					return;
			}
			else	
					gl_FragColor = (Es.x * diffuseMap + Es.y * specular) * u_LightColor * cubeFilter * attenMap;
	}
}
}
