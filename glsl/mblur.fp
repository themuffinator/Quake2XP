uniform sampler2DRect u_ScreenTex;
uniform sampler2DRect u_DepthTex;
uniform mat4 u_InverseModelViewMat; // inverse model->view
uniform mat4 u_PrevModelViewProj; // previous model->view->projection
uniform vec3 u_viewOrg; // for extracting current world space position
uniform vec2 u_depthParms;


#include depth.inc

void main(void) 
{
	// get current world space position:
	float depth = DecodeDepth(texture2DRect(u_DepthTex, gl_FragCoord.xy).x, u_depthParms);
	vec4 current = vec4(u_viewOrg, 1.0) * depth;
	current = u_InverseModelViewMat * current;
 
	// get previous screen space position:
	vec4 previous = u_PrevModelViewProj * current;
	previous.xyz /= previous.w;
	previous.xy = previous.xy * 0.5 + 0.5;

	vec2 blurVec = previous.xy - gl_FragCoord.xy;

	vec4 result = texture2DRect(u_ScreenTex, gl_FragCoord.xy);
   
	int nSamples = 16;

	for (int i = 1; i < nSamples; ++i) {
	// get offset in range [-0.5, 0.5]:
	vec2 offset = blurVec * (float(i) / float(16.0 - 1) - 0.5);
	// sample & add to result:
	result += texture2DRect(u_ScreenTex, gl_FragCoord.xy + offset);
   }
 
   result /= float(nSamples);
   gl_FragColor = result;
}
