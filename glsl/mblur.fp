uniform sampler2DRect	 	u_ScreenTex;
uniform sampler2DRect		u_MaskTex;
uniform vec2				u_velocity;
uniform int					u_numSamples;

void main(void) 
{
	vec2 uv = gl_FragCoord.xy;
	//Get the initial color at this pixel.  
	vec4 color = texture2DRect(u_ScreenTex, uv);
	
	float mask = texture2DRect(u_MaskTex, gl_FragCoord.xy).a;
	if(mask == 0.0){
		gl_FragColor =  color;
		return;
	}
	vec2 velocity  = clamp(u_velocity, -1.0, 1.0);
	uv += velocity.xy;  
	for(int i = 1; i < u_numSamples; ++i)  
	{  
	//Sample the color buffer along the velocity vector.  
	vec4 accum = texture2DRect(u_ScreenTex, uv);  
	//Add the current color to our color sum.  
	color += accum;  
	uv += velocity;
	}
	//Average all of the samples to get the final blur color.  
	gl_FragColor =  color / u_numSamples; 
}
