layout (bindless_sampler, location = U_TMU0) uniform sampler2DRect	u_ScreenTex;
layout (bindless_sampler, location = U_TMU1) uniform sampler3D		u_lutTex;

layout(location = U_PARAM_VEC3_0)	uniform vec3	u_lutSize;
layout(location = U_PARAM_FLOAT_1)	uniform float	u_colorTempK;
layout(location = U_PARAM_INT_0)	uniform	int		u_colorK;

#ifndef saturate
#define saturate(x)    clamp(x, 0.0, 1.0)
#endif

vec3 ColorTemperatureToRGB(float temperatureInKelvins)
{
	vec3 retColor;
	
    temperatureInKelvins = clamp(temperatureInKelvins, 1000.0, 40000.0) / 100.0;
    
    if (temperatureInKelvins <= 66.0)
    {
        retColor.r = 1.0;
        retColor.g = saturate(0.39008157876901960784 * log(temperatureInKelvins) - 0.63184144378862745098);
    }
    else
    {
    	float t = temperatureInKelvins - 60.0;
        retColor.r = saturate(1.29293618606274509804 * pow(t, -0.1332047592));
        retColor.g = saturate(1.12989086089529411765 * pow(t, -0.0755148492));
    }
    
    if (temperatureInKelvins >= 66.0)
        retColor.b = 1.0;
    else if(temperatureInKelvins <= 19.0)
        retColor.b = 0.0;
    else
        retColor.b = saturate(0.54320678911019607843 * log(temperatureInKelvins - 10.0) - 1.19625408914);

    return retColor;
}

void main(void){

vec3 rawColor = texture2DRect(u_ScreenTex, gl_FragCoord.xy).rgb;

if(u_colorK == 1){

	vec3 colorTempRGB;
	
	if(u_colorTempK > 999.0)
		colorTempRGB = ColorTemperatureToRGB(u_colorTempK);
	else
		colorTempRGB = vec3(1.0);
	rawColor *= colorTempRGB;
	fragData = vec4(rawColor, 1.0);
	return;
}

if(u_colorK == 0){
	//Apply LUT Table 
	//developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter24.html
	vec3 scale = (u_lutSize - 1.0) / u_lutSize;
    vec3 offset = 1.0 / (2.0 * u_lutSize);

	fragData.rgb = texture(u_lutTex, scale * rawColor.rgb + offset).rgb;
  fragData.a = 1.0;

	}
}
