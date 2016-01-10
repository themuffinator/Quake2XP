uniform sampler2D	u_map;
uniform sampler2D	u_normalMap;
uniform vec2		u_params;

in vec2 texCoord;

#include lighting.inc

void main(void) 
{
vec3 light = normalize(vec3(11.5 + u_params.x, 45.0 - u_params.x, 10.0));
vec3 view  = normalize(vec3(77.5, 31.5, 66.6));

vec3 color = texture(u_map, texCoord).rgb;
vec4 bump = texture(u_normalMap, texCoord);
vec3 specular = vec3(bump.a);	
vec3 normal = normalize(bump.rgb -0.5);

vec2 Es = PhongLighting(normal, light, view, 16.0);

fragData.rgb = (Es.x * color + Es.y * specular) * u_params.y;
fragData.a = 1.0;
}
