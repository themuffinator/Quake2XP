
uniform sampler2D		u_Diffuse;
uniform sampler2D		u_LightMap;
uniform sampler2D		u_Add;
uniform sampler2D		u_NormalMap;
uniform sampler2D		u_deluxMap;
uniform sampler2D		u_Caustics;
uniform float       	u_ColorModulate;
uniform float       	u_ambientScale;    
uniform float       	u_CausticsModulate; 
uniform vec2			u_texSize;
uniform vec2			u_parallaxScale;
uniform int				u_parallaxType;
uniform int				u_numLights;
uniform int				u_activeLights;
uniform int				u_isCaustics;
uniform vec3			u_LightColor[13];
uniform float			u_LightRadius[13];
uniform float			u_specularScale;
uniform float			u_specularExp;

varying vec3			v_viewVecTS;
varying vec3			t, b, n;
varying vec2			v_wTexCoord;
varying vec2			v_lTexCoord;
varying vec4			v_color;
varying vec3			v_lightVec[13];
 
#include parallax.inc
#include lighting.inc

void main ()
{
vec3 V = normalize(v_viewVecTS);
vec4 lightMap = texture2D(u_LightMap, v_lTexCoord.xy); 
vec3 wDelux = normalize(texture2D(u_deluxMap, v_lTexCoord).rgb - 0.5);
vec4 diffuseMap;
vec4 glowMap;
vec4 causticsMap;
vec3 normalMap;
float specTmp;
vec3 tbnDelux;
vec4 bumpLight;

#ifdef PARALLAX
vec2 P = CalcParallaxOffset(u_Diffuse, v_wTexCoord.xy, V);
diffuseMap = texture2D(u_Diffuse, P);
glowMap = texture2D(u_Add, P);
causticsMap = texture2D(u_Caustics, P);
normalMap =  normalize(texture2D(u_NormalMap, P.xy).rgb - 0.5);
specTmp = texture2D(u_NormalMap,   P.xy).a;

#else

diffuseMap = texture2D(u_Diffuse,  v_wTexCoord.xy);
glowMap = texture2D(u_Add,  v_wTexCoord.xy);
causticsMap = texture2D(u_Caustics, v_wTexCoord.xy);
normalMap =  normalize(texture2D(u_NormalMap, v_wTexCoord.xy).rgb - 0.5);
specTmp = texture2D(u_NormalMap, v_wTexCoord).a;

#endif 

vec4 specular = vec4(specTmp) * u_specularScale;


#ifdef VERTEXLIGHT
diffuseMap *= clamp(v_color, 0.0, 0.666);
#endif

// Bump World 
#ifdef BUMP

#ifdef VERTEXLIGHT
tbnDelux.x = abs(dot(n, t));
tbnDelux.y = abs(dot(n, b));
tbnDelux.z = 1.0;
vec2 Es = PhongLighting(normalMap, tbnDelux, V, u_specularExp);

#else

//Put delux into tangent space
tbnDelux.x = dot(wDelux, t);
tbnDelux.y = dot(wDelux, b);
tbnDelux.z = abs(dot(wDelux, n));

tbnDelux = clamp(tbnDelux, 0.333, 1.0);
vec2 Es = PhongLighting(normalMap, tbnDelux, V, u_specularExp);

#endif

#ifdef LIGHTMAP
bumpLight = (Es.x * diffuseMap) + (Es.y * specular * lightMap);
#endif

#ifdef VERTEXLIGHT
bumpLight = (Es.x * diffuseMap * v_color) + (Es.y * specular * v_color); //via lava surfaces 
#endif

diffuseMap *= u_ambientScale;
diffuseMap += bumpLight;

#ifdef LIGHTMAP
diffuseMap *= lightMap;
#endif

vec4 finalColor = diffuseMap + glowMap;

if (u_isCaustics == 1){
vec4 tmp;
tmp = causticsMap * finalColor;
tmp *= u_CausticsModulate;
finalColor = tmp + finalColor;
}

// Add dinamyc lights
if(u_numLights <= 13 && u_numLights > 0  && u_activeLights == 1){

vec3 tmp1;
float att;
vec3 L;
vec2 E;
vec3 Dlighting;

if(u_numLights >= 1 && u_LightRadius[0] >=1){
tmp1 = v_lightVec[0];
tmp1 /= u_LightRadius[0];
att = max(1.0 - dot(tmp1, tmp1), 0.0);
L = normalize(v_lightVec[0]);
E = PhongLighting(normalMap, L, V, u_specularExp);
E *= att;
Dlighting = (E.x * diffuseMap.rgb + E.y * specular.rgb) * u_LightColor[0];
finalColor.rgb += Dlighting.rgb;
}
if(u_numLights >= 2 && u_LightRadius[1] >=1){
tmp1 = v_lightVec[1];
tmp1 /= u_LightRadius[1];
att = max(1.0 - dot(tmp1, tmp1), 0.0);
L = normalize(v_lightVec[1]);
E = PhongLighting(normalMap, L, V, u_specularExp);
E *= att;
Dlighting = (E.x * diffuseMap.rgb + E.y * specular.rgb) * u_LightColor[1];
finalColor.rgb += Dlighting.rgb;
}
if(u_numLights >= 3 && u_LightRadius[2] >=1){
tmp1 = v_lightVec[2];
tmp1 /= u_LightRadius[2];
att = max(1.0 - dot(tmp1, tmp1), 0.0);
L = normalize(v_lightVec[2]);
E = PhongLighting(normalMap, L, V, u_specularExp);
E *= att;
Dlighting = (E.x * diffuseMap.rgb + E.y * specular.rgb) * u_LightColor[2];
finalColor.rgb += Dlighting.rgb;
}
if(u_numLights >= 4 && u_LightRadius[3] >=1){
tmp1 = v_lightVec[3];
tmp1 /= u_LightRadius[3];
att = max(1.0 - dot(tmp1, tmp1), 0.0);
L = normalize(v_lightVec[3]);
E = PhongLighting(normalMap, L, V, u_specularExp);
E *= att;
Dlighting = (E.x * diffuseMap.rgb + E.y * specular.rgb) * u_LightColor[3];
finalColor.rgb += Dlighting.rgb;
}
if(u_numLights >= 5 && u_LightRadius[4] >=1){
tmp1 = v_lightVec[4];
tmp1 /= u_LightRadius[4];
att = max(1.0 - dot(tmp1, tmp1), 0.0);
L = normalize(v_lightVec[4]);
E = PhongLighting(normalMap, L, V, u_specularExp);
E *= att;
Dlighting = (E.x * diffuseMap.rgb + E.y * specular.rgb) * u_LightColor[4];
finalColor.rgb += Dlighting.rgb;
}
if(u_numLights >= 6 && u_LightRadius[5] >=1){
tmp1 = v_lightVec[5];
tmp1 /= u_LightRadius[5];
att = max(1.0 - dot(tmp1, tmp1), 0.0);
L = normalize(v_lightVec[5]);
E = PhongLighting(normalMap, L, V, u_specularExp);
E *= att;
Dlighting = (E.x * diffuseMap.rgb + E.y * specular.rgb) * u_LightColor[5];
finalColor.rgb += Dlighting.rgb;
}
if(u_numLights >= 7 && u_LightRadius[6] >=1){
tmp1 = v_lightVec[6];
tmp1 /= u_LightRadius[6];
att = max(1.0 - dot(tmp1, tmp1), 0.0);
L = normalize(v_lightVec[6]);
E = PhongLighting(normalMap, L, V, u_specularExp);
E *= att;
Dlighting = (E.x * diffuseMap.rgb + E.y * specular.rgb) * u_LightColor[6];
finalColor.rgb += Dlighting.rgb;
}
if(u_numLights >= 8 && u_LightRadius[7] >=1){
tmp1 = v_lightVec[7];
tmp1 /= u_LightRadius[7];
att = max(1.0 - dot(tmp1, tmp1), 0.0);
L = normalize(v_lightVec[7]);
E = PhongLighting(normalMap, L, V, u_specularExp);
E *= att;
Dlighting = (E.x * diffuseMap.rgb + E.y * specular.rgb) * u_LightColor[7];
finalColor.rgb += Dlighting.rgb;
}
if(u_numLights >= 9 && u_LightRadius[8] >=1){
tmp1 = v_lightVec[8];
tmp1 /= u_LightRadius[8];
att = max(1.0 - dot(tmp1, tmp1), 0.0);
L = normalize(v_lightVec[8]);
E = PhongLighting(normalMap, L, V, u_specularExp);
E *= att;
Dlighting = (E.x * diffuseMap.rgb + E.y * specular.rgb) * u_LightColor[8];
finalColor.rgb += Dlighting.rgb;
}
if(u_numLights >= 10 && u_LightRadius[9] >=1){
tmp1 = v_lightVec[9];
tmp1 /= u_LightRadius[9];
att = max(1.0 - dot(tmp1, tmp1), 0.0);
L = normalize(v_lightVec[9]);
E = PhongLighting(normalMap, L, V, u_specularExp);
E *= att;
Dlighting = (E.x * diffuseMap.rgb + E.y * specular.rgb) * u_LightColor[9];
finalColor.rgb += Dlighting.rgb;
}
if(u_numLights == 11 && u_LightRadius[10] >=1){
tmp1 = v_lightVec[10];
tmp1 /= u_LightRadius[10];
att = max(1.0 - dot(tmp1, tmp1), 0.0);
L = normalize(v_lightVec[10]);
E = PhongLighting(normalMap, L, V, u_specularExp);
E *= att;
Dlighting = (E.x * diffuseMap.rgb + E.y * specular.rgb) * u_LightColor[10];
finalColor.rgb += Dlighting.rgb;
}
if(u_numLights >= 12 && u_LightRadius[11] >=1){
tmp1 = v_lightVec[11];
tmp1 /= u_LightRadius[11];
att = max(1.0 - dot(tmp1, tmp1), 0.0);
L = normalize(v_lightVec[11]);
E = PhongLighting(normalMap, L, V, u_specularExp);
E *= att;
Dlighting = (E.x * diffuseMap.rgb + E.y * specular.rgb) * u_LightColor[11];
finalColor.rgb += Dlighting.rgb;
}
if(u_numLights == 13 && u_LightRadius[12] >=1){
tmp1 = v_lightVec[12];
tmp1 /= u_LightRadius[12];
att = max(1.0 - dot(tmp1, tmp1), 0.0);
L = normalize(v_lightVec[12]);
E = PhongLighting(normalMap, L, V, u_specularExp);
E *= att;
Dlighting = (E.x * diffuseMap.rgb + E.y * specular.rgb) * u_LightColor[12];
finalColor.rgb += Dlighting.rgb;
}

}

#else
// Non bump World
#ifdef LIGHTMAP
diffuseMap *= lightMap;
#endif

vec4 finalColor = diffuseMap + glowMap;

if (u_isCaustics == 1){
vec4 tmp;
tmp = causticsMap * finalColor;
tmp *= u_CausticsModulate;
finalColor = tmp + finalColor;
}

#endif

gl_FragColor = vec4(finalColor.rgb, 1.0) * u_ColorModulate;

}
