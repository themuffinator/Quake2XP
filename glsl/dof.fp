uniform sampler2DRect	u_ScreenTex;
uniform sampler2DRect	u_DepthTex;
uniform vec2			u_screenSize;
uniform vec4			u_dofParams;

#include depth.inc

void main(void) 
{
		float aspectratio = u_screenSize.x / u_screenSize.y;
        vec2 aspectcorrect = vec2(1.0, aspectratio);
   
		// Z-feather
		float depth = DecodeDepth(texture2DRect(u_DepthTex, gl_FragCoord.xy).x, u_dofParams.zw);
		float factor = depth - u_dofParams.x;

		vec2 dofblur = vec2 (clamp(factor * u_dofParams.y, -3.0, 3.0));

		//def 17
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

        // extra 41
	/*	col += texture2DRect(u_ScreenTex, gl_FragCoord.xy + (vec2( 0.15,0.37 )*aspectcorrect) * dofblur*0.9);
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
        */
		fragData = col/17.0;
		
}
