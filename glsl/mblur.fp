uniform sampler2DRect u_depthMap;
uniform sampler2DRect u_screenMap;
uniform mat4 PrevMatrix;
uniform mat4 invMatrix;
uniform vec2 u_screenSize;

#extension GL_ARB_texture_rectangle : enable

void main()
{
	vec4 tmp1,tmp2; 
	vec2 UV = gl_FragCoord.xy;

	//Retrieve depth of pixel  
	float z = texture2DRect(u_depthMap, UV).z;  
	
	//Simplified equation of GluUnproject
	vec4 currentPos = vec4( 2.0* (gl_FragCoord.x/u_screenSize.y)  - 1.0, 2.0* (gl_FragCoord.y/u_screenSize.x) - 1.0, 2.0*z -1.0 , 1.0);

	//Back into the worldSpace 
	tmp1 =  currentPos  * invMatrix;  
	
	//Homogenous value 
	vec4 posInWorldSpace = tmp1/tmp1.w;  
	
	//Using the world coordinate, we transform those into the previous frame
	tmp2 =  PrevMatrix * posInWorldSpace;  
	vec4 previousPos = tmp2/tmp2.w;  
	
	//Compute the frame velocity using the difference 
	vec2 velocity = ((currentPos - previousPos)/10.0).xy;

	//Get the initial color at this pixel.  
	vec4 originalColor = texture2DRect(u_screenMap, UV);
	UV += velocity.xy;  
	for(int i = 1; i < 20.0; ++i)  
	{  
		//Sample the color buffer along the velocity vector.  
		vec4 currentColor = texture2DRect(u_screenMap, UV);  
		//Add the current color to our color sum.  
		originalColor += currentColor;  
		UV.x += velocity.x;
		UV.y += velocity.y;
	}  
	//Average all of the samples to get the final blur color.  
	gl_FragColor = originalColor / 20.0;  
	
}


