
uniform sampler2D		u_Diffuse;
uniform sampler2D		u_NormalMap;
uniform samplerCube 	u_CubeFilterMap;
uniform sampler3D	 	u_attenMap;

uniform float			u_ColorModulate;
uniform float			u_specularScale;
uniform float			u_specularExp;
uniform vec4 			u_LightColor;
uniform float 			u_LightRadius;

varying vec3			v_viewVecTS;
varying vec3			v_lightVec;
varying vec2			v_colorCoord;
varying vec4			v_CubeCoord;
varying vec4			v_lightCoord;
varying vec3			v_AttenCoord;

#include lighting.inc
#include parallax.inc

void main ()
{
 
vec3 V = normalize(v_viewVecTS);
vec3 L = normalize(v_lightVec);

#ifdef PARALLAX
vec2 P = CalcParallaxOffset(u_Diffuse, v_colorCoord.xy, V);
vec4 diffuseMap = texture2D(u_Diffuse, P);
vec3 normalMap =  normalize(texture2D(u_NormalMap, P).rgb * 2.0 - 1.0);
float specTmp = texture2D(u_NormalMap, P).a;
#else
vec4 diffuseMap = texture2D(u_Diffuse,  v_colorCoord.xy);
vec3 normalMap =  normalize(texture2D(u_NormalMap, v_colorCoord.xy).rgb * 2.0 - 1.0);
float specTmp = texture2D(u_NormalMap, v_colorCoord).a;
#endif

vec4 specular = vec4(specTmp) * u_specularScale;

vec4 u_attenMap = texture3D(u_attenMap ,v_AttenCoord);

// light filter
vec4 cubeFilter = textureCube(u_CubeFilterMap, v_CubeCoord.xyz);
cubeFilter *= 2;

#ifdef AMBIENT   
vec4 ambient = u_attenMap * (normalMap.z * normalMap.z);
gl_FragColor = diffuseMap * ambient * u_LightColor;

#else

vec2 Es = PhongLighting(normalMap, L, V, u_specularExp);
gl_FragColor = (Es.x * diffuseMap + Es.y * specular) * cubeFilter * u_attenMap * u_LightColor;

#endif 
}
