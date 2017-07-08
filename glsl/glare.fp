layout (binding = 0) uniform	sampler2DRect	u_map;

uniform float	u_glareParams;

void main(void)
{
	vec2 fragCoord = gl_FragCoord.xy;
	vec2 dx = vec2 (1.0) * u_glareParams;
	vec2 dx2 = vec2 (-1.0, 1.0) * u_glareParams; 

	const 	vec4 chromaticOffsets[9] = vec4[](    // Robert Beckebans cromatic offsets 
			vec4(0.5, 0.5, 0.5, 1.0), // w
			vec4(0.8, 0.3, 0.3, 1.0),
			vec4(0.5, 0.2, 0.8, 1.0),
			vec4(0.2, 0.2, 1.0, 1.0), // b
			vec4(0.2, 0.3, 0.9, 1.0),
			vec4(0.2, 0.9, 0.2, 1.0), // g
			vec4(0.3, 0.5, 0.3, 1.0),
			vec4(0.3, 0.5, 0.3, 1.0),
			vec4(0.3, 0.5, 0.3, 1.0)
			);
  
	const float gaussFact[9] = float[9](0.13298076, 0.12579441, 0.10648267, 0.08065691, 0.05467002, 0.03315905, 0.01799699, 0.00874063, 0.00379866);
  
    vec2 sdx = dx;
    vec2 sdx2 = dx2;
    vec4 sColor = ( texture2DRect(u_map, fragCoord) * gaussFact[0] ) * chromaticOffsets[0];
    
    for (int i = 1; i < 9; i++){
		sColor += ( texture2DRect(u_map, fragCoord + sdx) + texture2DRect(u_map, fragCoord - sdx) ) *  gaussFact[i] * chromaticOffsets[i];
		sdx += dx;
    } 
    
    for (int i = 1; i < 9; i++){
		  sColor += ( texture2DRect(u_map, fragCoord + sdx2) + texture2DRect(u_map, fragCoord - sdx2) ) *  gaussFact[i] * chromaticOffsets[i];
		  sdx2 += dx2;
    }

    fragData = sColor;
}

