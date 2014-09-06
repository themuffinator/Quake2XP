uniform sampler2D		u_bumpMap;
uniform sampler2D		u_diffuseMap;
uniform samplerCube 	u_CubeFilterMap;
uniform sampler3D	 	u_attenMap;

uniform float           u_LightRadius;
uniform float			u_specularScale;
uniform vec4			u_LightColor;

uniform float			u_fogDensity;
uniform int				u_fog;

varying vec2			v_texCoord;
varying vec3			v_viewVec;
varying vec3			v_lightVec;
varying vec4			v_CubeCoord;
varying vec3			v_AttenCoord;

#include lighting.inc

void main(){

// compute the light vector
vec3 L = normalize(v_lightVec);
// compute the view vector
vec3 V = normalize(v_viewVec);

vec3 N =  normalize(texture2D(u_bumpMap, v_texCoord).rgb * 2.0 - 1.0);
float tmp = texture2D(u_bumpMap,   v_texCoord.xy).a;

float specular = tmp * u_specularScale;
specular /= mix(0.5, 1.0, specular);

vec4 diffuse  = texture2D(u_diffuseMap,  v_texCoord.xy);
vec4 cubeFilter = textureCube(u_CubeFilterMap, v_CubeCoord.xyz);
cubeFilter *= 2;

vec4 u_attenMap = texture3D(u_attenMap ,v_AttenCoord);

#ifdef AMBIENT

if(u_fog == 1){

const float LOG2 = 1.442695;
float z = gl_FragCoord.z / gl_FragCoord.w;
float fogFactor = exp2(-u_fogDensity * u_fogDensity * z * z * LOG2);

vec4 temp = diffuse * u_LightColor;
gl_FragColor = (mix(u_LightColor, temp, fogFactor)) * u_attenMap;

}else{

gl_FragColor = diffuse * LambertLighting(N, V) * u_LightColor * u_attenMap;

}
#else

vec2 E = PhongLighting(N, L, V, 16.0);
gl_FragColor = (E.x * diffuse + E.y * specular) * cubeFilter * u_attenMap * u_LightColor;

#endif
}
