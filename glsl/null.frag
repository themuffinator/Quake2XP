//!#include "include/global.inc"

layout (location = 2) out vec4 fogmask;
in  vec3 color;
void main(void) 
{
    fogmask = vec4(color, 1.0);
    fragData = vec4(0.0, 0.0, 0.0, 1.0);
}
