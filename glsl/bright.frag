//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0) uniform sampler2DRect u_map; 
layout (location = U_PARAM_INT_0)	uniform int glare;

void main()   
{  
vec4 tex = texture( u_map, gl_FragCoord.xy);  
// Calculate luminance
float lum = dot(vec4(0.2125, 0.7154, 0.0721, 0.0), tex);

// Extract very bright areas of the map.
if (lum >= 1.0){
    if(glare == 1)
        fragData = vec4(1.0);
    else
        fragData = tex; // legacy bloom
    }
else
    fragData = vec4(0.0);
}
