uniform sampler2D u_map;
uniform sampler2D u_normalMap;

in vec2 texCoord;
in vec3 lightVec;

#include lighting.inc

vec4 Desaturate(vec3 color, float Desaturation)
{
	vec3 grayXfer = vec3(0.3, 0.59, 0.11);
	vec3 gray = vec3(dot(grayXfer, color));
	return vec4(mix(color, gray, Desaturation), 1.0);
}

void main(void) 
{
vec3 light = normalize(lightVec);

vec4 color = texture(u_map, texCoord);
vec4 specular = Desaturate(color.xyz, 1.0);	
vec3 normal = normalize(texture(u_normalMap, texCoord).rgb * 2.0 - 1.0);
normal.z = 1.0;
vec2 Es = PhongLighting(normal, light, light, 16.0);

fragData.xyz = (Es.x * color.xyz + Es.y * specular.xyz * 0.5) * 16.0;
fragData.w = 1.0;
}
