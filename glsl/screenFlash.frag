layout (bindless_sampler, location  = U_TMU0) uniform sampler2DRect	u_ScreenTex;

layout(location = U_SCREEN_SIZE) uniform vec2	u_screenSize; 
layout(location = U_PARAM_VEC3_0) uniform vec3	u_flashColor; 

void main(void) 
{
	vec2 uv = gl_FragCoord.xy / u_screenSize;
	vec3 color = texture2DRect(u_ScreenTex, gl_FragCoord.xy).rgb;

	float dist = length( uv - vec2(0.5,0.5) ); //find center
    float vignette = 1.4 - dist;
    vignette = smoothstep(0.1, 1.0, vignette);

	fragData.rgb = mix(u_flashColor, color, vignette);
	fragData.a = 1.0;
}