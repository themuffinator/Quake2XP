
uniform sampler2D		u_Diffuse;
uniform sampler2D		u_LightMap;
uniform sampler2D		u_Add;
uniform sampler2D		u_NormalMap; // lava
uniform sampler2D		u_Caustics;
uniform sampler2D		u_envMap;

uniform float       	u_ColorModulate;
uniform float       	u_ambientScale;    
uniform float       	u_CausticsModulate; 
uniform int				u_isCaustics;
uniform int				u_envPass;
uniform float			u_envPassScale;
uniform float			u_specularScale; // for lava
uniform float			u_specularExp;   // for lava

varying vec3			v_viewVecTS;
varying vec3			t, b, n;
varying vec2			v_wTexCoord;
varying vec2			v_lTexCoord;
varying vec2			v_envCoord;
varying vec4			v_color;

 
#include parallax.inc
#include lighting.inc
void main ()
{
vec3 V = normalize(v_viewVecTS);
vec4 lightMap = texture2D(u_LightMap, v_lTexCoord.xy);
vec4 envMap = texture2D(u_envMap, v_envCoord.xy);
vec4 diffuseMap;
vec4 glowMap;
vec4 causticsMap;
vec4 bumpLight;
vec3 normalMap;
float specTmp;
float envMask;

#ifdef PARALLAX
vec2 P = CalcParallaxOffset(u_Diffuse, v_wTexCoord.xy, V);
diffuseMap = texture2D(u_Diffuse, P);
glowMap = texture2D(u_Add, P);
causticsMap = texture2D(u_Caustics, P);
normalMap =  normalize(texture2D(u_NormalMap, P.xy).rgb - 0.5);
specTmp = texture2D(u_NormalMap,   P.xy).a;
envMask =  texture2D(u_envMap, P.xy).a;

#else

diffuseMap = texture2D(u_Diffuse,  v_wTexCoord.xy);
glowMap = texture2D(u_Add,  v_wTexCoord.xy);
causticsMap = texture2D(u_Caustics, v_wTexCoord.xy);
normalMap =  normalize(texture2D(u_NormalMap, v_wTexCoord.xy).rgb - 0.5);
specTmp = texture2D(u_NormalMap, v_wTexCoord).a;
envMask =  texture2D(u_envMap, v_wTexCoord.xy).a;

#endif 


#ifdef VERTEXLIGHT
vec4 specular = vec4(specTmp) * u_specularScale;
diffuseMap *= clamp(v_color, 0.0, 0.666);
vec3 tbnDelux;
tbnDelux.x = abs(dot(n, t));
tbnDelux.y = abs(dot(n, b));
tbnDelux.z = 1.0;
vec2 Es = PhongLighting(normalMap, tbnDelux, V, u_specularExp);
bumpLight = (Es.x * diffuseMap * v_color) + (Es.y * specular * v_color); //via lava surfaces 
diffuseMap += bumpLight;
#endif

#ifdef LIGHTMAP
diffuseMap *= lightMap * u_ambientScale;
#endif

vec4 finalColor = diffuseMap + glowMap;

if(u_envPass == 1)
{
envMap *= envMask;
envMap *= u_envPassScale;
finalColor +=envMap;
}

if (u_isCaustics == 1){
vec4 tmp;
tmp = causticsMap * finalColor;
tmp *= u_CausticsModulate;
finalColor = tmp + finalColor;
}

gl_FragColor = vec4(finalColor.rgb, 1.0) * u_ColorModulate;

}
