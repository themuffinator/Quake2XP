layout (binding = 0) uniform sampler2DRect u_ScreenTex;

void main(void) 
{
fragData = texture2DRect(u_ScreenTex, gl_FragCoord.xy);
fragData.a = 1.0;
}
