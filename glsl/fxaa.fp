/*=======================
FXAA II by TIMOTHY LOTTES 
NVIDIA CORP.
=======================*/

uniform sampler2DRect u_ScreenTex;

#define FXAA_REDUCE_MIN   (1.0/ 128.0)
#define FXAA_REDUCE_MUL   (1.0 / 8.0)
#define FXAA_SPAN_MAX     8.0

/*
FXAA_EDGE_THRESHOLD
The minimum amount of local contrast required to apply algorithm.
1/3 - too little
1/4 - low quality
1/8 - high quality
1/16 - overkill
FXAA_EDGE_THRESHOLD_MIN
Trims the algorithm from processing darks.
1/32 - visible limit
1/16 - high quality
1/12 - upper limit (start of visible unfiltered edges)
*/
#define FXAA_EDGE_THRESHOLD     	1/2
#define FXAA_EDGE_THRESHOLD_MIN     1/12

vec4 fxaaPixelShader(sampler2DRect ScreenTex)
{
    vec4 color;
    vec3 rgbNW = texture2DRect(ScreenTex, (gl_FragCoord.xy + vec2(-1.0, -1.0))).xyz;
    vec3 rgbNE = texture2DRect(ScreenTex, (gl_FragCoord.xy + vec2(1.0, -1.0))).xyz;
    vec3 rgbSW = texture2DRect(ScreenTex, (gl_FragCoord.xy + vec2(-1.0, 1.0))).xyz;
    vec3 rgbSE = texture2DRect(ScreenTex, (gl_FragCoord.xy + vec2(1.0, 1.0))).xyz;
    vec3 rgbM  = texture2DRect(ScreenTex, gl_FragCoord.xy).xyz;

    vec3 luma = vec3(0.299, 0.587, 0.114);
    
	float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

	float range = lumaMax - lumaMin;
	if(range < max(FXAA_EDGE_THRESHOLD_MIN, lumaMax * FXAA_EDGE_THRESHOLD))
		return vec4(rgbM, 1.0);
		//return vec4(1.0, 0.0, 0.0, 1.0);// debug!

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
    
    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) *
                          (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
    
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir =	min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), 
			max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcpDirMin));
      
    vec3 rgbA = 0.5 * (
        texture2DRect(ScreenTex, gl_FragCoord.xy + dir * (1.0 / 3.0 - 0.5)).xyz +
        texture2DRect(ScreenTex, gl_FragCoord.xy + dir * (2.0 / 3.0 - 0.5)).xyz);

    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture2DRect(ScreenTex, gl_FragCoord.xy + dir * -0.5).xyz +
        texture2DRect(ScreenTex, gl_FragCoord.xy + dir * 0.5).xyz);

    float lumaB = dot(rgbB, luma);
    if ((lumaB < lumaMin) || (lumaB > lumaMax))
        color = vec4(rgbA, 1.0);
    else
        color = vec4(rgbB, 1.0);
    return color;
}

void main(void)
{
    gl_FragColor = fxaaPixelShader(u_ScreenTex);
}