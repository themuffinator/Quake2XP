varying	vec2		v_texCoord;
varying	vec4		v_color;

uniform sampler2D 	u_Diffuse;
uniform sampler2D 	u_Add;

#ifdef ENVMAP
uniform sampler2D	u_env;
uniform float		u_envScale;
varying		vec2	v_envCoord;
#endif

#ifdef SHELL
varying		vec2	v_shellCoord;
#endif

uniform float       u_ColorModulate;  
uniform float       u_AddShift; 

void main ()
{
#ifdef WEAPON
gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0); // weapon mask
#endif

#ifdef SHELL
vec4 r0 = texture2D(u_Diffuse,  v_shellCoord);
gl_FragColor = r0 * u_ColorModulate;

#else

vec4 r0 = texture2D(u_Diffuse,  v_texCoord);
vec4 r1 = texture2D(u_Add,      v_texCoord);

vec4 color;
r0 *= v_color;
r1.rgb *= u_AddShift;
color = r0+r1;

#ifdef ENVMAP
vec4 r3 = texture2D(u_env,  v_envCoord);
r3 *= r1.a;
r3*= u_envScale;
color += r3;
#endif

gl_FragColor = color * u_ColorModulate;
#endif
}
