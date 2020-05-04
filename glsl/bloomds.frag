layout (bindless_sampler, location  = U_TMU0) uniform sampler2DRect u_map; 

layout (location = U_PARAM_FLOAT_0)	uniform float u_BloomThreshold;   

void main()   
{  
vec2 tc = gl_FragCoord.xy * 4.0; 

vec4 tex = texture2DRect( u_map, tc );  

tex += texture2DRect( u_map, tc + vec2(1.0, 0.0)); 
tex += texture2DRect( u_map, tc + vec2(1.0, 1.0)); 
tex += texture2DRect( u_map, tc + vec2(0.0, 1.0)); 
tex *= 0.25;

// Calculate luminance
float lum = dot(vec4(0.2125, 0.7154, 0.0721, 0.0), tex);

// Extract very bright areas of the map.
if (lum > u_BloomThreshold)
    fragData = tex;
else
    fragData = vec4(0.0);
}
