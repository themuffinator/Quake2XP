//!#include "include/global.inc"

layout (bindless_sampler, location  = U_TMU0)  uniform sampler2D	u_map;
layout (bindless_sampler, location  = U_TMU1)  uniform sampler2D	u_normalMap;

layout(location = U_PARAM_VEC4_0)	uniform vec4	u_lightShift;

in vec2			texCoord;

//!#include "include/lighting.inc"
#include lighting.inc 

void main()
{
    vec3 diffuse = texture(u_map, texCoord).rgb;
    vec3 normal = normalize(texture(u_normalMap, texCoord).rgb * 2.0 - 1.0);
    float specular = texture(u_normalMap, texCoord).a;	

    vec3 L = normalize(vec3(u_lightShift.x, u_lightShift.y, u_lightShift.z));
    vec3 V  = normalize(vec3(u_lightShift.x, u_lightShift.y, 1.0));
  
    vec2 Es = PhongLighting (normal.xyz, L, V, 16.0);
    vec4 lighting = vec4(diffuse.rgb * Es.x + specular * Es.y, 1.0);
    fragData = lighting * 1.25;
    fragData.a = 1.0;
}
