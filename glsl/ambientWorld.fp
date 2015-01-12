
uniform sampler2D		u_Diffuse;
uniform sampler2D		u_LightMap;
uniform sampler2D		u_Add;
uniform sampler2D		u_envMap;
uniform sampler2D		u_csmMap;

uniform float       	u_ColorModulate;
uniform float       	u_ambientScale;    
uniform int				u_envPass;
uniform float			u_envPassScale;

varying vec3			v_positionVS;
varying vec3			v_viewVecTS;
varying vec2			v_wTexCoord;
varying vec2			v_lTexCoord;
varying vec2			v_envCoord;
varying vec3			v_tbn[3];
 
#include parallax.inc

void main ()
{
vec3	V = normalize(v_viewVecTS);
vec3	Vp = normalize(v_positionVS);
vec3	T = normalize(v_tbn[0]);
vec3	B = normalize(v_tbn[1]);
vec3	N = normalize(v_tbn[2]);

vec4	lightMap = texture2D(u_LightMap, v_lTexCoord.xy);
vec4	envMap = texture2D(u_envMap, v_envCoord.xy);
vec4	diffuseMap;
vec4	glowMap;
float	envMask;

#ifdef PARALLAX
vec2 P;
if(u_parallaxType == 1){
// ray intersection in view direction
float a = abs(dot(N, Vp));
a = sin(clamp(a, 0.0, 1.0) * HALF_PI) / a;	// thx Berserker for corner artifact correction

vec3 dp = vec3(v_wTexCoord, 0.0);
vec3 ds = vec3(a * u_parallaxParams.x * dot(T, Vp), a * u_parallaxParams.y * dot(B, Vp), 1.0);
float distFactor = 0.05 * sqrt(length(fwidth(v_wTexCoord)));
IntersectConeExp(u_csmMap, dp, ds, distFactor);
P = dp.xy;
}
else
P = CalcParallaxOffset(u_Diffuse, v_wTexCoord.xy, V);

diffuseMap = texture2D(u_Diffuse, P);
glowMap = texture2D(u_Add, P);
envMask =  texture2D(u_envMap, P.xy).a;

#else

diffuseMap = texture2D(u_Diffuse,  v_wTexCoord.xy);
glowMap = texture2D(u_Add,  v_wTexCoord.xy);
envMask =  texture2D(u_envMap, v_wTexCoord.xy).a;

#endif 

lightMap *= u_ambientScale;
diffuseMap += glowMap;
diffuseMap *= lightMap;

vec4 finalColor = diffuseMap;

if(u_envPass == 1)
{
envMap *= envMask;
envMap *= u_envPassScale;
finalColor += envMap;
}

gl_FragColor = vec4(finalColor.rgb, 1.0) * u_ColorModulate;
}
