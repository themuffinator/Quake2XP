
uniform sampler2D		u_Diffuse;
uniform sampler2D		u_LightMap;
uniform sampler2D		u_Add;
uniform sampler2D		u_envMap;

uniform float       	u_ColorModulate;
uniform float       	u_ambientScale;    
uniform int				u_envPass;
uniform float			u_envPassScale;

varying vec3			v_viewVecTS;

varying vec2			v_wTexCoord;
varying vec2			v_lTexCoord;
varying vec2			v_envCoord;

 
#include parallax.inc
#include lighting.inc
void main ()
{
vec3 V = normalize(v_viewVecTS);
vec4 lightMap = texture2D(u_LightMap, v_lTexCoord.xy);
vec4 envMap = texture2D(u_envMap, v_envCoord.xy);
vec4 diffuseMap;
vec4 glowMap;
float envMask;

#ifdef PARALLAX
vec2 P = CalcParallaxOffset(u_Diffuse, v_wTexCoord.xy, V);
diffuseMap = texture2D(u_Diffuse, P);
glowMap = texture2D(u_Add, P);
envMask =  texture2D(u_envMap, P.xy).a;

#else

diffuseMap = texture2D(u_Diffuse,  v_wTexCoord.xy);
glowMap = texture2D(u_Add,  v_wTexCoord.xy);
envMask =  texture2D(u_envMap, v_wTexCoord.xy).a;

#endif 

//vec4 tmp = causticsMap * diffuseMap;

#ifdef LIGHTMAP
lightMap *= u_ambientScale;
diffuseMap += glowMap;
diffuseMap *= lightMap;
#else
diffuseMap *= u_ambientScale;
#endif

vec4 finalColor = diffuseMap;

if(u_envPass == 1)
{
envMap *= envMask;
envMap *= u_envPassScale;
finalColor +=envMap;
}

//if (u_isCaustics == 1){
//tmp *= u_CausticsModulate;
//finalColor = tmp + finalColor;
//}

gl_FragColor = vec4(finalColor.rgb, 1.0) * u_ColorModulate;

}
