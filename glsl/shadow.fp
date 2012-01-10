uniform sampler2DRect u_mask;
uniform float u_alpha;

void main(void) 
{
 vec4 mask = texture2DRect(u_mask, gl_FragCoord.xy);
 
 mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(1.0, 0.0));
 mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(2.0, 0.0));
 mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(3.0, 0.0));
 mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(4.0, 0.0));

 mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(-1.0, 0.0));
 mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(-2.0, 0.0));
 mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(-3.0, 0.0));
 mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(-4.0, 0.0));

 mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(0.0, 1.0));
 mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(0.0, 2.0));
 mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(0.0, 3.0));
 mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(0.0, 4.0));

 mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(0.0, -1.0));
 mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(0.0, -2.0));
 mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(0.0, -3.0));
 mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(0.0, -4.0));

 mask /= 16.0;

 gl_FragColor = vec4(0.0, 0.0, 0.0, mask.a * u_alpha);
}
