//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0)   uniform samplerCube	u_map;

in vec3	v_texCoord; 

void main(void){

    fragData = textureLod(u_map, v_texCoord.xyz, 0);
    fragData *= 1.2;
}
