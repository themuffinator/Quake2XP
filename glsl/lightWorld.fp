
uniform sampler2D		u_Diffuse;
uniform sampler2D		u_NormalMap;
uniform samplerCube 		u_CubeFilterMap;

uniform float      		u_ColorModulate;
uniform float			u_specularScale;
uniform float			u_specularExp;
uniform vec4 			u_LightColor;
uniform float 			u_LightRadius;

varying vec3			v_viewVecTS;
varying vec3			v_lightVec;
varying vec2			v_colorCoord;
varying vec4			v_CubeCoord;

#include lighting.inc
#include parallax.inc

void main ()
{

vec3 V = normalize(v_viewVecTS);
vec3 L = normalize(v_lightVec);

#ifdef PARALLAX
vec2 P = CalcParallaxOffset(u_Diffuse, v_colorCoord.xy, V);
vec4 diffuseMap = texture2D(u_Diffuse, P);
vec3 normalMap =  normalize(texture2D(u_NormalMap, P).rgb - 0.5);
float specTmp = texture2D(u_NormalMap, P).a;
#else
vec4 diffuseMap = texture2D(u_Diffuse,  v_colorCoord.xy);
vec3 normalMap =  normalize(texture2D(u_NormalMap, v_colorCoord.xy).rgb - 0.5);
float specTmp = texture2D(u_NormalMap, v_colorCoord).a;
#endif

vec4 specular = vec4(specTmp) * u_specularScale;

// compute the atten
vec3 tmp1 = v_lightVec;
tmp1 /= u_LightRadius;
float att = max(1.0 - dot(tmp1, tmp1), 0.0);

vec2 Es = PhongLighting(normalMap, L, V, u_specularExp);
vec4 cubeFilter = textureCube(u_CubeFilterMap, v_CubeCoord.xyz);

gl_FragColor = cubeFilter * u_LightColor * att * (Es.x * diffuseMap + Es.y * specular);

}
