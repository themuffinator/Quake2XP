layout (binding = 0) uniform sampler2DRect	 	u_ScreenTex;

uniform vec3 u_params; // x-velocity, y-velocity, numSamples 

void main(void) 
{
	//Get the initial color at this pixel.  
	vec4 color = vec4(0.0);
	vec2 uv = gl_FragCoord.xy;
	vec2 velocity = u_params.xy;
	uv += velocity.xy;

	for(int i = 1; i < u_params.z; ++i) {		
		//Sample the color buffer along the velocity vector.
		vec4 accum = texture2DRect(u_ScreenTex, uv);
		//Add the current color to our color sum.  
		color += accum;  
		uv += velocity;
	}

	//Average all of the samples to get the final blur color.  
	fragData =  color / u_params.z; 
}