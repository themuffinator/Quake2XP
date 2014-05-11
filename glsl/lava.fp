varying vec2			v_diffuseTexCoord;
varying vec3			v_viewVec;
varying vec3  			v_lightVec;

uniform sampler2D		u_colorMap;
uniform sampler2D		u_NormalMap;

#include lighting.inc
#include parallax.inc

void main()
{

vec3 V = normalize(v_viewVec);
vec3 L = normalize(v_lightVec);
vec2 texCoord;

// calc parallax offet
vec2 P = CalcParallaxOffset(u_colorMap, v_diffuseTexCoord.xy, V);
if (u_parallaxType >=1)
texCoord = P;
else
texCoord = v_diffuseTexCoord;

//load diffuse map
vec4 diffuse  = texture2D (u_colorMap, texCoord.xy);
// load normal map
vec3 normalMap =  normalize(texture2D(u_NormalMap, texCoord.xy).rgb * 2.0 - 1.0);
//load specular
vec4 specTmp = texture2D(u_NormalMap, texCoord.xy).a;
// move from alpha to rgb
vec4 specular = vec4(specTmp) * 0.4;

vec2 Es = PhongLighting(normalMap, L, V, 8.0);
vec4 bumpLight = (Es.x * diffuse) + (Es.y * specular);
diffuse *= 0.3;
diffuse += bumpLight;
 
gl_FragColor = vec4(diffuse.rgb, 1.0);
}
