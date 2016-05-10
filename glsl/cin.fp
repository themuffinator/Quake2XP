in vec2 v_texCoord;
in vec2 v_lineOffset;

layout (binding = 0) uniform sampler2D u_cinMap;

const float brightness = 0.85;
const vec2 sine_comp = vec2(0.0005, 0.35);

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

vec3 median(in sampler2D tex) {

  vec3 v[9];
  /*
  3x3 median too large for 256*256 screen texture. Scale median radius.
  1.0/256.0 = 0,00390625
  1.0/512.0 = 0,001953125
  1.0/1024.0 = 0,0009765625
  */
  vec2 screenOffs = vec2(0.001953125);
   
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

void main ()
{
//	vec4 cin = texture(u_cinMap, v_texCoord.xy);
  vec4 cin = vec4(median(u_cinMap), 1.0);
	vec4 scanline = cin * (brightness + dot(sine_comp * sin(v_texCoord.xy * v_lineOffset), vec2(1.0)));
	fragData = clamp(scanline, 0.0, 1.0);
	fragData *= 1.5;
}
