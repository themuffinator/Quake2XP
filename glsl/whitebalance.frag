layout (bindless_sampler, location = U_TMU0) uniform sampler2DRect	u_ScreenTex;

layout(location = U_PARAM_FLOAT_0)	uniform float	u_colorTempK;

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
vec3 colorTempRGB;
	
if(u_colorTempK > 999.0)
	colorTempRGB = ColorTemperatureToRGB(u_colorTempK);
else
	colorTempRGB = vec3(1.0);

	rawColor *= colorTempRGB;
	fragData = vec4(rawColor, 1.0);
}
