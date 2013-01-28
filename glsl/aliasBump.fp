uniform sampler2D		u_bumpMap;
uniform sampler2D		u_diffuseMap;
uniform samplerCube 	u_CubeFilterMap;

uniform float           u_LightRadius;
uniform vec3			u_LightColor;

varying vec2			v_texCoord;
varying vec3			v_viewVec;
varying vec3			v_lightVec;
varying vec4			v_CubeCoord;


#include lighting.inc

void main(){

vec3 N =  normalize(texture2D(u_bumpMap, v_texCoord).rgb * 2.0 - 1.0);
float tmp = texture2D(u_bumpMap,   v_texCoord.xy).a;
vec4 specular = vec4(tmp, tmp, tmp, tmp);
vec4 diffuse  = texture2D(u_diffuseMap,  v_texCoord.xy);
vec4 cubeFilter = textureCube(u_CubeFilterMap, v_CubeCoord.xyz);

// compute the atten
vec3 tmp1 = v_lightVec;
tmp1 /= u_LightRadius;
float att = max(1.0 - dot(tmp1, tmp1), 0.0);

// compute the light vector
vec3 L = normalize(v_lightVec);

// compute the view vector
vec3 V = normalize(v_viewVec);

vec2 E = PhongLighting(N, L, V, 16.0);

gl_FragColor.rgb = cubeFilter * att * vec4(u_LightColor, 1) * (E.x * diffuse + E.y * specular);

}
