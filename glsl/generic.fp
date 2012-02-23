varying vec2 v_texCoord;
uniform sampler2D u_map;
uniform float u_colorScale;
 
void main(void) 
{
vec4 diffuse = texture2D(u_map, v_texCoord.xy);
gl_FragColor =  diffuse * gl_Color * u_colorScale;
}
