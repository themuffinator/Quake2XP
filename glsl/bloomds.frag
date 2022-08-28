//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0) uniform sampler2DRect u_map; 

layout (location = U_PARAM_FLOAT_0)	uniform float u_BloomThreshold;   

void main()   
{  
vec4 tex = texture( u_map, gl_FragCoord.xy);  
// Calculate luminance
float lum = dot(vec4(0.2125, 0.7154, 0.0721, 0.0), tex);

// Extract very bright areas of the map.
if (lum >= u_BloomThreshold)
    fragData = tex;
else
    fragData = vec4(0.0);
}
