layout (binding = 0) uniform sampler2DRect	u_ScreenTex;

layout(location = U_SCREEN_SIZE) uniform vec2	u_screenSize; 

vec4 SepiaColor (vec4 color)
{	
	float lum = dot(color.rgb, vec3(0.2125, 0.7154, 0.0721));
	vec3 sepia = vec3(1.2, 1.0, 0.8); 
	sepia *= lum;
	vec3 tmp = mix(color.rgb, sepia, 0.88);

	return vec4(tmp, 1.0);
}

void main(void) 
{
	vec2 uv = gl_FragCoord.xy / u_screenSize;
	vec4 color = texture2DRect(u_ScreenTex, gl_FragCoord.xy);
	color *= 0.45;
	vec4 sepia = SepiaColor(color);

	float OuterVignetting	= 1.4 - 0.75;
	float InnerVignetting	= 1.0 - 0.75;

	float d = distance(vec2(0.5, 0.5), uv) * 1.414213;
	float vignetting = clamp((OuterVignetting - d) / (OuterVignetting - InnerVignetting), 0.0, 1.0);

	fragData = mix(color, sepia, vignetting);
	fragData.a = 1.0;
}