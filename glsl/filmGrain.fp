layout (binding = 0) uniform sampler2DRect u_ScreenTex;

uniform vec4	u_params;	// x- filter type: 1 - technicolor1; 2 - technicolor3; 3 - sepia
							// y- noise interns
							// z - scarch intens
							// w - vigent size
uniform vec2	u_screenSize; 
uniform float	u_rand;
uniform int		u_time;

#define REDFILTER 		vec4(1.0, 0.0, 0.0, 0.0)
#define BLUEGREENFILTER vec4(0.0, 1.0, 0.7, 0.0)

#define GREENFILTER 	vec4(0.0, 1.0, 0.0, 0.0)
#define BLUEFILTER		vec4(0.0, 0.0, 1.0, 0.0)

#define REDORANGEFILTER vec4(0.99, 0.263, 0.0, 0.0)

#define CYANFILTER		vec4(0.0, 1.0, 1.0, 0.0)
#define MAGENTAFILTER	vec4(1.0, 0.0, 1.0, 0.0)
#define YELLOWFILTER 	vec4(1.0, 1.0, 0.0, 0.0)


#define ONE_DIV_THREE 1.0 / 3.0

vec4 TechniColorSys1(in vec4 color)
{
	
	vec4 redrecord = color * REDFILTER;
	vec4 bluegreenrecord = color * BLUEGREENFILTER;
	
	vec4 rednegative = vec4(redrecord.r);
	vec4 bluegreennegative = vec4((bluegreenrecord.g + bluegreenrecord.b) * 0.5);

	vec4 redoutput = rednegative * REDFILTER;
	vec4 bluegreenoutput = bluegreennegative * BLUEGREENFILTER;

	vec4 result = redoutput + bluegreenoutput;

	return mix(color, result, 0.44);
}

// fuck off TechniColorSys2 - blue shit!

vec4 TechniColorSys3(in vec4 color)
{
	vec4 greenrecord = (color) * GREENFILTER;
	vec4 bluerecord = (color) * MAGENTAFILTER;
	vec4 redrecord = (color) * REDORANGEFILTER;
		
	vec4 rednegative = vec4((redrecord.r + redrecord.g + redrecord.b) * ONE_DIV_THREE);
	vec4 greennegative = vec4((greenrecord.r + greenrecord.g + greenrecord.b) * ONE_DIV_THREE);
	vec4 bluenegative = vec4((bluerecord.r+ bluerecord.g + bluerecord.b) * ONE_DIV_THREE);

	vec4 redoutput = rednegative + CYANFILTER;
	vec4 greenoutput = greennegative + MAGENTAFILTER;
	vec4 blueoutput = bluenegative + YELLOWFILTER;

	vec4 result = redoutput * greenoutput * blueoutput;


	return mix(color, result, 0.44);
}

vec4 SepiaColor (vec4 color)
{	
	float lum = dot(color.rgb, vec3(0.30, 0.59, 0.11));
	vec3 sepia = vec3(1.2, 1.0, 0.8); 
	sepia *= lum;
	vec3 tmp = mix(color.rgb, sepia, 0.88);

	return vec4(tmp, 1.0);
}


/*==================================
2D Noise by Ian McEwan, Ashima Arts.
==================================*/

vec3 mod289(vec3 x)		{ return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 mod289(vec2 x)		{ return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x)	{ return mod289(((x * 34.0) + 1.0) * x); }

float snoise (vec2 v)
{
const vec4 C = vec4(0.211324865405187,	// (3.0-sqrt(3.0))/6.0
					0.366025403784439,	// 0.5*(sqrt(3.0)-1.0)
					-0.577350269189626,	// -1.0 + 2.0 * C.x
					0.024390243902439);	// 1.0 / 41.0

	// First corner
	vec2 i  = floor(v + dot(v, C.yy) );
	vec2 x0 = v -   i + dot(i, C.xx);

	// Other corners
	vec2 i1;
	i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
	vec4 x12 = x0.xyxy + C.xxzz;
	x12.xy -= i1;

	// Permutations
	i = mod289(i); // Avoid truncation effects in permutation
	vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
		+ i.x + vec3(0.0, i1.x, 1.0 ));

	vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
	m = m*m ;
	m = m*m ;

	// Gradients: 41 points uniformly over a line, mapped onto a diamond.
	// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

	vec3 x = 2.0 * fract(p * C.www) - 1.0;
	vec3 h = abs(x) - 0.5;
	vec3 ox = floor(x + 0.5);
	vec3 a0 = x - ox;

	// Normalise gradients implicitly by scaling m
	// Approximation of: m *= inversesqrt( a0*a0 + h*h );
	m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

	// Compute final noise value at P
	vec3 g;
	g.x  = a0.x  * x0.x  + h.x  * x0.y;
	g.yz = a0.yz * x12.xz + h.yz * x12.yw;
	return 130.0 * dot(m, g);
}

void main()
{    
	vec2 uv = gl_FragCoord.xy / u_screenSize;
	vec4 color = texture2DRect(u_ScreenTex, gl_FragCoord.xy);

	if(u_params.x == 1)
		fragData = TechniColorSys1(color);   

	if(u_params.x == 2)
		fragData = TechniColorSys3(color); 

	if(u_params.x == 3)
		fragData = SepiaColor(color);
		  	
	float noise = snoise(uv * vec2(u_screenSize.x + u_rand * u_screenSize.y)) * 0.5;
	fragData += noise * u_params.y;     
	
	if ( u_rand < u_params.z )
	{
		// Pick a random spot to show scratches
		float dist = 1.0 / u_params.z;
		float d = distance(uv, vec2(u_rand * dist, u_rand * dist));
		if ( d < 0.4 )
		{
			// Generate the scratch
			float xPeriod = 8.0;
			float yPeriod = 1.0;
			float pi = 3.141592;
			float phase = u_time;
			float turbulence = snoise(uv * 2.5);
			float vScratch = 0.5 + (sin(((uv.x * xPeriod + uv.y * yPeriod + turbulence)) * pi + phase) * 0.5);
			vScratch = clamp((vScratch * 10000.0) + 0.35, 0.0, 1.0);

			fragData *= vScratch;
		}
	}

	float OuterVignetting	= 1.4 - u_params.w;
	float InnerVignetting	= 1.0 - u_params.w;

	float d = distance(vec2(0.5, 0.5), uv) * 1.414213;
	float vignetting = clamp((OuterVignetting - d) / (OuterVignetting - InnerVignetting), 0.0, 1.0);
	fragData *= vignetting;

}
