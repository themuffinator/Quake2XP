layout (binding = 0) uniform	sampler2DRect	u_map;

uniform vec3	u_glareParams;

#include tonemap.inc

// hdr glare func by Robert Beckebans
void main(void){

  vec2 st = gl_FragCoord.xy;
  
  // base color with tone mapping applied
  vec4 color = texture2DRect( u_map, st );
  
  const float gaussFact[9] = float[9](0.13298076, 0.12579441, 0.10648267, 0.08065691, 0.05467002, 0.03315905, 0.01799699, 0.00874063, 0.00379866);
  
  const vec3 chromaticOffsets[9] = vec3[](
  vec3(0.5, 0.5, 0.5), // w
  vec3(0.8, 0.3, 0.3),
  vec3(0.5, 0.2, 0.8),
  vec3(0.2, 0.2, 1.0), // b
  vec3(0.2, 0.3, 0.9),
  vec3(0.2, 0.9, 0.2), // g
  vec3(0.3, 0.5, 0.3),
  vec3(0.3, 0.5, 0.3),
  vec3(0.3, 0.5, 0.3)
  );
  
  vec3 sumColor = vec3( 0.0 );
  vec3 sumSpectrum = vec3( 0.0 );

  const int samples = 9;
  
  float scale = u_glareParams.x; // bloom width
  const float weightScale = u_glareParams.x * 2.666; // bloom strength

// to left  
  for( int i = 0; i < samples; i++ )
    {
    vec3 so = chromaticOffsets[ i ];
    vec4 color = texture2DRect( u_map, st + vec2( float( i ), 0 ) * scale );
      
    float weight = gaussFact[ i ];
    sumColor += color.rgb * ( so.rgb * weight * weightScale );
  }
  
// to right
  for( int i = 1; i < samples; i++ )
    {
    vec3 so = chromaticOffsets[ i ];
    vec4 color = texture2DRect( u_map, st + vec2( float( -i ), 0 ) * scale );
      
    float weight = gaussFact[ i ];
    sumColor += color.rgb * ( so.rgb * weight * weightScale );
	}
  // to ldr
	fragData = vec4(performTonemap(sumColor), 1.0);
}