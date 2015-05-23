varying vec2			v_diffuseTexCoord;
varying vec3			v_viewVec;
varying vec3  			v_lightVec;

uniform sampler2D		u_colorMap;

#include parallax.inc

void main()
{
vec3 V = normalize(v_viewVec);
vec2 texCoord = v_diffuseTexCoord;
vec2 P = texCoord;

if(u_parallaxType >= 1)	
	P = CalcParallaxOffset(u_colorMap, v_diffuseTexCoord.xy, V);

//load diffuse map
vec4 diffuse  = texture2D (u_colorMap, texCoord.xy);
 
gl_FragColor = vec4(diffuse.rgb * 0.33, 1.0);
}
