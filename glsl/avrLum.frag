//!#include "include/global.inc"

layout (bindless_sampler, location = U_TMU0) uniform sampler2D  	u_LumTex;

void main(void){
	vec3	currLum = exp2(textureLod(u_LumTex, vec2(0.5), 8).rgb);
	float	lum     = log2(dot(vec3(0.2125, 0.7154, 0.0721), currLum));
	
	fragData.rgb = vec3(lum);
	fragData.a = 1.0;
}

