uniform sampler2DRect 	u_ScreenTex;
uniform sampler2DRect 	u_DepthTex;
uniform vec2 			u_screenSize;
uniform mat4 			u_ProjMat;
uniform mat4 			u_ModelViewProj;

void main(void) 
{
	// tex coord conversion 
	vec2 fc = gl_FragCoord.xy / u_screenSize;

	// derive clip space from the depth buffer and screen position
	float windowZ = texture2DRect( u_DepthTex, gl_FragCoord.xy).x;
	vec3 ndc = vec3(fc * 2.0 - 1.0, windowZ * 2.0 - 1.0 );
	float clipW = -u_ProjMat[2][3] / ( -u_ProjMat[2][2] - ndc.z );

	vec4 clip = vec4( ndc * clipW, clipW );

	// convert from clip space this frame to clip space previous frame
	vec4 reClip;
	reClip.x = dot( u_ModelViewProj[0], clip);
	reClip.y = dot( u_ModelViewProj[1], clip);
	reClip.z = dot( u_ModelViewProj[2], clip);
	reClip.w = dot( u_ModelViewProj[3], clip);

	// convert to NDC values
	vec2 prevTexCoord;
	prevTexCoord.x = ( reClip.x / reClip.w ) * 0.5 + 0.5;
	prevTexCoord.y = ( reClip.y / reClip.w ) * 0.5 + 0.5;

	// sample along the line from prevTexCoord to fragment.texcoord0

	vec2 texCoord = prevTexCoord; //fragment.texcoord0;
	vec2 delta = ( fc - prevTexCoord );

	vec3 sum = vec3( 0.0 );
	float goodSamples = 0;
	float samples = 16;

	for ( float i = 0 ; i < samples ; i = i + 1 ) {
		vec2 pos = fc + delta * ( ( i / ( samples - 1 ) ) - 0.5 );
	//	vec4 color = texture2DRect( u_ScreenTex, pos * u_screenSize );		
		vec4 color = texture2DRect( u_ScreenTex, pos );
		// only take the values that are not part of the weapon
		sum += color.xyz * color.w;
		goodSamples += color.w;
	}
	float invScale = 1.0 / goodSamples;

	gl_FragColor = vec4( sum * invScale, 1.0 );
}
