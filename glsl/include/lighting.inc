#define	CUTOFF_EPSILON		1.0/255.0		// light intensity below which fragment is discarded

vec2 PhongLighting (const in vec3 N, const in vec3 L, const in vec3 V, const float sExp) {
	vec2 E;
	E.x = max(dot(N, L), 0.0);
	vec3 R = reflect(-L, N);
	E.y = pow(max(dot(R, V), 0.0), sExp);
	E.y *= E.x;
	return E;
}

float halfLambert(in vec3 N, in vec3 L )  
{  
     return dot ( N, L ) * 0.5 + 2.0;
} 

float LambertLighting(const in vec3 N, const in vec3 L) {
	float E;
	return E = max(dot (N, L), 0.0);
}

float blinnPhongSpecular(in vec3 normalVec, in vec3 lightVec, in float specPower)
{
    vec3 halfAngle = normalize(normalVec + lightVec);
    return pow(clamp(0.0,1.0,dot(normalVec,halfAngle)),specPower);
}

vec4 subScatterFS(in vec3 V, in vec3 L, in vec3 N, vec3 lightColor, in vec4 Diffuse, in float Att, in float Specular)
{
    float MaterialThickness = 2.0;
    vec3 ExtinctionCoefficient = vec3(0.80, 0.12, 0.20);
    float RimScalar = 10.0;

    vec4 dotLN = vec4(halfLambert(L, N) * Att);
    dotLN *= Diffuse;
     
    vec3 indirectLightComponent = vec3(MaterialThickness * max(0.0,dot(-N, L)));
    indirectLightComponent += MaterialThickness * halfLambert(-V, L);
    indirectLightComponent *= Att;
    indirectLightComponent.r *= ExtinctionCoefficient.r;
    indirectLightComponent.g *= ExtinctionCoefficient.g;
    indirectLightComponent.b *= ExtinctionCoefficient.b;
     
    vec3 rim = vec3(1.0 - max(0.0,dot(N, V)));
    rim *= rim;
    rim *= max(0.0,dot(N,L)) * Specular;
     
    vec4 finalCol = dotLN + vec4(indirectLightComponent,1.0);
    finalCol.rgb += (rim * RimScalar * Att * finalCol.a);
    
    finalCol.rgb += vec3(blinnPhongSpecular(N,L,1.0) * Att * Specular * finalCol.a * 0.05);
    finalCol.rgb *= lightColor.rgb;
    return finalCol;   
}