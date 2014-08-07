uniform sampler2DRect u_fboTex;
uniform vec4 u_control; // x - brightens, y - contrast, z - saturation, w - gamma

float A = 0.15;
float B = 0.50;
float C = 0.10;
float D = 0.20;
float E = 0.02;
float F = 0.30;
float W = 11.2;
float exposurebias = 2.0;
float multiplier = 3.0;
float power = 0.454;

vec3 BrightnesContrastSaturation(vec3 color, float brt, float con, float sat)
{
  // Increase or decrease theese values to adjust r, g and b color channels seperately
  const float AvgLumR = 0.5;
  const float AvgLumG = 0.5;
  const float AvgLumB = 0.5;
  
  const vec3 LumCoeff = vec3(0.2125, 0.7154, 0.0721);
  
	vec3 AvgLumin = vec3(AvgLumR, AvgLumG, AvgLumB);
	vec3 brtColor = color * brt;
	vec3 intensity = vec3(dot(brtColor, LumCoeff));
	vec3 satColor = mix(intensity, brtColor, sat);
	vec3 conColor = mix(AvgLumin, satColor, con);
	return conColor;
}

vec3 Tonemap(vec3 x)
{
return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec3 performTonemap(vec3 tmp)
{
vec3 tmpmap;
vec3 cur;
vec3 newcur;
vec3 whitescale;

tmpmap = multiplier * tmp;
cur = Tonemap(exposurebias*tmpmap);
whitescale = 1.0/Tonemap(vec3(W, W, W));
newcur = cur*whitescale;
return vec3(pow(newcur.r, power), pow(newcur.g, power), pow(newcur.b, power));
}

void main(void) 
{
 vec3 color = texture2DRect(u_fboTex, gl_FragCoord.xy).rgb;
 color = performTonemap(color.rgb);

 color = BrightnesContrastSaturation(color, u_control.x, u_control.y, u_control.z);
 color = pow(color, vec3(u_control.w));
 gl_FragColor.rgb = color;
 gl_FragColor.a = 1.0;
}
