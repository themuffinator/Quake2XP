//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0) uniform sampler2DRect u_screenTex;

const vec3 colorMap[16] = vec3[](
	  vec3( 0.0, 0.0, 0.0 ), // black
	  vec3( 0.0, 0.0, 0.1647 ),    // darkest blue
	  vec3( 0.0, 0.0, 0.3647 ),    // darker blue
	  vec3( 0.0, 0.0, 0.6647 ),    // dark blue
	  vec3( 0.0, 0.0, 0.9647 ),    // blue
	  vec3( 0.0, 0.9255, 0.9255 ), // cyan
	  vec3( 0.0, 0.5647, 0.0 ),    // dark green
	  vec3( 0.0, 0.7843, 0.0 ),    // green
	  vec3( 1.0, 1.0, 0.0 ),       // yellow
	  vec3( 0.90588, 0.75294, 0.0 ), // yellow-orange
	  vec3( 1.0, 0.5647, 0.0 ),    // orange
	  vec3( 1.0, 0.0, 0.0 ),       // bright red
	  vec3( 0.8392, 0.0, 0.0 ),    // red
	  vec3( 1.0, 0.0, 1.0 ),       // magenta
	  vec3( 0.6, 0.3333, 0.7882 ), // purple
	  vec3( 1.0, 1.0, 1.0 )        // white
	  );

void main(){

vec4 color = texture(u_screenTex, gl_FragCoord.xy);
float lum = dot(vec3(0.2125, 0.7154, 0.0721), color.rgb);
float v = log2( lum / 0.1 );
v = clamp( v + 5.0, 0.0, 15.0 );
int index = int( ceil( v ) );
fragData.rgb = mix( colorMap[index],colorMap[ min( 15, index + 1 ) ], fract( v ) );
fragData.a = 1.0;
}
