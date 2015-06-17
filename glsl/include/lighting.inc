#define	CUTOFF_EPSILON		1.0/255.0		// light intensity below which fragment is discarded

float LambertLighting(const in vec3 N, const in vec3 L) {
	return max(dot (N, L), 0.0);
}

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
     return dot ( N, L ) * 0.5 + 0.5;
} 

float blinnPhongSpecular(in vec3 normalVec, in vec3 lightVec, in float specPower)
{
    vec3 halfAngle = normalize(normalVec + lightVec);
    return pow(clamp(dot(normalVec,halfAngle),0.0,1.0), specPower);
}

vec4 subScatterFS(in vec3 V, in vec3 L, in vec3 N, vec3 lightColor, in vec4 Diffuse, in float Att, in float Specular)
{
    float MaterialThickness = 0.0666;  // SATAN!!!!
    vec3 ExtinctionCoefficient = vec3(0.80, 0.12, 0.20);
    float RimScalar = 5.0;

    vec4 dotLN = vec4(halfLambert(L, N) * Att);
    dotLN *= Diffuse;
     
    vec3 indirectLightComponent = vec3(MaterialThickness * max(dot(-N, L), 0.0));
    indirectLightComponent += MaterialThickness * halfLambert(-V, L);
    indirectLightComponent *= Att * ExtinctionCoefficient;
     
    vec3 rim = vec3(1.0 - max(dot(N, V), 0.0));
    rim *= rim;
    rim *= max(dot(N,L),0.0) * Specular;
     
    vec4 finalCol = dotLN + vec4(indirectLightComponent,1.0);
    finalCol.rgb += (rim * RimScalar * Att * finalCol.a);
    
    finalCol.rgb += vec3(blinnPhongSpecular(N,L, 0.5) * Att * Specular * finalCol.a * 0.05);
    finalCol.rgb *= lightColor.rgb;
    return finalCol;   
}