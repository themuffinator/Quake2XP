in	vec2		v_texCoord;
in	vec4		v_color;
in	vec2		v_envCoord;
in	vec2		v_shellCoord;

layout (binding = 0) uniform sampler2D 		u_Diffuse;
layout (binding = 1) uniform sampler2D 		u_Add;
layout (binding = 2) uniform sampler2D		u_env;
layout (binding = 3) uniform sampler2D		u_NormalMap;
layout (binding = 4) uniform sampler2D		u_ssaoMap;
layout (binding = 5) uniform sampler2D		u_rgh;

layout(location = U_ENV_SCALE)		uniform float	u_envScale;
layout(location = U_ENV_PASS)		uniform int		u_isEnvMap;
layout(location = U_TRANS_PASS)		uniform int		u_isTransluscent;
layout(location = U_SHELL_PASS)		uniform	int		u_isShell;
layout(location = U_COLOR_MUL)		uniform float	u_ColorModulate;
layout(location = U_COLOR_OFFSET)	uniform float	u_AddShift; 
layout(location = U_PARAM_INT_0)	uniform int		u_alphaMask; 

void main ()
{
	if(u_isShell == 1){
		vec4 r0 = texture(u_Diffuse,  v_shellCoord);
		fragData = r0;
		return;
	}

	if(u_alphaMask == 1){
		float mask = texture(u_rgh, v_texCoord).g;
		if (mask <= 0.01) {
			discard;
			return;
			}
	}
	
	if(u_isTransluscent == 1){
		vec4 diffuse = texture(u_Diffuse, v_texCoord) * v_color;
		vec4 env = texture(u_env,  v_shellCoord);
		vec3 normalMap = normalize(texture(u_NormalMap, v_texCoord).xyz * 2.0 - 1.0);
		env *= u_envScale;
		diffuse *= (normalMap.z * 0.5 + 0.5);
		diffuse += env;
		fragData = diffuse;
		return;
	}

	vec4 diffuse = texture(u_Diffuse, v_texCoord) * v_color;
	vec3 glow = texture(u_Add, v_texCoord).rgb;
	vec3 normalMap = normalize(texture(u_NormalMap, v_texCoord).xyz * 2.0 - 1.0);
	
  // fake AO/cavity
	fragData.xyz = diffuse.xyz * (normalMap.z * 0.5 + 0.5);
	fragData.xyz *= texture(u_ssaoMap, v_texCoord).x;

	fragData.xyz += glow * u_AddShift;

	if (u_isEnvMap == 1)
		fragData.xyz += texture(u_env, v_envCoord).xyz * u_envScale;

	fragData.w = 1.0;
}
