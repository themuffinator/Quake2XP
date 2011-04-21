uniform sampler2D			u_bumpMap;
uniform sampler2D			u_diffuseMap;

uniform float               u_LightRadius;
uniform vec3				u_LightColor;

varying vec2				v_texCoord;
varying vec3				v_viewVec;
varying vec3				v_lightVec;

/*
==============
PhongLighting

Returns diffuse and specular intensities.
==============
*/
vec2 PhongLighting (const in vec3 N, const in vec3 L, const in vec3 V, const float sExp) {
	vec2 E;
	E.x = max(dot(N, L), 0.0);

	vec3 R = reflect(-L, N);
	E.y = pow(max(dot(R, V), 0.0), sExp);
	E.y *= (E.x < 0.01 ? 0.0 : 1.0);


	return E;
}

void main(){

vec3 N =  normalize(texture2D(u_bumpMap, v_texCoord).rgb * 2.0 - 1.0);
float tmp = texture2D(u_bumpMap,   v_texCoord.xy).a;
vec4 specular = vec4(tmp, tmp, tmp, tmp);
vec4 diffuse  = texture2D(u_diffuseMap,  v_texCoord.xy);

// compute the atten
vec3 tmp1 = v_lightVec;
tmp1 /= u_LightRadius;
float att = max(1.0 - dot(tmp1, tmp1), 0.0);

// compute the light vector
vec3 L = normalize(v_lightVec);

// compute the view vector
vec3 V = normalize(v_viewVec);

vec2 E = PhongLighting(N, L, V, 16.0);

E *= att;

gl_FragColor.rgb = (E.x * diffuse.rgb + E.y * specular.rgb)* u_LightColor;

}