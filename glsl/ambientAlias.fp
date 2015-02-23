varying	vec2		v_texCoord;
varying	vec4		v_color;

uniform sampler2D 	u_Diffuse;
uniform sampler2D 	u_Add;

uniform sampler2D	u_env;
uniform float		u_envScale;
uniform int			u_isEnvMap;

uniform	int			u_isShell;

varying		vec2	v_envCoord;
varying		vec2	v_shellCoord;

uniform float       u_ColorModulate;  
uniform float       u_AddShift; 

void main ()
{

if(u_isShell == 1){
	vec4 r0 = texture2D(u_Diffuse,  v_shellCoord);
	gl_FragColor = r0;
	return;
}

vec4 r0 = texture2D(u_Diffuse,  v_texCoord);
vec4 r1 = texture2D(u_Add,      v_texCoord);

vec4 color;
r0 *= v_color;
r1.rgb *= u_AddShift;
color = r0+r1;

if(u_isEnvMap == 1){
	vec4 r3 = texture2D(u_env,  v_envCoord);
	r3 *= r1.a;
	r3 *= u_envScale;
	color += r3;
}

gl_FragColor = color * u_ColorModulate;

}
