//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0)  uniform sampler2D	u_map;
layout (bindless_sampler, location  = U_TMU1)  uniform sampler2D	u_normalMap;

layout (location = U_PARAM_INT_0)	uniform int		u_flags;
layout (location = U_PARAM_INT_1)	uniform int		u_hdrOutput;
layout (location = U_PARAM_VEC4_0)	uniform vec4	u_light;
layout (location = U_PARAM_FLOAT_0) uniform float	u_lod;
layout (location = U_PARAM_FLOAT_1) uniform float	u_colorFade;

in vec2		v_texCoord;
in vec4		v_color;

#include lighting.inc //!#include "include/lighting.inc"

const uint	PF_SCANLINE		= 1;
const uint	PF_LIGHT		= 2;
const uint	PF_VIGNETTE		= 4;
const uint	PF_CROSSHAIR	= 8;
const uint	PF_SRGBGAMMA	= 16;
const uint	PF_LOADSCREEN	= 32;
const uint	PF_VERTEXCOLOR	= 64;
const uint	PF_COLOREDFONT	= 128;
const uint	PF_MEDIANFILTER	= 256;
const uint	PF_TECHCOLOR	= 512;
const uint	PF_SKIPAMBIENT  = 1024;
const uint	PF_NOALPHA		= 2048;

vec4 Desaturate(vec3 color, float Desaturation){
	vec3 lum = vec3(0.2125, 0.7154, 0.0721);
	vec3 gray = vec3(dot(lum, color));
	return vec4(mix(color, gray, Desaturation), 1.0);
}

/*
3x3 Median
Morgan McGuire and Kyle Whitson
*/

#define s2(a, b)			temp = a; a = min(a, b); b = max(temp, b);
#define mn3(a, b, c)	s2(a, b); s2(a, c);
#define mx3(a, b, c)	s2(b, c); s2(a, c);

#define mnmx3(a, b, c)			    mx3(a, b, c); s2(a, b);									// 3 exchanges
#define mnmx4(a, b, c, d)		    s2(a, b); s2(c, d); s2(a, c); s2(b, d);					// 4 exchanges
#define mnmx5(a, b, c, d, e)	  s2(a, b); s2(c, d); mn3(a, c, e); mx3(b, d, e);			// 6 exchanges
#define mnmx6(a, b, c, d, e, f) s2(a, d); s2(b, e); s2(c, f); mn3(a, b, c); mx3(d, e, f);	// 7 exchanges


//  3x3 median too large for 256*256 screen texture. Scale median radius.
#define ONE_DIV_256		1.0/256.0
#define ONE_DIV_512		1.0/512.0
#define ONE_DIV_768		1.0/768.0
#define ONE_DIV_1024	1.0/1024.0

vec3 MedianFilter(in sampler2D tex) {

  vec3 v[9];
  vec2 screenOffs = vec2(ONE_DIV_512);
   
  // Add the pixels which make up our window to the pixel array.
  for(int dX = -1; dX <= 1; ++dX) {
    for(int dY = -1; dY <= 1; ++dY) {		
      vec2 offset = vec2(float(dX), float(dY));
		    
      // If a pixel in the window is located at (x+dX, y+dY), put it at index (dX + R)(2R + 1) + (dY + R) of the
      // pixel array. This will fill the pixel array, with the top left pixel of the window at pixel[0] and the
      // bottom right pixel of the window at pixel[N-1].
      v[(dX + 1) * 3 + (dY + 1)] = textureLod(tex, v_texCoord.xy + offset * screenOffs, 0.0).rgb;
    }
  }         
  
  vec3 temp;
  // Starting with a subset of size 6, remove the min and max each time
  mnmx6(v[0], v[1], v[2], v[3], v[4], v[5]);
  mnmx5(v[1], v[2], v[3], v[4], v[6]);
  mnmx4(v[2], v[3], v[4], v[7]);
  mnmx3(v[3], v[4], v[8]);
  
  return v[4];
}

#define REDFILTER 		vec4(1.0, 0.0, 0.0, 0.0)
#define BLUEGREENFILTER vec4(0.0, 1.0, 0.7, 0.0)

