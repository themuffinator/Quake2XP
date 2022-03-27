//!#include "include/global.inc"
layout(triangles) in;
layout(line_strip, max_vertices=6) out;

layout(location = U_PARAM_FLOAT_0)  uniform float   len;
layout(location = U_MVP_MATRIX)     uniform mat4    u_modelViewProjectionMatrix;

in vertex{
  vec3 tangent;
  vec3 biNormal;
  vec3 normal;
} v[];

out vec4 vColor;

void main(){

  for(int i = 0; i < gl_in.length(); i++){

    vec3 P = gl_in[i].gl_Position.xyz;
    vec3 T = v[i].tangent;   
    vec3 B = v[i].biNormal;
    vec3 N = v[i].normal;

    gl_Position = u_modelViewProjectionMatrix * vec4(P, 1.0);
    vColor = vec4(1.0, 0.0, 0.0, 1.0);
    EmitVertex();    
    gl_Position = u_modelViewProjectionMatrix * vec4(P + T * len, 1.0);
    EmitVertex();    
    EndPrimitive();
 
    gl_Position = u_modelViewProjectionMatrix * vec4(P, 1.0);
    vColor = vec4(0.0, 1.0, 0.0, 1.0);
    EmitVertex();    
    gl_Position = u_modelViewProjectionMatrix * vec4(P + B * len, 1.0);
    EmitVertex();        
    EndPrimitive();

    gl_Position = u_modelViewProjectionMatrix * vec4(P, 1.0);
    vColor = vec4(0.0, 0.0, 1.0, 1.0);
    EmitVertex();    
    gl_Position = u_modelViewProjectionMatrix * vec4(P + N * len, 1.0);
    EmitVertex();
    EndPrimitive();
  }
}