layout (binding = 0)  uniform sampler2D u_map;

layout (location = U_COLOR_MUL) uniform float u_colorScale;
layout (location = U_SCREEN_SIZE)	uniform vec2	u_screenSize;
#define RGB_MASK_SIZE 3.0

in vec2 v_texCoord;

vec4 Desaturate(vec3 color, float Desaturation)
{
	vec3 lum = vec3(0.2125, 0.7154, 0.0721);
	vec3 gray = vec3(dot(lum, color));
	return vec4(mix(color, gray, Desaturation), 1.0);
}

void main(void) 
{
vec4 color =  texture(u_map, v_texCoord.xy);
vec4 mono = Desaturate(color.rgb, 1.0); 
vec4 deltas = color - mono;
fragData = mono + u_colorScale * deltas;

// create rgb CRT mask
float pix = gl_FragCoord.y * u_screenSize.x + gl_FragCoord.x;
pix = floor(pix);
vec4 rgbMask = vec4(mod(pix, RGB_MASK_SIZE), mod((pix + 1.0), RGB_MASK_SIZE), mod((pix + 2.0), RGB_MASK_SIZE), 1.0);
rgbMask = rgbMask / (RGB_MASK_SIZE - 1.0) + 0.5;
fragData *= rgbMask;
	
float OuterVignetting	= 1.4 - 0.45;
float InnerVignetting	= 1.0 - 0.45;

float d = distance(vec2(0.5, 0.5), v_texCoord.xy) * 1.414213;
float vignetting = clamp((OuterVignetting - d) / (OuterVignetting - InnerVignetting), 0.0, 1.0);
fragData *= vignetting;
fragData.a = 1.0;
}
