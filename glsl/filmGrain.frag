layout (binding = 0) uniform sampler2DRect u_ScreenTex;

layout(location = U_PARAM_VEC3_0) uniform vec3	u_params;	// x- noise interns
															// y - scarch intens
															// z - vigent size
layout(location = U_SCREEN_SIZE)	uniform vec2	u_screenSize; 
layout(location = U_PARAM_FLOAT_0)	uniform float	u_rand;
layout(location = U_PARAM_INT_0)	uniform int		u_time;


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
	fragData = texture2DRect(u_ScreenTex, gl_FragCoord.xy);
	
	float noise = snoise(uv * vec2(u_screenSize.x + u_rand * u_screenSize.y)) * 0.5;
	fragData += noise * u_params.x;     
	
	if ( u_rand < u_params.y )
	{
		// Pick a random spot to show scratches
		float dist = 1.0 / u_params.y;
		float d = distance(uv, vec2(u_rand * dist, u_rand * dist));
		if ( d < 0.4 )
		{
			// Generate the scratch
			float xPeriod = 8.0;
			float yPeriod = 1.0;
			float phase = u_time;
			float turbulence = snoise(uv * 2.5);
			float vScratch = 0.5 + (sin(((uv.x * xPeriod + uv.y * yPeriod + turbulence)) * PI + phase) * 0.5);
			vScratch = clamp((vScratch * 10000.0) + 0.35, 0.0, 1.0);

			fragData *= vScratch;
		}
	}

	float OuterVignetting	= 1.4 - u_params.z;
	float InnerVignetting	= 1.0 - u_params.z;

	float d = distance(vec2(0.5, 0.5), uv) * 1.414213;
	float vignetting = clamp((OuterVignetting - d) / (OuterVignetting - InnerVignetting), 0.0, 1.0);
	fragData *= vignetting;

}
