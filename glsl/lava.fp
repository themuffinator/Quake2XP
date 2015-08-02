varying vec2			v_diffuseTexCoord;
varying vec3			v_viewVec;

uniform sampler2D		u_colorMap;
uniform float			u_ambient;
uniform int				u_parallax;

#include parallax.inc

void main()
{
vec3 V = normalize(v_viewVec);
vec2 texCoord = v_diffuseTexCoord;
vec2 P = CalcParallaxOffset(u_colorMap, v_diffuseTexCoord.xy, V);
float scale = clamp(u_ambient, 0.33, 1.0);

//load diffuse map
vec4 diffuse  = texture2D (u_colorMap, P);
 
gl_FragColor = vec4(diffuse.rgb * scale, 1.0);
}
