layout (binding = 0) uniform sampler2D	u_map;
layout (binding = 1) uniform sampler2D	u_normalMap;
uniform vec2		u_params;

in vec2 texCoord;

#include lighting.inc

void main(void) 
{
vec3 light = normalize(vec3(11.5 + u_params.x, 45.0 - u_params.x, 10.0));
vec3 view  = normalize(vec3(77.5, 31.5, 66.6));

vec3 color = texture(u_map, texCoord).rgb;
vec3 normal = normalize(texture(u_normalMap, texCoord).rgb * 2.0 - 1.0);
float specular = texture(u_normalMap, texCoord).a;	


vec2 Es = PhongLighting(normal, light, view, 16.0);

fragData.rgb = (Es.x * color + Es.y * specular) * u_params.y;
fragData.a = 1.0;
}
