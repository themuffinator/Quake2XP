//!#include "include/global.inc"
layout (bindless_sampler, location = U_TMU0) uniform sampler2DRect	u_ScreenTex;
layout (bindless_sampler, location = U_TMU1) uniform sampler3D		u_lutTex;

layout(location = U_PARAM_VEC3_0)	uniform vec3	u_lutSize;

void main(void){
//Apply LUT Table 
//developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter24.html
vec3 rawColor = texture2DRect(u_ScreenTex, gl_FragCoord.xy).rgb;
vec3 scale = (u_lutSize - 1.0) / u_lutSize;
vec3 offset = 1.0 / (2.0 * u_lutSize);
fragData.rgb = texture(u_lutTex, scale * rawColor.rgb + offset).rgb;
fragData.a = 1.0;
}
