uniform sampler2DRect u_fboTex;

void main(void) 
{
 vec4 color = texture2DRect(u_fboTex, gl_FragCoord.xy);
 gl_FragColor = color;
		
}
