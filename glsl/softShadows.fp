uniform sampler2DRect	u_ScreenTex;

void main (void) {

		fragData.rg =  texture2DRect(u_ScreenTex, gl_FragCoord.xy).rg;
		fragData.ba = vec2(1.0);
}
