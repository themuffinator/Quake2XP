//!#include "include/global.inc"
layout (bindless_sampler, location  = U_TMU0) uniform sampler2DRect u_screenTex;

const vec4 colors[5] = vec4[](vec4(0.0, 0.0, 1.0, 1.0), //blue
                              vec4(0.0, 1.0, 0.0, 1.0), //green
                              vec4(1.0, 0.0, 0.0, 1.0), //red
                              vec4(1.0, 1.0, 0.0, 1.0), //yellow
                              vec4(1.0, 1.0, 1.0, 1.0));//white
                              
vec3 unreal(vec3 x) {
  return x / (x + 0.155) * 1.019;
}
void main(){

vec4 color = texture(u_screenTex, gl_FragCoord.xy);
color.rgb = unreal(color.rgb); // fix out of range
float lum = dot(vec3(0.2125, 0.7154, 0.0721), color.rgb);

int ix = (lum < 0.5) ? 0 : 1;
fragData = mix(colors[ix], colors[ix+1], (lum-float(ix) * 0.5) * 2.0);
}
