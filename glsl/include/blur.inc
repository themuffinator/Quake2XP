
vec4 direction = vec4(1.0, 0.0, 0.0, 1.0);
     
vec3 boxBlur(sampler2DRect blurTex, vec2 tc, float  blurSamples){

	float numSamples = (1.0 / (blurSamples * 4.0 + 1.0));
	vec3 sum = texture( blurTex, tc).rgb;	// central point

	for ( float i = 1.0; i <= blurSamples; i += 1.0 ){

		sum += texture(blurTex, tc + vec2(i, 0.0)).rgb;
		sum += texture(blurTex, tc + vec2(-i, 0.0)).rgb;
		sum += texture(blurTex, tc + vec2(0.0, i)).rgb;
		sum += texture(blurTex, tc + vec2(0.0, -i)).rgb;
	}
	return sum * numSamples;
}

vec4 boxBlur2(sampler2DRect blurTex, float  blurSamples, vec2 N){

	float numSamples = (1.0 / (blurSamples * 4.0 + 1.0));
	vec4 sum = texture( blurTex, gl_FragCoord.xy + N);	// central point

	for ( float i = 1.0; i <= blurSamples; i += 1.0 ){

		sum += texture(blurTex, gl_FragCoord.xy + N + vec2(i, 0.0));
		sum += texture(blurTex, gl_FragCoord.xy + N + vec2(-i, 0.0));
		sum += texture(blurTex, gl_FragCoord.xy + N + vec2(0.0, i));
		sum += texture(blurTex, gl_FragCoord.xy + N + vec2(0.0, -i));
	}
	return sum * numSamples;
}