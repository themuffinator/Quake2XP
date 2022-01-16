//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0) uniform sampler2DRect u_stencilTex; 

#include blur.inc //!#include "include/blur.inc"

void main()   
{  
//    float stencilTex = boxBlur(u_stencilTex, gl_FragCoord.xy, 16.0).r;
    fragData = texture2DRect(u_stencilTex, gl_FragCoord.xy);
}
