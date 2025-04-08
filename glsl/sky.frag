//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0)   uniform samplerCube	u_map;

in vec3	v_texCoord; 

void main(void){

    fragData = pow(textureLod(u_map, v_texCoord.xyz, 0), vec4(2.2));
    fragData *= 1.2;
}
