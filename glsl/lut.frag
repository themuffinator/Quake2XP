layout (binding = 0) uniform sampler2DRect	u_ScreenTex;
layout (binding = 1) uniform sampler3D		u_lutTex;

layout(location = U_PARAM_FLOAT_0) uniform float u_lutSize;

void main(void){

vec3 rawColor = texture2DRect(u_ScreenTex, gl_FragCoord.xy).rgb;

//Apply LUT Table 
//developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter24.html

vec3 scale = vec3((u_lutSize - 1.0) / u_lutSize);
vec3 offset = vec3(1.0 / (2.0 * u_lutSize));

fragData.rgb = texture(u_lutTex, scale * rawColor.rgb + offset).rgb;
fragData.a = 1.0;
}
