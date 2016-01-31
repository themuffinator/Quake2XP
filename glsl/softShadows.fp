uniform sampler2DRect	u_ScreenTex;

void main (void) {

		fragData =  texture2DRect(u_ScreenTex, gl_FragCoord.xy);
}
