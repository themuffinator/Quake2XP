layout (location = U_PARALLAX_PARAMS)	uniform vec4			u_parallaxParams; // x = scale[0] y = scale[1] z = tex_w[2] w = tex_h
layout (location = U_PARALLAX_TYPE)		uniform int				u_parallaxType;
layout (location = U_PARAM_FLOAT_0)		uniform float			u_shadowOffset;



float ComputeLOD( vec2 tc, vec2 texSize ) { 
 vec2 dx = dFdx( tc ); 
 vec2 dy = dFdy( tc ); 
 vec2 mag = ( abs( dx )  + abs( dy )  ) * texSize; 
 float lod = log2( max( mag.x, mag.y ) ); 
 return lod; 
} 

vec2 CalcParallaxOffset (in sampler2D hiMap, in vec2 texCoord, in vec3 viewVec) {
	
	// 14 sample relief mapping: linear search and then binary search, 
	// improved qfusion shader
	float lod = ComputeLOD(texCoord, u_parallaxParams.zw);
	vec3 offsetBest;

  
  if(u_parallaxType >= 1) {
  vec3 offsetVector = vec3(viewVec.xy * u_parallaxParams.xy * vec2(-1.0, -1.0), -1.0);
	offsetBest = vec3(texCoord, 1.0);
	offsetVector *= 0.1;

	offsetBest += offsetVector *  step(textureLod(hiMap, offsetBest.xy, lod).a, offsetBest.z);
	offsetBest += offsetVector *  step(textureLod(hiMap, offsetBest.xy, lod).a, offsetBest.z);
	offsetBest += offsetVector *  step(textureLod(hiMap, offsetBest.xy, lod).a, offsetBest.z);
	offsetBest += offsetVector *  step(textureLod(hiMap, offsetBest.xy, lod).a, offsetBest.z);
	offsetBest += offsetVector *  step(textureLod(hiMap, offsetBest.xy, lod).a, offsetBest.z);
	offsetBest += offsetVector *  step(textureLod(hiMap, offsetBest.xy, lod).a, offsetBest.z);
	offsetBest += offsetVector *  step(textureLod(hiMap, offsetBest.xy, lod).a, offsetBest.z);
	offsetBest += offsetVector *  step(textureLod(hiMap, offsetBest.xy, lod).a, offsetBest.z);
	offsetBest += offsetVector *  step(textureLod(hiMap, offsetBest.xy, lod).a, offsetBest.z);
	offsetBest += offsetVector * (step(textureLod(hiMap, offsetBest.xy, lod).a, offsetBest.z)          - 0.5);
	offsetBest += offsetVector * (step(textureLod(hiMap, offsetBest.xy, lod).a, offsetBest.z) * 0.5    - 0.25);
	offsetBest += offsetVector * (step(textureLod(hiMap, offsetBest.xy, lod).a, offsetBest.z) * 0.25   - 0.125);
	offsetBest += offsetVector * (step(textureLod(hiMap, offsetBest.xy, lod).a, offsetBest.z) * 0.125  - 0.0625);
	offsetBest += offsetVector * (step(textureLod(hiMap, offsetBest.xy, lod).a, offsetBest.z) * 0.0625 - 0.03125);
	
	return offsetBest.xy;  
	
}

 if(u_parallaxType == 0){
  // simple fastest parallax mapping
	float offset = textureLod( hiMap, texCoord.xy, lod ).a;
	offset = offset * 0.04 - 0.02;
	offsetBest.xy = offset * viewVec.xy + texCoord.xy;
	return offsetBest.xy;  
  }
}

float selfShadow(in sampler2D hiMap, in vec3 N, in vec3 L, in vec2 texCoord)
{
   vec2		delta;
   float	steps, height;
   float	NL = saturate(dot(N, L));
   
   if(NL <= 0.0)
		return 1.0;

    float lod = ComputeLOD(texCoord, u_parallaxParams.zw);
    
	if(lod > 8.0)
		return 1.0;

	float tex = textureLod(hiMap, texCoord, lod).a;
    
	if(tex >= 0.9)
		return 1.0;

    vec2 offsetCoord = texCoord;
    float numShadowSteps = mix(60.0, 5.0, -L.z);
    steps = 1.0 / numShadowSteps;
    delta = L.xy / L.z * u_shadowOffset / numShadowSteps;
    height = tex + steps * 0.1;

      while ((tex < height) && (height < 1.0)) {
                height += steps;
                offsetCoord += delta;
                tex = textureLod(hiMap, offsetCoord, lod).a;
            }

    return clamp( (height - tex ) * 8.0, 0.0, 1.0 );
}

#define PARALLAX_STEPS 15

vec2 ParallaxOcclusionMap(in sampler2D hiMap, const in vec2 texCoord, const in vec3 viewVec )
{
 float steps =  1.0 / PARALLAX_STEPS;
 float lod = ComputeLOD( texCoord, u_parallaxParams.zw );

 vec2 delta = vec2( viewVec.x, viewVec.y ) * u_parallaxParams.xy / (-viewVec.z * PARALLAX_STEPS);     
 float bias = -1.0;
 vec2 baseTC = texCoord;
 baseTC -= (1.0 - bias) * PARALLAX_STEPS * delta;
   
 float NB0 = textureLod( hiMap, baseTC, lod ).a;
 float height = 1.0 - steps;
 vec2 offset = texCoord + delta;
 
 float NB1 = textureLod( hiMap, offset, lod ).a;

 for( int i = 0; i < int( PARALLAX_STEPS ); i++ )
 {
  if( NB1 >= height )
   break;

  NB0 = NB1;
  height -= steps;
  offset += delta;
  NB1 = textureLod( hiMap, offset, lod ).a;
 }

 vec2 offsetBest = offset;
 float error = 1.0;
 float t1 = height;
 float t0 = t1 + steps;
 float delta1 = t1 - NB1;
 float delta0 = t0 - NB0;

 vec4 intersect = vec4( delta * PARALLAX_STEPS, delta * PARALLAX_STEPS + texCoord );

 float t = 0;

 for( int i = 0; i < 10; i++ )
 {
  if( abs( error ) <= 0.01 )
   break;

  float denom = delta1 - delta0;
  t = (t0 * delta1 - t1 * delta0) / denom;
  offsetBest = -t * intersect.xy + intersect.zw;

  float NB = textureLod( hiMap, offsetBest, lod ).a;

  error = t - NB;
  if( error < 0.0 )
  {
   delta1 = error;
   t1 = t;
  }
  else
  {
   delta0 = error;
   t0 = t;
  }
 }

 return offsetBest;
}