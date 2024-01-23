//!#include "include/global.inc"
layout(location = 0) in vec4 att_position;
layout(location = 1) in vec3 att_normal;
layout(location = 2) in vec3 att_tangent;
layout(location = 3) in vec3 att_binormal;

out vertex{
  vec3 tangent;
  vec3 biNormal;
  vec3 normal;
}v;

void main(){

  v.tangent   = att_tangent;
  v.biNormal  = att_binormal;
  v.normal    = att_normal;
  
  gl_Position = vec4 (att_position.xyz, 1.0);
}