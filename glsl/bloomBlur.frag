//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0) uniform sampler2DRect u_map;  
layout (location = U_PARAM_INT_0)	uniform bool horizontal;

const float weight[13] = float[](   0.015625,0.0596875,0.09375,0.1640625,0.234375, 0.2734375, 0.3125,
                                    0.2734375,0.234375,0.1640625, 0.09375, 0.0596875, 0.015625);

 void main(void){
    int i;
    float ofs = 1.0;
    vec4 color;

    if(horizontal){
        vec2 tc = gl_FragCoord.xy + vec2(-6.0, 0.0);
        
        for(i = 0; i < 13; i++){
            color += weight[i] * texture(u_map, tc );
            tc.x +=ofs;
        }
       fragData = color;
       fragData.a = 1.0;
    }
    
    if(!horizontal){
        vec2 tc = gl_FragCoord.xy + vec2(0.0, -6.0);
        
        for(i = 0; i < 13; i++){
            color += weight[i] * texture(u_map, tc );
            tc.y +=ofs;
        }
       fragData = color;
       fragData.a = 1.0;
    }
} 