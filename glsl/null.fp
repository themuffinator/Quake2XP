varying vec3 color;

void main(void) 
{
gl_FragColor = vec4(color[0], color[1], color[2], 1.0);
//gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
}
