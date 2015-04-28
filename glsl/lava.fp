varying vec2			v_diffuseTexCoord;
varying vec3			v_viewVec;
varying vec3  			v_lightVec;
varying vec3			v_tbn[3];
varying vec3			v_positionVS;

uniform sampler2D		u_colorMap;
uniform sampler2D		u_csmMap;

#include parallax.inc

void main()
{
vec3 V = normalize(v_viewVec);
vec3 Vp = normalize(v_positionVS);
vec3 T = normalize(v_tbn[0]);
vec3 B = normalize(v_tbn[1]);
vec3 N = normalize(v_tbn[2]);
vec2 texCoord = v_diffuseTexCoord;
vec2 P = texCoord;

if(u_parallaxType >= 1)	
	P = CalcParallaxOffset(u_colorMap, v_diffuseTexCoord.xy, V);

//load diffuse map
vec4 diffuse  = texture2D (u_colorMap, texCoord.xy);
 
gl_FragColor = vec4(diffuse.rgb * 0.33, 1.0);
}
