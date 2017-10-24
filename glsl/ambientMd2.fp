in	vec2		v_texCoord;
in	vec4		v_color;
in	vec2		v_envCoord;
in	vec2		v_shellCoord;

layout (binding = 0) uniform sampler2D 		u_Diffuse;
layout (binding = 1) uniform sampler2D 		u_Add;
layout (binding = 2) uniform sampler2D		u_env;
layout (binding = 3) uniform sampler2D		u_NormalMap;
layout (binding = 4) uniform sampler2DRect	u_ssaoMap;

uniform float		u_envScale;
uniform int			u_isEnvMap;

uniform	int			u_isShell;
uniform int			u_ssao;
uniform float		u_ColorModulate;
uniform float		u_AddShift; 

void main ()
{
	if(u_isShell >= 1){
		vec4 r0 = texture(u_Diffuse,  v_shellCoord);
		fragData = r0;
		return;
	}

	vec4 diffuse = texture(u_Diffuse, v_texCoord) * v_color;
	vec4 glow = texture(u_Add, v_texCoord);
	vec3 normalMap = normalize(texture(u_NormalMap, v_texCoord).xyz * 2.0 - 1.0);
	
  // fake AO/cavity
	fragData.xyz = diffuse.xyz * (normalMap.z * 0.5 + 0.5);


	if (u_ssao == 1)
		fragData.xyz *= texture2DRect(u_ssaoMap, gl_FragCoord.xy * 0.5).x;

	fragData.xyz += glow.rgb * u_AddShift;

	if (u_isEnvMap == 1)
		fragData.xyz += texture(u_env, v_envCoord).xyz * glow.a * u_envScale;

	fragData.xyz *= u_ColorModulate;
	fragData.w = diffuse.w;
}
