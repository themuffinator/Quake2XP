layout (bindless_sampler, location  = U_TMU0) uniform	sampler2DRect	u_map;

layout(location = U_PARAM_FLOAT_0) uniform float	u_glareParams;

void main(void) // Robert Beckebans hdr glare
{
	vec2 st = gl_FragCoord.xy;
	
	// base color with tone mapping applied
	vec4 color = texture2DRect( u_map, st );
	
	const float gaussFact[9] = float[9](0.13298076, 0.12579441, 0.10648267, 0.08065691, 0.05467002, 0.03315905, 0.01799699, 0.00874063, 0.00379866);
	
	const vec3 chromaticOffsets[9] = vec3[](
	vec3(0.5, 0.5, 0.5), // w
	vec3(0.8, 0.3, 0.3),
//	vec3(1.0, 0.2, 0.2), // r
	vec3(0.5, 0.2, 0.8),
	vec3(0.2, 0.2, 1.0), // b
	vec3(0.2, 0.3, 0.9),
	vec3(0.2, 0.9, 0.2), // g
	vec3(0.3, 0.5, 0.3),
	vec3(0.3, 0.5, 0.3),
	vec3(0.3, 0.5, 0.3)
	//vec3(0.3, 0.5, 0.3)
	);
	
	vec3 sumColor = vec3( 0.0 );
	vec3 sumSpectrum = vec3( 0.0 );

	const int samples = 9;
	
	float scale = 3.0; // bloom width
	const float weightScale = 1.5; // bloom strength
	
	for( int i = 0; i < samples; i++ )
    {
		vec3 so = chromaticOffsets[ i ];
		vec4 color = texture2DRect( u_map, st + vec2( float( i ), 0 )  *  scale );
			
		float weight = gaussFact[ i ];
		sumColor += color.rgb * ( so.rgb * weight * weightScale );
	}
	
#if 1
	for( int i = 1; i < samples; i++ )
    {
		vec3 so = chromaticOffsets[ i ];
		vec4 color = texture2DRect( u_map, st + vec2( float( -i ), 0 )  *  scale );
			
		float weight = gaussFact[ i ];
		sumColor += color.rgb * ( so.rgb * weight * weightScale );
	}
#endif
	
	fragData = vec4( sumColor, 1.0 );
}


void main2(void)
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



