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

#ifdef CAUSTICS
uniform sampler2D  u_Caustics;
uniform float      u_CausticsModulate;
#endif

uniform float      u_ColorModulate;  
uniform float      u_AddShift; 


void main ()
{
#ifdef SHELL
vec4 r0 = texture2D(u_Diffuse,  v_shellCoord);
gl_FragColor = r0 * u_ColorModulate;

#else

vec4 r0 = texture2D(u_Diffuse,  v_texCoord);
vec4 r1 = texture2D(u_Add,      v_texCoord);

#ifdef CAUSTICS
vec4 r2 = texture2D(u_Caustics, v_texCoord);
vec4 tmp;
#endif

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

#ifdef CAUSTICS
tmp = r2 * color;
tmp *= u_CausticsModulate;
color = tmp + color;
#endif

gl_FragColor = color * u_ColorModulate;
#endif
}
