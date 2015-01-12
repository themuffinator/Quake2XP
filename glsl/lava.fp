varying vec2			v_diffuseTexCoord;
varying vec3			v_viewVec;
varying vec3  			v_lightVec;
varying vec3			v_tbn[3];
varying vec3			v_positionVS;

uniform sampler2D		u_colorMap;
uniform sampler2D		u_csmMap;

#include parallax.inc

void main()
{
vec3 V = normalize(v_viewVec);
vec3 Vp = normalize(v_positionVS);
vec3 T = normalize(v_tbn[0]);
vec3 B = normalize(v_tbn[1]);
vec3 N = normalize(v_tbn[2]);
vec2 texCoord;
vec2 P;

if(u_parallaxType == 1){
	// ray intersection in view direction
	float a = abs(dot(N, Vp));
	a = sin(clamp(a, 0.0, 1.0) * HALF_PI) / a;	// thx Berserker for corner artifact correction
	vec3 dp = vec3(v_diffuseTexCoord, 0.0);
	vec3 ds = vec3(a * u_parallaxParams.x * dot(T, Vp), a * u_parallaxParams.y * dot(B, Vp), 1.0);
	float distFactor = 0.05 * sqrt(length(fwidth(v_diffuseTexCoord)));
	IntersectConeExp(u_csmMap, dp, ds, distFactor);
	P = dp.xy;
}

if(u_parallaxType == 2)	
	P = CalcParallaxOffset(u_colorMap, v_diffuseTexCoord.xy, V);

if (u_parallaxType >=1)
	texCoord = P;
else
	texCoord = v_diffuseTexCoord;

//load diffuse map
vec4 diffuse  = texture2D (u_colorMap, texCoord.xy);
 
gl_FragColor = vec4(diffuse.rgb * 0.33, 1.0);
}
