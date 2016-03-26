#ifndef saturate
#define saturate(x)    clamp(x, 0.0, 1.0)
#endif

float sqr (in float x) { return x * x; }
vec2 sqr (in vec2 x) { return x * x; }
vec3 sqr (in vec3 x) { return x * x; }
vec4 sqr (in vec4 x) { return x * x; }


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

float PointAttenuation (const in vec3 p, const in float e) {
	return pow(max(1.0 - dot(p, p), 0.0), e);
}

/*
============================

Sub Surface Scattering

============================
*/

float halfLambert(in vec3 N, in vec3 L )  
{  
     return dot ( N, L ) * 0.5 + 0.5;
} 

float blinnPhongSpecular(in vec3 normalVec, in vec3 lightVec, in float specPower)
{
    vec3 halfAngle = normalize(normalVec + lightVec);
    return pow(clamp(dot(normalVec,halfAngle),0.0,1.0), specPower);
}

vec4 SkinLighting(in vec3 V, in vec3 L, in vec3 N, vec3 lightColor, in vec4 Diffuse, in float Att, in float Specular)
{
    float MaterialThickness = 0.3;
    vec3 ExtinctionCoefficient = vec3(0.25, 0.1, 0.07);
    float backFaceSpecularIntens = 0.666;

    vec4 dotLN = vec4(halfLambert(L, N) * Att);
    dotLN *= Diffuse;
     
    vec3 indirectLightComponent = vec3(MaterialThickness * max(dot(N, L), 0.0));
    indirectLightComponent += MaterialThickness * halfLambert(V, L);
    indirectLightComponent *= Att * ExtinctionCoefficient;
     
    vec3 rim = vec3(1.0 - max(dot(N, V), 0.0));
    rim *= rim;
    rim *= max(dot(N,L),0.0) * Specular;
     
    vec4 finalCol = dotLN + vec4(indirectLightComponent, 1.0);
    finalCol.rgb += (rim * backFaceSpecularIntens * Att * finalCol.a);
    
    finalCol.rgb += vec3(blinnPhongSpecular(N,L, 0.5) * Att * Specular * finalCol.a * 0.05);
    finalCol.rgb *= lightColor.rgb;
    finalCol.a = 1.0;
    return finalCol;   
}

/*
=================================================================

	BRDF

=================================================================
*/

/*
==============
Diffuse_*

Diffuse term.
==============
*/
vec3 Diffuse_Lambert (in vec3 color) {
	return color * INV_PI;
}

vec3 Diffuse_Burley (in vec3 color, in float roughness, in float NV, in float NL, in float VH) {
	float FD90 = 0.5 + 2.0 * VH * VH * roughness;
	float FdV = 1.0 + (FD90 - 1.0) * pow(1.0 - NV, 5.0);
	float FdL = 1.0 + (FD90 - 1.0) * pow(1.0 - NL, 5.0);

	return color * INV_PI * FdV * FdL;
}

vec3 Diffuse_OrenNayar (in vec3 color, in float roughness, in float NV, in float NL, in float VH) {
	float VL = 2.0 * VH - 1.0;
	float m = roughness * roughness;
	float m2 = m * m;
	float C1 = 1.0 - 0.5 * m2 / (m2 + 0.33);
	float Cosri = VL - NV * NL;
	float C2 = 0.45 * m2 / (m2 + 0.09) * Cosri * (Cosri >= 0.0 ? min(1.0, NL / NV) : NL);

	return color * INV_PI * (NL * C1 + C2);
}

/*
==============
SpecularD_*

Microfacet distribution function.
==============
*/
float SpecularD_Blinn (in float roughness, in float NH) {
	float m = roughness * roughness;
	float m2 = m * m;
	float n = 2.0 / m2 - 2.0;

	return (n + 2.0) / (2.0 * PI) * pow(max(abs(NH), 0.000001f), n);
}

float SpecularD_Beckmann (in float roughness, in float NH) {
	float m = roughness * roughness;
	float m2 = m * m;
	float NH2 = NH * NH;

	return exp((NH2 - 1.0) / (m2 * NH2)) / (PI * m2 * NH2 * NH2);
}

