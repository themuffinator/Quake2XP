
uniform sampler2D		u_Diffuse;
uniform sampler2D		u_NormalMap;
uniform samplerCube		u_CubeFilterMap;
uniform sampler3D	 	u_attenMap;
uniform sampler2D		u_Caustics;
uniform sampler2D		u_csmMap;

uniform float			u_ColorModulate;
uniform float			u_specularScale;
uniform float			u_specularExp;
uniform float			u_toksvigFactor;
uniform vec4 			u_LightColor;
uniform float 			u_LightRadius;
uniform int				u_fog;
uniform float			u_fogDensity;
uniform float			u_CausticsModulate; 
uniform int				u_isCaustics;
uniform int				u_isAmbient;

varying vec3			v_tbn[3];
varying vec4			v_positionVS;
varying vec3			v_viewVecTS;
varying vec3			v_lightVec;
varying vec2			v_texCoord;
varying vec4			v_CubeCoord;
varying vec4			v_lightCoord;
varying vec3			v_AttenCoord;

#include lighting.inc
#include parallax.inc

void main ()
{

float attenMap = texture3D(u_attenMap, v_AttenCoord).r;

if(attenMap <= CUTOFF_EPSILON){
	discard;
		return;
}
 
vec3	V = normalize(v_viewVecTS);
vec3	L = normalize(v_lightVec);
vec3	Vp = normalize(v_positionVS);
vec3	T = normalize(v_tbn[0]);
vec3	B = normalize(v_tbn[1]);
vec3	N = normalize(v_tbn[2]);
vec4	diffuseMap;
vec3	normalMap;
float	specTmp;
vec4	causticsMap;
vec2	P = v_texCoord.xy;

if(u_parallaxType == 1){
	// ray intersection in view direction
	float a = abs(dot(N, Vp));
	a = sin(clamp(a, 0.0, 1.0) * HALF_PI) / a;	// thx Berserker for corner artifact correction
	vec3 dp = vec3(v_texCoord, 0.0);
	vec3 ds = vec3(a * u_parallaxParams.x * dot(T, Vp), a * u_parallaxParams.y * dot(B, Vp), 1.0);
	float distFactor = 0.05 * sqrt(length(fwidth(v_texCoord)));
	IntersectConeExp(u_csmMap, dp, ds, distFactor);
	P = dp.xy;
}

if(u_parallaxType == 2)	{
	P = CalcParallaxOffset(u_Diffuse, v_texCoord.xy, V);
}

diffuseMap = texture2D(u_Diffuse,  P);
normalMap =  normalize(texture2D(u_NormalMap, P).rgb * 2.0 - 1.0);
specTmp = texture2D(u_NormalMap, P).a;
causticsMap = texture2D(u_Caustics, P);

float specular = specTmp * u_specularScale;
specular /= mix(u_toksvigFactor, 1.0, specular);

// light filter
vec4 cubeFilter = textureCube(u_CubeFilterMap, v_CubeCoord.xyz);
cubeFilter *= 2;

if (u_isCaustics == 1){
vec4 tmp = causticsMap * diffuseMap;
tmp *= u_CausticsModulate;
diffuseMap = tmp + diffuseMap;
}

if(u_isAmbient == 1){
	gl_FragColor = diffuseMap * LambertLighting(normalMap, L) * u_LightColor * attenMap;
	return;
}

	if(u_isAmbient == 0){
		vec2 Es = PhongLighting(normalMap, L, V, u_specularExp);

			if(u_fog == 1){
				float fogCoord = abs(gl_FragCoord.z / gl_FragCoord.w); // = gl_FragCoord.z / gl_FragCoord.w;
				float fogFactor = exp(-u_fogDensity * fogCoord); //exp1
				//float fogFactor = exp(-pow(u_fogDensity * fogCoord, 2.0)); //exp2

				vec4 color = (Es.x * diffuseMap + Es.y * specular) * u_LightColor * cubeFilter * attenMap;
				gl_FragColor = mix(u_LightColor, color, fogFactor) * attenMap;  // u_LightColor == fogColor
				return;
	}

	if(u_fog == 0)
		gl_FragColor = (Es.x * diffuseMap + Es.y * specular) * u_LightColor * cubeFilter * attenMap;

	} 

}
