layout(location = 0) in vec3 att_position;
layout(location = 1) in vec3 att_normal;
layout(location = 4) in vec4 att_color4f;
layout(location = 5) in vec2 att_texCoordDiffuse;

layout(location = U_SHELL_PARAMS)	uniform	vec2		u_shellParams;
layout(location = U_VIEW_POS)		uniform	vec3		u_viewOrg;
layout(location = U_ENV_PASS)		uniform	int			u_isEnvMap;
layout(location = U_SHELL_PASS)		uniform	int			u_isShell;
layout(location = U_MVP_MATRIX)		uniform mat4		u_modelViewProjectionMatrix;

out	vec2			v_texCoord;
out	vec2			v_envCoord;
out	vec2			v_shellCoord;
out	vec4			v_color;

void main ()
{
vec4 xyz = vec4(att_position, 1.0);

if(u_isShell == 1 || u_isEnvMap == 1){
	vec3 viewPos = normalize(u_viewOrg - att_position);
	float d = dot(att_normal, viewPos);
	vec3 reflected = att_normal * 2.0 * d - viewPos;
	v_shellCoord = v_envCoord = 0.5 + reflected.yz * vec2(0.5, -0.5);

	v_shellCoord += u_shellParams.x;
	
	if(u_isShell == 1)
		xyz.xyz += att_normal * u_shellParams.y;
}

v_texCoord = att_texCoordDiffuse;
v_color = att_color4f;

gl_Position = u_modelViewProjectionMatrix * xyz;
}
