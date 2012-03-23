varying vec2 v_texCoord;
uniform sampler2D u_map;
uniform float u_colorScale;
const vec4 LUMINANCE_MONO = vec4(0.27, 0.67, 0.06, 1.0);


/*
** Desaturation photoshop math
*/

vec4 Desaturate(vec3 color, float Desaturation)
{
	vec3 grayXfer = vec3(0.3, 0.59, 0.11);
	vec3 gray = vec3(dot(grayXfer, color));
	return vec4(mix(color, gray, Desaturation), 1.0);
}

void main(void) 
{
vec4 color =  texture2D(u_map, v_texCoord.xy);
float lum = dot(LUMINANCE_MONO, color);
//vec4 mono = vec4(lum, lum, lum, 1.0);
vec4 mono = Desaturate(color.rgb, 1); 
vec4 deltas = color - mono;
gl_FragColor = mono + u_colorScale * deltas;
}
