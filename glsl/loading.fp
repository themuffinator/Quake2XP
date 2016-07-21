layout (binding = 0)  uniform sampler2D u_map;
uniform float u_colorScale;

in vec2 v_texCoord;

vec4 Desaturate(vec3 color, float Desaturation)
{
	vec3 grayXfer = vec3(0.3, 0.59, 0.11);
	vec3 gray = vec3(dot(grayXfer, color));
	return vec4(mix(color, gray, Desaturation), 1.0);
}

void main(void) 
{
vec4 color =  texture(u_map, v_texCoord.xy);
vec4 mono = Desaturate(color.rgb, 1.0); 
vec4 deltas = color - mono;
fragData = mono + u_colorScale * deltas;
}
