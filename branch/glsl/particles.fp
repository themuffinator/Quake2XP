
varying float			v_depth;
varying	vec4			v_color;
uniform sampler2D		u_map0;
uniform sampler2DRect	u_depthBufferMap;
uniform vec2			u_depthParms;
uniform vec2			u_mask;
uniform float			u_thickness;
uniform float			u_colorScale;
varying vec2			v_texCoord0;


float DecodeDepth (const float d, const in vec2 parms) {
	return parms.x / (parms.y - d);
}

void main (void) {
	vec4 color = texture2D(u_map0, v_texCoord0);
	
	if(u_thickness > 0.0){
	// Z-feather
	float depth = DecodeDepth(texture2DRect(u_depthBufferMap, gl_FragCoord.xy).x, u_depthParms);
	float softness = clamp((depth - v_depth) / u_thickness, 0.0, 1.0);
	
	gl_FragColor = color * v_color * u_colorScale;
	gl_FragColor *= mix(vec4(1.0), vec4(softness), u_mask.xxxy);
	}
	else
	gl_FragColor = color * v_color;
}