float SpecularD_GGX (in float roughness, in float NH) {
	float m = roughness * roughness;
	float m2 = m * m;
	float d = (NH * m2 - NH) * NH + 1.0;

	return m2 / (PI * d * d);
}

float SpecularD_GGXAnisotropic (in vec2 roughness, in float NH, in vec3 H, in vec3 X, in vec3 Y) {
	float mx = roughness.x * roughness.x;
	float my = roughness.y * roughness.y;
	float XH = dot(X, H);
	float YH = dot(Y, H);
	float d = XH * XH / (mx * mx) + YH * YH / (my * my) + NH * NH;

	return 1.0 / (PI * mx * my * d * d);
}

/*
==============
SpecularG_*

Approximation of GGX geometric visibility term (Heitz).
==============
*/

float SpecularG_Neumann (in float NV, in float NL) {
	return 1.0 / (4.0 * max(NL, NV));
}

float SpecularG_Kelemen (in float VH) {
	return 1.0 / (4.0 * VH * VH);
}

float SpecularG_Schlick (in float roughness, in float NV, in float NL) {
	float k = sqr(roughness) * 0.5;
	float schlickV = NV * (1.0 - k) + k;
	float schlickL = NL * (1.0 - k) + k;

	return 0.25 / (schlickV * schlickL);
}

// Smith term for GGX
// [Smith 1967, "Geometrical shadowing of a random rough surface"]
float SpecularG_Smith (in float roughness, in float NV, in float NL) {
	float a = sqr(roughness);
	float a2 = a * a;

	float smithV = NV + sqrt(NV * (NV - NV * a2) + a2);
	float smithL = NL + sqrt(NL * (NL - NL * a2) + a2);

	return 1.0 / (smithV * smithL);
}

float SpecularG_SmithApprox (in float roughness, in float NV, in float NL) {
	float a = sqr(roughness);
	float GV = NL * (NV * (1.0 - a) + a);
	float GL = NV * (NL * (1.0 - a) + a);

	return 0.5 / (GV + GL);
}

/*
==============
SpecularF_*

Fresnel term.
==============
*/
vec3 SpecularF_Schlick (in vec3 color, in float VH) {
	float f = pow(1.0 - VH, 5.0);
	
	// clamp reflectance to a minimum of 2% of the "brightest" green channel
	return mix(color, saturate(50.0 * color.ggg), f);
}

vec3 SpecularF_Fresnel (in vec3 color, in float VH) {
	vec3 color0 = sqrt(clamp(color, 0.0, 0.99));
	vec3 n = (1.0 + color0) / (1.0 - color0);
	vec3 g = sqrt(n * n + VH * VH - 1.0);
	vec3 a = (g - VH) / (g + VH);
	vec3 b = ((g + VH) * VH - 1.0) / ((g - VH) * VH + 1.0);

	return 0.5 * a * a * (1.0 + b * b);
}

/*
==============
Lighting_BRDF

Convolves lighting equation from a point light.
==============
*/
vec3 Lighting_BRDF (in vec3 diffuseColor, in vec3 specularColor, in float roughness, in vec3 normal, in vec3 lightVec, in vec3 viewVec) {
	float NL = saturate(dot(normal, lightVec));
   
	if (NL > 0.0) {
		float NV = abs(dot(normal, viewVec)) + 0.00001;
		vec3 halfVec = normalize(lightVec + viewVec);
		float NH = saturate(dot(normal, halfVec));
		float VH = saturate(dot(viewVec, halfVec));

		vec3 diffuse =  Diffuse_OrenNayar(diffuseColor, roughness, NV, NL, VH);
		float D = SpecularD_GGX(roughness, NH);
		float G = SpecularG_SmithApprox(roughness, NV, NL);
  		vec3 F = SpecularF_Schlick(specularColor, VH);

		return NL * (diffuse + D * G * F);
	}

	return vec3(0.0);
}