in vec2		v_texCoord;
in vec2		v_texCoord1;
in vec4		v_colorArray;

layout (binding = 0) uniform sampler2D	u_map;
layout (binding = 1) uniform sampler2D	u_map1;

uniform vec4		u_color;
uniform float		u_colorScale;

uniform int			u_ATTRIB_COLORS, u_ATTRIB_CONSOLE, u_isSky, u_3d;
 
void main(void) 
{
vec4 diffuse = texture(u_map, v_texCoord.xy);
vec4 diffuse1 = texture(u_map1, v_texCoord1.xy);

if(u_ATTRIB_CONSOLE == 1){
	fragData = diffuse * diffuse1 * v_colorArray * u_colorScale;
	return;
}

if(u_ATTRIB_COLORS == 1){
	fragData =  vec4(diffuse.rgb * v_colorArray.rgb * u_colorScale, diffuse.a);
	return;
}

if(u_isSky == 1){
	fragData =  vec4(diffuse.rgb * u_colorScale, 1.0);
	return;
}
if(u_ATTRIB_COLORS != 1 && u_ATTRIB_CONSOLE != 1){
	fragData =  u_color;
	}

if(u_3d == 1)
	fragData =  u_color;

}
