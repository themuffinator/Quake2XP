layout (binding = 0) uniform sampler2DRect		u_map0; // screen
layout (binding = 1) uniform sampler2DRect		u_map1; // bloom

uniform float	u_bloomParams; //multipler

float A = 0.15;
float B = 0.50;
float C = 0.10;
float D = 0.20;
float E = 0.02;
float F = 0.30;
float W = 11.2;

vec3 Uncharted2Tonemap(vec3 x)
{
   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}
#define GAMMA 1.f/0.7f

void main(void) { 

	vec4 screen = texture2DRect(u_map0, gl_FragCoord.xy); 
	vec4 bloom = texture2DRect(u_map1, gl_FragCoord.xy * 0.25);
	
	screen *= 4.f;
	bloom  *= 4.f;

	screen += bloom * u_bloomParams;

	float bias = 2.f;
	vec3 tonemap = Uncharted2Tonemap(screen.rgb * bias);
	
	vec3 whiteScale = 1.f / Uncharted2Tonemap(vec3(W));
	vec3 color = tonemap * whiteScale;
	
	color = pow(color, vec3(GAMMA));

	fragData = vec4(color, 1.f);
}
