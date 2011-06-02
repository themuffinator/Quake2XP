uniform sampler2DRect u_ScreenTex;
uniform sampler2DRect u_DepthTex;
uniform vec2 u_screenSize;
uniform vec2 u_depthParms;
uniform float u_focus;  // = 512.0;
uniform float u_bias;	// = 0.005; aperture - bigger values for shallower depth of field
 
float blurClamp = 3.0;  // max blur amount

float DecodeDepth (const float d, const in vec2 parms) {
	return parms.x / (parms.y - d);
}
 
void main(void) 
{
		float aspectratio = u_screenSize.x / u_screenSize.y;
        vec2 aspectcorrect = vec2(1.0, aspectratio);
   
		// Z-feather
		float depth = DecodeDepth(texture2DRect(u_DepthTex, gl_FragCoord.xy).x, u_depthParms);
		float factor = depth - u_focus;

		vec2 dofblur = vec2 (clamp(factor * u_bias, -blurClamp, blurClamp));
        
		vec4 col = vec4(0.0);
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy);
		col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.0, 0.4 )*aspectcorrect) * dofblur);
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.15,0.37 )*aspectcorrect) * dofblur);
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.29,0.29 )*aspectcorrect) * dofblur);
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( -0.37,0.15 )*aspectcorrect) * dofblur);        
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.4,0.0 )*aspectcorrect) * dofblur);   
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.37,-0.15 )*aspectcorrect) * dofblur);        
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.29,-0.29 )*aspectcorrect) * dofblur);        
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( -0.15,-0.37 )*aspectcorrect) * dofblur);
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.0,-0.4 )*aspectcorrect) * dofblur);  
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( -0.15,0.37 )*aspectcorrect) * dofblur);
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( -0.29,0.29 )*aspectcorrect) * dofblur);
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.37,0.15 )*aspectcorrect) * dofblur); 
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( -0.4,0.0 )*aspectcorrect) * dofblur);  
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( -0.37,-0.15 )*aspectcorrect) * dofblur);       
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( -0.29,-0.29 )*aspectcorrect) * dofblur);       
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.15,-0.37 )*aspectcorrect) * dofblur);
        
		#ifdef EXTRA
		col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.15,0.37 )*aspectcorrect) * dofblur*0.9);
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( -0.37,0.15 )*aspectcorrect) * dofblur*0.9);            
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.37,-0.15 )*aspectcorrect) * dofblur*0.9);            
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( -0.15,-0.37 )*aspectcorrect) * dofblur*0.9);
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( -0.15,0.37 )*aspectcorrect) * dofblur*0.9);
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.37,0.15 )*aspectcorrect) * dofblur*0.9);             
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( -0.37,-0.15 )*aspectcorrect) * dofblur*0.9);   
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.15,-0.37 )*aspectcorrect) * dofblur*0.9);    
        
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.29,0.29 )*aspectcorrect) * dofblur*0.7);
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.4,0.0 )*aspectcorrect) * dofblur*0.7);       
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.29,-0.29 )*aspectcorrect) * dofblur*0.7);    
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.0,-0.4 )*aspectcorrect) * dofblur*0.7);      
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( -0.29,0.29 )*aspectcorrect) * dofblur*0.7);
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( -0.4,0.0 )*aspectcorrect) * dofblur*0.7);      
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( -0.29,-0.29 )*aspectcorrect) * dofblur*0.7);   
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.0,0.4 )*aspectcorrect) * dofblur*0.7);
                         
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.29,0.29 )*aspectcorrect) * dofblur*0.4);
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.4,0.0 )*aspectcorrect) * dofblur*0.4);       
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.29,-0.29 )*aspectcorrect) * dofblur*0.4);    
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.0,-0.4 )*aspectcorrect) * dofblur*0.4);      
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( -0.29,0.29 )*aspectcorrect) * dofblur*0.4);
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( -0.4,0.0 )*aspectcorrect) * dofblur*0.4);      
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( -0.29,-0.29 )*aspectcorrect) * dofblur*0.4);   
        col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.0,0.4 )*aspectcorrect) * dofblur*0.4);       
        gl_FragColor = col/41.0;
		
		#else
		               
        gl_FragColor = col/17.0;

		#endif
		
}
