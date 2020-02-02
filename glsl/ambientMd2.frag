in	vec2		v_texCoord;
in	vec4		v_color;
in	vec2		v_envCoord;
in	vec2		v_shellCoord;

layout (bindless_sampler, location  = U_TMU0) uniform sampler2D 	u_Diffuse;
layout (bindless_sampler, location  = U_TMU1) uniform sampler2D 	u_Add;
layout (bindless_sampler, location  = U_TMU2) uniform sampler2D		u_env;
layout (bindless_sampler, location  = U_TMU3) uniform sampler2D		u_NormalMap;
layout (bindless_sampler, location  = U_TMU4) uniform sampler2DRect	u_ssaoMap;

layout(location = U_ENV_SCALE)		uniform float	u_envScale;
layout(location = U_ENV_PASS)		uniform int		u_isEnvMap;
layout(location = U_SHELL_PASS)		uniform	int		u_isShell;
layout(location = U_USE_SSAO)		uniform int		u_ssao;
layout(location = U_COLOR_MUL)		uniform float	u_ColorModulate;
layout(location = U_COLOR_OFFSET)	uniform float	u_AddShift; 

void main ()
{
	if(u_isShell >= 1){
		vec4 r0 = texture(u_Diffuse,  v_shellCoord);
		fragData = r0;
		return;
	}

	vec4 glow = texture(u_Add, v_texCoord);
	vec3 normalMap = normalize(texture(u_NormalMap, v_texCoord).rgb * 2.0 - 1.0);
	vec4 diffuse = texture(u_Diffuse, v_texCoord) * v_color;
	
  // fake AO/cavity
	fragData.rgb = diffuse.rgb * (normalMap.z * 0.5 + 0.5);


	if (u_ssao == 1)
		fragData.rgb *= texture2DRect(u_ssaoMap, gl_FragCoord.xy * 0.5).rgb;

	fragData.rgb += glow.rgb * u_AddShift;

	if (u_isEnvMap == 1)
		fragData.rgb += texture(u_env, v_envCoord).rgb * glow.a * u_envScale;

	fragData.rgb *= u_ColorModulate;

	fragData.a = 1.0;
}