#define GREENFILTER 	vec4(0.0, 1.0, 0.0, 0.0)
#define BLUEFILTER		vec4(0.0, 0.0, 1.0, 0.0)

#define REDORANGEFILTER vec4(0.99, 0.263, 0.0, 0.0)

#define CYANFILTER		vec4(0.0, 1.0, 1.0, 0.0)
#define MAGENTAFILTER	vec4(1.0, 0.0, 1.0, 0.0)
#define YELLOWFILTER 	vec4(1.0, 1.0, 0.0, 0.0)

vec4 TechniColor(in vec4 color){
	
	vec4 redrecord = color * REDFILTER;
	vec4 bluegreenrecord = color * BLUEGREENFILTER;
	
	vec4 rednegative = vec4(redrecord.r);
	vec4 bluegreennegative = vec4((bluegreenrecord.g + bluegreenrecord.b) * 0.5);

	vec4 redoutput = rednegative * REDFILTER;
	vec4 bluegreenoutput = bluegreennegative * BLUEGREENFILTER;

	vec4 result = redoutput + bluegreenoutput;

	return mix(color, result, 0.44);
}

void main(void){

	if(bool(u_flags & PF_VERTEXCOLOR)){
		fragData = v_color;
		return;
	}

	vec4 image;

	if(bool(u_flags & PF_LOADSCREEN)){
		image = textureLod(u_map, v_texCoord.xy, u_lod);
	}
	else if(bool(u_flags & PF_MEDIANFILTER))
			image = vec4(MedianFilter(u_map), 1.0);
		else
			image = textureLod(u_map, v_texCoord.xy, 0.0);
    
	if(u_hdrOutput == 1)
		image = pow(image, vec4(2.2));

	if((u_flags & PF_TECHCOLOR) == PF_TECHCOLOR)
		image = TechniColor(image);

	if((u_flags & PF_SRGBGAMMA) == PF_SRGBGAMMA)
		image = pow(image, vec4(1.0/2.2));

	if(bool(u_flags & (PF_COLOREDFONT | PF_CROSSHAIR))){
		
		if((u_flags & PF_CROSSHAIR) == PF_CROSSHAIR){ // scale for hdr device output
			if(u_hdrOutput == 1)
				image *= 14.0;			
			}
		fragData = image * v_color;
		return;
	}

	if((u_flags & PF_LIGHT) == PF_LIGHT){
		vec3	normal = normalize(texture(u_normalMap, v_texCoord).rgb * 2.0 - 1.0);
		float	specular = texture(u_normalMap, v_texCoord).a;	

		vec3 L = normalize(vec3(u_light.x, u_light.y, u_light.z));
		vec3 V  = normalize(vec3(u_light.x, u_light.y, 1.0));
  
		vec2 Es = PhongLighting (normal.xyz, L, V, 16.0);
		vec3 lighting = vec3(image.rgb * Es.x + specular * Es.y);
		
		if((u_flags & PF_SKIPAMBIENT) == PF_SKIPAMBIENT)
			image.rgb = lighting * u_light.w;
		else
			image.rgb = image.rgb * 0.5 + lighting;
	}

	if((u_flags & PF_LOADSCREEN) == PF_LOADSCREEN){
		vec4 mono = Desaturate(image.rgb, 1.0); 
		vec4 deltas = image - mono;
		image = mono + u_colorFade * deltas;
		image.a = 1.0;
	}

	if((u_flags & PF_SCANLINE)== PF_SCANLINE){
		image.rgb -= mod(gl_FragCoord.y, 3.0) < 1.0 ? 0.1 : 0.0;
	}

	if((u_flags & PF_VIGNETTE)== PF_VIGNETTE){
		float OuterVignetting	= 1.4 - 0.45;
		float InnerVignetting	= 1.0 - 0.45;

		float d = distance(vec2(0.5, 0.5), v_texCoord.xy) * 1.414213;
		float vignetting = clamp((OuterVignetting - d) / (OuterVignetting - InnerVignetting), 0.0, 1.0);
		image.rgb *= vignetting;
	}
	fragData = image;

	if((u_flags & PF_NOALPHA) == PF_NOALPHA)
		fragData.a = 1.0;
}
