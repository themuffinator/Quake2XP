layout (location = U_PARALLAX_PARAMS) uniform vec4			u_parallaxParams; // x = scale[0] y = scale[1] z = tex_w[2] w = tex_h
layout (location = U_PARALLAX_TYPE) uniform int				u_parallaxType;

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

float selfShadow(in sampler2D hiMap, in vec3 Normal, in vec3 L, in vec2 texCoord)
{
   vec2		delta;
   float	steps;
   float	height = 1.0;
   float	shadow = 0.0;
   float	reliefScale = 0.05;
   float	NL = saturate(dot(Normal, L));
   float	lod = ComputeLOD(texCoord, u_parallaxParams.zw);
	
   if(NL > 0.0)
   {
    vec2 offsetCoord = texCoord;
    float tex = textureLod(hiMap, texCoord, lod).a;

    float numShadowSteps = mix(60.0, 5.0, -L.z);
    steps = 1.0 / numShadowSteps;
    delta = vec2(-L.x, -L.y) * reliefScale / (numShadowSteps * -L.z);
    height = tex + steps * 0.1;
  
      while ((tex < height) && (height < 1.0)) {
                height += steps;
                offsetCoord += delta;
                tex = textureLod(hiMap, offsetCoord, lod).a;
            }

     shadow = float(tex < height);                         
   }   
   return shadow;
}