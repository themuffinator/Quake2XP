varying	vec2		v_texCoord;
varying	vec4		v_color;

uniform sampler2D 	u_Diffuse;
uniform sampler2D	u_NormalMap;
uniform sampler2D 	u_Add;

uniform sampler2D	u_env;
uniform float		u_envScale;
uniform int			u_isEnvMap;

uniform	int			u_isShell;

varying		vec2	v_envCoord;
varying		vec2	v_shellCoord;

uniform float       u_ColorModulate;
//uniform float       u_ambientScale;    
uniform float       u_AddShift; 

void main ()
{
	if(u_isShell == 1){
		vec4 r0 = texture2D(u_Diffuse,  v_shellCoord);
		gl_FragColor = r0;
		return;
	}

	vec4 diffuse = texture2D(u_Diffuse, v_texCoord) * v_color;// * u_ambientScale;
	vec4 glow = texture2D(u_Add, v_texCoord);
	vec3 normalMap = normalize(texture2D(u_NormalMap, v_texCoord).xyz * 2.0 - 1.0);

	// fake AO/cavity
//	gl_FragColor.xyz = diffuse.xyz;
	gl_FragColor.xyz = diffuse.xyz * (normalMap.z * 0.5 + 0.5);
//	gl_FragColor.xyz = diffuse.xyz * normalMap.z;

	gl_FragColor.xyz += glow * u_AddShift;

	if (u_isEnvMap == 1)
		gl_FragColor.xyz += texture2D(u_env, v_envCoord).xyz * glow.a * u_envScale;

	gl_FragColor.xyz *= u_ColorModulate;
	gl_FragColor.w = diffuse.w;
}
