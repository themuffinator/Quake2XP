//!#include "global.inc"
layout (location = U_PARALLAX_PARAMS)	uniform vec4			u_parallaxParams; // x = scale[0] y = scale[1] z = tex_w[2] w = tex_h
layout (location = U_PARALLAX_TYPE)		uniform int				u_parallaxType;
layout (location = U_PARAM_FLOAT_0)		uniform float			u_shadowOffset;
layout (location = U_PARAM_FLOAT_1)		uniform float			u_shadowBlur;


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

float selfShadow(in sampler2D hiMap, in vec3 L, in vec2 texCoord){

    float lod = ComputeLOD(texCoord, u_parallaxParams.zw);
    
	if(lod > 8.0)
		return 1.0;

    vec2 offs = L.xy * u_parallaxParams.xy * u_shadowOffset;
	float blur = u_shadowBlur;

    float base =  textureLod(hiMap, texCoord, lod).a;

    float sh0  = ( textureLod(hiMap, texCoord + offs * 0.88, lod).a - base - 0.88 ) * 0.5 * blur;
    float sh1  = ( textureLod(hiMap, texCoord + offs * 0.77, lod).a - base - 0.77 ) * 1.0 * blur;
    float sh2  = ( textureLod(hiMap, texCoord + offs * 0.66, lod).a - base - 0.66 ) * 1.5 * blur;
    float sh3  = ( textureLod(hiMap, texCoord + offs * 0.55, lod).a - base - 0.55 ) * 2.0 * blur;
    float sh4  = ( textureLod(hiMap, texCoord + offs * 0.44, lod).a - base - 0.44 ) * 2.5 * blur;
    float sh5  = ( textureLod(hiMap, texCoord + offs * 0.33, lod).a - base - 0.33 ) * 3.0 * blur;
    float sh6  = ( textureLod(hiMap, texCoord + offs * 0.22, lod).a - base - 0.22 ) * 3.5 * blur;
	float sh7  = ( textureLod(hiMap, texCoord + offs * 0.11, lod).a - base - 0.11 ) * 4.0 * blur;

    return saturate( 1.0 - max(max(max(max(max(max( max( sh0, sh1 ), sh2 ), sh3 ), sh4 ), sh5 ), sh6 ), sh7 ));
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