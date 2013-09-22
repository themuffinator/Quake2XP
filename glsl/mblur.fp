uniform sampler2DRect 	u_ScreenTex;
uniform sampler2DRect 	u_DepthTex;
uniform vec3			u_viewOrg;
uniform vec2 			u_screenSize;
uniform vec2			u_depthParms;
uniform mat4 			u_InverseModelViewMat;
uniform mat4 			u_PrevModelViewProj;
varying vec3			v_viewRay; 

#include depth.inc

void main(void) 
{
	// tex coord conversion 
	vec2 fc = gl_FragCoord.xy / u_screenSize;

 // get current world space position:
   //   vec4 current = vec4(v_viewRay, 1.0) * texture(u_DepthTex, gl_FragCoord.xy).r;
    //  current = u_InverseModelViewMat /*gl_ModelViewMatrixInverse*/ * current;
 	float depth = DecodeDepth(texture(u_DepthTex, gl_FragCoord.xy).x, u_depthParms);
	vec3 position = u_viewOrg + v_viewRay * depth;
	
//	vec4 current = u_InverseModelViewMat /*gl_ModelViewMatrixInverse*/ * vec4(position, 0.0);

   // get previous screen space position:
      vec4 previous = u_PrevModelViewProj * vec4(position, 0.0)/*current*/;
      previous.xyz /= previous.w;
      previous.xy = previous.xy * 0.5 + 0.5;

      vec2 blurVec = previous.xy - fc;
	  // perform blur:
	int nSamples = 16;
	
  vec4 result = texture(u_ScreenTex, gl_FragCoord.xy);
	for (int i = 1; i < nSamples; ++i) {
	// get offset in range [-0.5, 0.5]:
    vec2 offset = blurVec * (float(i) / float(nSamples - 1) - 0.5);
  
	// sample & add to result:
   result += texture(u_ScreenTex, (fc + offset) * u_screenSize);
   }
 
   result /= float(nSamples);

   gl_FragColor = result;
}
