varying vec2 v_texCoord;
uniform sampler2D u_map;
 
void main(void) 
{
  gl_FragColor = texture2D(u_map, v_texCoord.xy);
}
