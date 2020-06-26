
vec4 direction = vec4(1.0, 0.0, 0.0, 1.0);
     
vec3 boxBlur(sampler2DRect blurTex, vec2 tc, float  blurSamples){

	float numSamples = (1.0 / (blurSamples * 4.0 + 1.0));
	vec3 sum = texture2DRect( blurTex, tc).rgb;	// central point

	for ( float i = 1.0; i <= blurSamples; i += 1.0 ){

		sum += texture2DRect(blurTex, tc + vec2(i, 0.0)).rgb;
		sum += texture2DRect(blurTex, tc + vec2(-i, 0.0)).rgb;
		sum += texture2DRect(blurTex, tc + vec2(0.0, i)).rgb;
		sum += texture2DRect(blurTex, tc + vec2(0.0, -i)).rgb;
	}
	return sum * numSamples;
}

vec4 boxBlur2(sampler2DRect blurTex, float  blurSamples, vec2 N){

	float numSamples = (1.0 / (blurSamples * 4.0 + 1.0));
	vec4 sum = texture2DRect( blurTex, gl_FragCoord.xy + N);	// central point

	for ( float i = 1.0; i <= blurSamples; i += 1.0 ){

		sum += texture2DRect(blurTex, gl_FragCoord.xy + N + vec2(i, 0.0));
		sum += texture2DRect(blurTex, gl_FragCoord.xy + N + vec2(-i, 0.0));
		sum += texture2DRect(blurTex, gl_FragCoord.xy + N + vec2(0.0, i));
		sum += texture2DRect(blurTex, gl_FragCoord.xy + N + vec2(0.0, -i));
	}
	return sum * numSamples;
}

vec4 blur13(sampler2DRect image, vec2 uv, vec2 resolution, vec2 direction) {
  vec4 color = vec4(0.0);
  vec2 off1 = vec2(1.411764705882353) * direction;
  vec2 off2 = vec2(3.2941176470588234) * direction;
  vec2 off3 = vec2(5.176470588235294) * direction;
  color += texture2DRect(image, uv) * 0.1964825501511404;
  color += texture2DRect(image, uv + (off1 / resolution)) * 0.2969069646728344;
  color += texture2DRect(image, uv - (off1 / resolution)) * 0.2969069646728344;
  color += texture2DRect(image, uv + (off2 / resolution)) * 0.09447039785044732;
  color += texture2DRect(image, uv - (off2 / resolution)) * 0.09447039785044732;
  color += texture2DRect(image, uv + (off3 / resolution)) * 0.010381362401148057;
  color += texture2DRect(image, uv - (off3 / resolution)) * 0.010381362401148057;
  return color;
}

vec4 blur9(sampler2DRect image, vec2 uv, vec2 resolution, vec2 direction) {
  vec4 color = vec4(0.0);
  vec2 off1 = vec2(1.3846153846) * direction;
  vec2 off2 = vec2(3.2307692308) * direction;
  color += texture2DRect(image, uv) * 0.2270270270;
  color += texture2DRect(image, uv + (off1 / resolution)) * 0.3162162162;
  color += texture2DRect(image, uv - (off1 / resolution)) * 0.3162162162;
  color += texture2DRect(image, uv + (off2 / resolution)) * 0.0702702703;
  color += texture2DRect(image, uv - (off2 / resolution)) * 0.0702702703;
  return color;
}

vec4 blur5(sampler2DRect image, vec2 uv, vec2 resolution, vec2 direction) {
  vec4 color = vec4(0.0);
  vec2 off1 = vec2(1.3333333333333333) * direction;
  color += texture2DRect(image, uv) * 0.29411764705882354;
  color += texture2DRect(image, uv + (off1 / resolution)) * 0.35294117647058826;
  color += texture2DRect(image, uv - (off1 / resolution)) * 0.35294117647058826;
  return color; 
}
