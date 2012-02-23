varying vec2				v_deformTexCoord;
varying vec2				v_diffuseTexCoord;
varying float				v_depth;
varying vec2				v_deformMul;

uniform float				u_thickness;
uniform float				u_alpha;
uniform float				u_ColorModulate;
uniform vec2				u_viewport;
uniform vec2				u_depthParms;


uniform sampler2D			u_colorMap;
uniform sampler2D			u_dstMap;
uniform	sampler2DRect		g_depthBufferMap;
uniform	sampler2DRect		g_colorBufferMap;

float DecodeDepth (const float d, const in vec2 parms) {
	return parms.x / (parms.y - d);
}


void main()
{

//load dst texture
vec4 offset	= texture2D (u_dstMap, v_deformTexCoord.xy);

// calc texcoord for diffuse
vec2 coord;
coord.x = v_diffuseTexCoord.x + offset.z;
coord.y = v_diffuseTexCoord.y + offset.w;

//load diffuse map
vec4 diffuse  = texture2D (u_colorMap, coord.xy);  

// set vertex lighting
diffuse *= gl_Color;
diffuse = clamp(diffuse, 0.0, 1.0); 

#ifdef TRANS
vec2 N = offset.xy; // use autogen dst texture

// Z-feather
float depth = DecodeDepth(texture2DRect(g_depthBufferMap, gl_FragCoord.xy).x, u_depthParms);
N *= clamp((depth - v_depth) / u_thickness, 0.0, 1.0);

// scale by the deform multiplier and the viewport size
N *= v_deformMul * u_viewport.xy;

// chromatic aberration approximation coefficients
gl_FragColor.x = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + N * 0.85).x;
gl_FragColor.y = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + N * 1.0).y;
gl_FragColor.z = texture2DRect(g_colorBufferMap, gl_FragCoord.xy + N * 1.15).z;
//blend water texture
gl_FragColor.xyz += diffuse.xyz * u_alpha;

#else

// final color with out refract
gl_FragColor = vec4(diffuse.rgb, 1.0);

#endif
}