layout (binding = 0) uniform sampler2D		 	u_frameTex;
layout (binding = 1) uniform sampler2DRect	 	u_DepthTex;

uniform vec3 u_params; // x- near, y- far, numSamples 
in vec3 viewRay;
in vec2 texCoord;
uniform mat4 u_prevMVP, u_inverseMV;

#include depth.inc

void main(void) 
{
	// get current world space position:
    vec3 current = viewRay * DecodeDepth(texture2DRect(u_DepthTex, gl_FragCoord.xy).x, u_params.xy);
	mat4 currentToPrevious = u_prevMVP * u_inverseMV;	
	vec4 previous = currentToPrevious * vec4(current, 1.0);
	previous.xyz /= previous.w;
	previous.xy = previous.xy * 0.5 + 0.5;

    vec2 blurVec = previous.xy - texCoord;

	// perform blur:
	vec4 result = texture(u_frameTex, texCoord);
	for (int i = 1; i < u_params.z; i++) {
		// get offset in range [-0.5, 0.5]:
		vec2 offset = blurVec * (float(i) / float(u_params.z - 1) - 0.5);
  
		// sample & add to result:
		result += texture(u_frameTex, texCoord + offset);
   }
    
	fragData = result / float(u_params.z);
}
