uniform sampler2DRect u_depthMap;
uniform sampler2DRect u_screenMap;
uniform mat4 PrevMatrix;

void main(void)
{
vec4 zOverW = texture2DRect(u_depthMap, gl_FragCoord.xy);

// H is the viewport position at this pixel in the range -1 to 1.
vec4 H = vec4(gl_FragCoord.x * 2 - 1, (1 - gl_FragCoord.y) * 2 - 1, zOverW.g, 1);
// Transform by the view-projection inverse.
vec4 D = gl_ModelViewProjectionMatrixInverse * H;
// Divide by w to get the world position.
vec4 worldPos = D / vec4(D.w);

// Current viewport position
vec4 currentPos = H;
// Use the world position, and transform by the previous view-projection matrix.
vec4 previousPos = PrevMatrix * worldPos;
// Convert to nonhomogeneous points [-1,1] by dividing by w.
previousPos = previousPos / vec4(previousPos.w);
// Use this frame's position and last frame's to compute the pixel velocity.
vec2 velocity = vec2(currentPos.xy - previousPos.xy)/2.0;

   // Get the initial color at this pixel.  
   vec4 color = texture2DRect(u_screenMap, gl_FragCoord.xy);  
	
	for(int i = 1; i < 8; ++i)  
		{  
		// Sample the color buffer along the velocity vector.  
		vec4 currentColor = texture2DRect(u_screenMap, gl_FragCoord.xy + vec2(velocity));  
		// Add the current color to our color sum.  
		color += currentColor;  
	}  
	// Average all of the samples to get the final blur color.  
	gl_FragColor = color / 8; 
}
