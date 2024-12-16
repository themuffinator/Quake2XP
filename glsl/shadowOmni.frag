//!#include "include/global.inc"
in vec3 pos;
void main(void) 
{
    fragData.rgb = vec3(length(pos));
    fragData.a = 0.0;
     
}
