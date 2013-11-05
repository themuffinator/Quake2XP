uniform sampler2DRect 	u_ScreenTex;
uniform sampler2DRect 	u_DepthTex;
uniform mat4 			u_InverseModelViewMat;
uniform mat4 			u_PrevModelViewProj;

void main(void) 
{
	float depth = texture2DRect(u_DepthTex, gl_FragCoord.xy).r;
	vec4 v = u_InverseModelViewMat * vec4(gl_FragCoord.xy, depth, 1.0);
	vec3 worldPos = v.xyz / v.w;

	  // Current viewport position  
	vec4 currentPos = v;  
	// Use the world position, and transform by the previous view-  
	// projection matrix.  
	vec4 previousPos = vec4(worldPos, 1.0) * u_PrevModelViewProj;  
	// Convert to nonhomogeneous points [-1,1] by dividing by w.  
	previousPos /= previousPos.w;  
	// Use this frame's position and last frame's to compute the pixel  
	// velocity.  
	vec2 velocity = (currentPos - previousPos)/3000.0;  

	// Get the initial color at this pixel.  
	//vec4 color = vec4(0.0, 0.0, 0.0, 0.0); 
   
	//for(int i = 0; i < 16; ++i)  
 //     {  
 //     // Sample the color buffer along the velocity vector.  
 //     vec4 currentColor = texture2DRect(u_ScreenTex, gl_FragCoord.xy + vec2(velocity)*i);  
 //     // Add the current color to our color sum.  
 //     color += currentColor;  
	//}  

	vec4 color = texture2DRect(u_ScreenTex, gl_FragCoord.xy);  
	vec2 texCoord = gl_FragCoord.xy;
	
	texCoord += velocity;  
	
	for(int i = 1; i < 32; ++i, texCoord += velocity)  
		{  
		// Sample the color buffer along the velocity vector.  
		vec4 currentColor = texture2DRect(u_ScreenTex, texCoord);  
		// Add the current color to our color sum.  
		color += currentColor;  
		}  
   // Average all of the samples to get the final blur color.  
   gl_FragColor = color / 32.0;  
}
