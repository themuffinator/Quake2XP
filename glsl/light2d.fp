uniform sampler2D	u_map;
uniform sampler2D	u_normalMap;
uniform float		u_shift, u_intens;

in vec2 texCoord;

#include lighting.inc

void main(void) 
{
vec3 light = normalize(vec3(11.5 + u_shift, 45.0 - u_shift, 10.0));
vec3 view  = normalize(vec3(77.5, 31.5, 66.6));

vec3 color = texture(u_map, texCoord).rgb;
vec4 bump = texture(u_normalMap, texCoord);
vec3 specular = vec3(bump.a);	
vec3 normal = normalize(bump.rgb * 2.0 - 1.0);

vec2 Es = PhongLighting(normal, light, view, 16.0);

fragData.rgb = (Es.x * color + Es.y * specular) * u_intens;
fragData.a = 1.0;
}
