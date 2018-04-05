layout (binding = 0) uniform sampler2D u_cinMap;

in vec2			v_texCoord;

/*
3x3 Median
Morgan McGuire and Kyle Whitson
*/

#define s2(a, b)			temp = a; a = min(a, b); b = max(temp, b);
#define mn3(a, b, c)	s2(a, b); s2(a, c);
#define mx3(a, b, c)	s2(b, c); s2(a, c);

#define mnmx3(a, b, c)			    mx3(a, b, c); s2(a, b);                                   // 3 exchanges
#define mnmx4(a, b, c, d)		    s2(a, b); s2(c, d); s2(a, c); s2(b, d);                   // 4 exchanges
#define mnmx5(a, b, c, d, e)	  s2(a, b); s2(c, d); mn3(a, c, e); mx3(b, d, e);           // 6 exchanges
#define mnmx6(a, b, c, d, e, f) s2(a, d); s2(b, e); s2(c, f); mn3(a, b, c); mx3(d, e, f); // 7 exchanges


//  3x3 median too large for 256*256 screen texture. Scale median radius.
#define ONE_DIV_256		1.0/256.0
#define ONE_DIV_512		1.0/512.0
#define ONE_DIV_768		1.0/768.0
#define ONE_DIV_1024	1.0/1024.0

vec3 median(in sampler2D tex) {

  vec3 v[9];

  vec2 screenOffs = vec2(ONE_DIV_768);
   
  // Add the pixels which make up our window to the pixel array.
  for(int dX = -1; dX <= 1; ++dX) {
    for(int dY = -1; dY <= 1; ++dY) {		
      vec2 offset = vec2(float(dX), float(dY));
		    
      // If a pixel in the window is located at (x+dX, y+dY), put it at index (dX + R)(2R + 1) + (dY + R) of the
      // pixel array. This will fill the pixel array, with the top left pixel of the window at pixel[0] and the
      // bottom right pixel of the window at pixel[N-1].
      v[(dX + 1) * 3 + (dY + 1)] = texture(tex, v_texCoord.xy + offset * screenOffs).rgb;
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

vec4 TechniColor(in vec4 color)
{
	
	vec4 redrecord = color * REDFILTER;
	vec4 bluegreenrecord = color * BLUEGREENFILTER;
	
	vec4 rednegative = vec4(redrecord.r);
	vec4 bluegreennegative = vec4((bluegreenrecord.g + bluegreenrecord.b) * 0.5);

	vec4 redoutput = rednegative * REDFILTER;
	vec4 bluegreenoutput = bluegreennegative * BLUEGREENFILTER;

	vec4 result = redoutput + bluegreenoutput;

	return mix(color, result, 0.44);
}

#define RGB_MASK_SIZE 3.0

void main ()
{
	vec4 cin = vec4(median(u_cinMap), 1.0) * 2.0;
	cin = clamp(cin, 0.05, 1.0);
	fragData = TechniColor(cin);

	// create rgb CRT mask
	float pix = gl_FragCoord.y * 1920.0 + gl_FragCoord.x;
    pix = floor(pix);
	vec4 rgbMask = vec4(mod(pix, RGB_MASK_SIZE), mod((pix + 1.0), RGB_MASK_SIZE), mod((pix + 2.0), RGB_MASK_SIZE), 1.0);
    rgbMask = rgbMask / (RGB_MASK_SIZE - 1.0) + 0.5;
	fragData *= rgbMask * 1.2;

//	fragData -= mod(gl_FragCoord.y, 3.0) < 1.0 ? 0.5 : 0.0;
}
