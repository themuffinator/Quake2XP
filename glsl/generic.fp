varying vec2		v_texCoord;
varying vec2		v_texCoord1;
varying vec4		v_colorArray;

uniform sampler2D	u_map;
uniform sampler2D	u_map1;
uniform vec3		u_color;
uniform float		u_colorScale;
 
void main(void) 
{
vec4 diffuse = texture2D(u_map, v_texCoord.xy);
vec4 diffuse1 = texture2D(u_map1, v_texCoord1.xy);

#ifdef CONSOLE
	gl_FragColor = diffuse * diffuse1 * v_colorArray * u_colorScale;
	return;
#endif

#ifdef ATTRIB_COLORS
	gl_FragColor =  diffuse * v_colorArray * u_colorScale;
#else
	gl_FragColor =  vec4(u_color, 1.0);
#endif
}
