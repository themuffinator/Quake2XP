/*
from http://http.developer.nvidia.com/GPUGems3/gpugems3_ch27.html
*/

uniform sampler2DRect u_depthMap;
uniform sampler2DRect u_screenMap;
uniform mat4 PrevMatrix;
uniform mat4 invMatrix;

void main(void)
{
vec4 zOverW = texture2DRect(u_depthMap, gl_FragCoord.xy);
// H is the viewport position at this pixel in the range -1 to 1.
vec4 H = vec4(gl_FragCoord.x * 2 - 1, (1 - gl_FragCoord.y) * 2 - 1, zOverW.g, 1);
// Transform by the view-projection inverse.
vec4 D = invMatrix * H;
// Divide by w to get the world position.
vec4 worldPos = D / vec4(D.w);

// Current viewport position
vec4 currentPos = H;
// Use the world position, and transform by the previous view-projection matrix.
vec4 previousPos = PrevMatrix * worldPos;
// Convert to nonhomogeneous points [-1,1] by dividing by w.
previousPos = previousPos / vec4(previousPos.w);
// Use this frame's position and last frame's to compute the pixel velocity.
vec2 velocity = vec2(currentPos.xy - previousPos.xy)/4000.0;

// vec2 velocity = vec2(1.5, -0.5); //debug vector

   // Get the initial color at this pixel.  
   vec4 color = vec4(0.0, 0.0, 0.0, 0.0); 
	
	for(int i = 1; i < 8; ++i)  
		{  
		// Sample the color buffer along the velocity vector.  
		vec4 currentColor = texture2DRect(u_screenMap, gl_FragCoord.xy + vec2(velocity)*i);  
		// Add the current color to our color sum.  
		color += currentColor;  
	}  
	// Average all of the samples to get the final blur color.  
	gl_FragColor = color / 8; 
}
