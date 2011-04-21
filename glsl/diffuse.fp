uniform sampler2D	u_Diffuse;
uniform sampler2D	u_LightMap;
uniform sampler2D	u_Add;
uniform float       u_ColorModulate;  
uniform vec2		u_bumpScale;
uniform sampler2D	u_Caustics;
uniform float       u_CausticsModulate; 

varying vec3		v_viewVecTS;

/*
===================
ParallaxOcclusionMap

High quality, uses Crysis(tm) shader.
===================
*/

vec2 ParallaxOcclusionMap (in sampler2D hiMap, in vec2 texCoord, in vec3 viewVec, int numSteps) {
	float step = 1.0 / float(numSteps);
	
	vec2 delta = 2.0 * u_bumpScale * viewVec.xy /    (-viewVec.z * float(numSteps));
		
	float NB0 = texture2D(hiMap, texCoord).a;
			
	float height = 1.0 - step;
	vec2 offset = texCoord + delta;
	float NB1 = texture2D(hiMap, offset).a;
	
	for (int i =0; i<numSteps; i++) {
		if (NB1 >= height)
			break;
			
		NB0 = NB1;

		height -= step;
		offset += delta;

		NB1 = texture2D(hiMap, offset).a;
	}
	
	vec2 offsetBest = offset;
	float error = 1.0;

	float t1 = height;
	float t0 = t1 + step;
	
	float delta1 = t1 - NB1;
	float delta0 = t0 - NB0;

	vec4 intersect = vec4(delta * float(numSteps), delta * float(numSteps) + texCoord);
   
	for (int i = 0; i < 10; i++) {
		if (abs(error) <= 0.01)
			break;
	
		float denom = delta1 - delta0;
		float t = (t0 * delta1 - t1 * delta0) / denom;
		offsetBest = -t * intersect.xy + intersect.zw;
		
		float NB = texture2D(hiMap, offsetBest).a;

		error = t - NB;
		if (error < 0.0) {
			delta1 = error;
			t1 = t;
		}
		else {
			delta0 = error;
			t0 = t;
		}
	}
	
	return offsetBest;
}

void main ()
{
vec4 tmp;

#ifdef PARALLAX
vec3 V = normalize(v_viewVecTS);
vec2 P = ParallaxOcclusionMap(u_Diffuse, gl_TexCoord[0].xy, V, 10);

vec4 r0 = texture2D(u_Diffuse,  P);
vec4 r1 = texture2D(u_LightMap, gl_TexCoord[1].xy); 
vec4 r2 = texture2D(u_Add,      P);
vec4 r3 = texture2D(u_Caustics, P);


#else

vec4 r0 = texture2D(u_Diffuse,  gl_TexCoord[0].xy);
vec4 r1 = texture2D(u_LightMap, gl_TexCoord[1].xy); 
vec4 r2 = texture2D(u_Add,      gl_TexCoord[0].xy);
vec4 r3 = texture2D(u_Caustics, gl_TexCoord[0].xy);
#endif 

vec4 color;

#ifdef LIGHTMAP
r0 *= r1;
#endif

#ifdef VERTEXLIGHT
r0 *= gl_Color;
#endif

color = r0 + r2;

#ifdef CAUSTICS
tmp = r3 * color;
tmp *= u_CausticsModulate;
color = tmp + color;
#endif

gl_FragColor = color * u_ColorModulate;

}
