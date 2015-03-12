varying vec2		v_texCoord;
varying vec2		v_texCoord1;
varying vec4		v_colorArray;

uniform sampler2D	u_map;
uniform sampler2D	u_map1;
uniform vec4		u_color;
uniform float		u_colorScale;

uniform int			u_ATTRIB_COLORS, u_ATTRIB_CONSOLE, u_isSky;
 
void main(void) 
{
vec4 diffuse = texture2D(u_map, v_texCoord.xy);
vec4 diffuse1 = texture2D(u_map1, v_texCoord1.xy);

if(u_ATTRIB_CONSOLE == 1){
	gl_FragColor = diffuse * diffuse1 * v_colorArray * u_colorScale;
	return;
}

if(u_ATTRIB_COLORS == 1){
	gl_FragColor =  vec4(diffuse.rgb * v_colorArray.rgb * u_colorScale, diffuse.a);
	return;
}

if(u_isSky == 1){
	gl_FragColor =  vec4(diffuse.rgb * u_colorScale, 1);
	//gl_FragDepth = 1.0;
	return;
}
if(u_ATTRIB_COLORS != 1 && u_ATTRIB_CONSOLE != 1)
	gl_FragColor =  u_color;
}
