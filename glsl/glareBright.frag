//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0) uniform sampler2D u_map; 
layout(location = U_SCREEN_SIZE)              uniform vec2	    u_screenSize;

void main()   
{
vec2 st =  gl_FragCoord.xy / u_screenSize;
    
vec4 tex = texture( u_map, st);  
// Calculate luminance
float lum = dot(vec4(0.2125, 0.7154, 0.0721, 0.0), tex);

// Extract very bright areas of the map.
if (lum >= 1.0)
  fragData = vec4(1.0);
else
  fragData = vec4(0.0);
}
