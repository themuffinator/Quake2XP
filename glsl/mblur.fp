uniform sampler2DRect	 	u_ScreenTex;
uniform vec2				u_velocity;

#include depth.inc

void main(void) 
{
	vec2 UV = gl_FragCoord.xy;
	//Get the initial color at this pixel.  
	vec4 originalColor = texture2DRect(u_ScreenTex, UV);
	vec2 velocity;
	velocity.x = clamp(u_velocity.x, -1.0, 1.0);
	velocity.y = clamp(u_velocity.y, -1.0, 1.0);
	UV += velocity.xy;  
	for(int i = 1; i < 30.0; ++i)  
	{  
		//Sample the color buffer along the velocity vector.  
		vec4 currentColor = texture2DRect(u_ScreenTex, UV);  
		//Add the current color to our color sum.  
		originalColor += currentColor;  
		UV.x += velocity.x;
		UV.y += velocity.y;
	}

	//Average all of the samples to get the final blur color.  
	gl_FragColor =  originalColor / 30.0; 
}
