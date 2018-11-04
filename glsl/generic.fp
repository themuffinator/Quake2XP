layout (binding = 0) uniform sampler2D	u_map;
layout (binding = 1) uniform sampler2D	u_normalMap;

layout(location = U_COLOR)			uniform vec4		u_color;
layout(location = U_COLOR_MUL)		uniform float		u_colorScale;
layout(location = U_PARAM_VEC3_0)	uniform vec3		u_lightShift;

layout(location = U_CONSOLE_BACK)	uniform int	    u_console;
layout(location = U_2D_PICS)		uniform int	    u_2dPics;
layout(location = U_FRAG_COLOR)		uniform int     u_fragColor;
layout (location = U_SCREEN_SIZE)	uniform vec2	u_screenSize;

in vec2		v_texCoord;
in vec4		v_color;


#include lighting.inc

#define RGB_MASK_SIZE 3.0
#define SMOOTHING 1.0 / 16.0

void main(void) 
{

vec4 diffuse = texture(u_map, v_texCoord.xy);

if(u_console == 1){
	
	vec3 normal = normalize(texture(u_normalMap, v_texCoord).rgb * 2.0 - 1.0);
	float specular = texture(u_normalMap, v_texCoord).a;	

	vec3 L = normalize(vec3(u_lightShift.x, u_lightShift.y, u_lightShift.z));
	vec3 V  = normalize(vec3(0.0, 0.0, 1.0));

	vec4 lighting = vec4(Lighting_BRDF(diffuse.rgb, vec3(specular), 0.4, normal.xyz, L, V), 1.0)  * vec4(1.0);

	fragData = diffuse * 0.25 + lighting;
    
    // create rgb CRT mask
	float pix = gl_FragCoord.y * u_screenSize.x + gl_FragCoord.x;
    pix = floor(pix);
	vec4 rgbMask = vec4(mod(pix, RGB_MASK_SIZE), mod((pix + 1.0), RGB_MASK_SIZE), mod((pix + 2.0), RGB_MASK_SIZE), 1.0);
    rgbMask = rgbMask / (RGB_MASK_SIZE - 1.0) + 0.5;
	fragData *= rgbMask * 1.2;

	fragData.a = 1.0;
	return;
}

if(u_2dPics == 1){

    // signed distance field
    float distance = diffuse.a;
    float alpha = smoothstep(0.5 - SMOOTHING, 0.5 + SMOOTHING, distance);
	fragData =  vec4(diffuse.rgb * v_color.rgb, diffuse.a * alpha);
	return;
}

if(u_fragColor == 1){
	fragData =  u_color;
	return;
	}
}
