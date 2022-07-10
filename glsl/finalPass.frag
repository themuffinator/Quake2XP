//!#include "include/global.inc"
layout (bindless_sampler, location = U_TMU0) uniform sampler2DRect	u_ScreenTex;

void main(){

    vec4 fragData = texture2DRect(u_ScreenTex, gl_FragCoord.xy);

    float gamma = 1.0 / 2.2; // hdr gamma
    fragData.rgb = pow( fragData.rgb, vec3(gamma) );
    fragData.a = 1.0;
}