
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

#define pow2(x) (x * x)

const float pi = atan(1.0) * 4.0;
const int samples = 35;
const float sigma = float(samples) * 0.25;

float gaussian(vec2 i) {
    return 1.0 / (2.0 * pi * pow2(sigma)) * exp(-((pow2(i.x) + pow2(i.y)) / (2.0 * pow2(sigma))));
}

//	vec2 texelSize = vec2(1.0) / u_screenSize.xy;
//	vec2 uv = gl_FragCoord.xy / u_screenSize;
//	vec3 blurTex = blur(u_map3, uv, texelSize);
vec3 blur(sampler2D sp, vec2 uv, vec2 scale) {
    vec3 col = vec3(0.0);
    float accum = 0.0;
    float weight;
    vec2 offset;
    
    for (int x = -samples / 2; x < samples / 2; ++x) {
        for (int y = -samples / 2; y < samples / 2; ++y) {
            offset = vec2(x, y);
            weight = gaussian(offset);
            col += texture(sp, uv + scale * offset).rgb * weight;
            accum += weight;
        }
    }
    
    return col / accum;
}