/*
uniform sampler2DRect u_mask;
uniform float u_alpha;
uniform vec2 u_screenSize;
*/

void main(void) 
{
/*
vec2 offset = vec2( u_screenSize.y / u_screenSize.x , 1.0);
// Box Blur with 5x5 kernel

float
mask  = texture2DRect(u_mask, gl_FragCoord.xy + vec2(offset.x * -2.0,	offset.y * -2.0)).a;
mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(offset.x * -1.0,	offset.y * -2.0)).a;
mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(0.0,				offset.y * -2.0)).a;
mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(offset.x,			offset.y * -2.0)).a;
mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(offset.x * 2.0,	offset.y * -2.0)).a;

mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(offset.x * -2.0,	offset.y * -1.0)).a;
mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(offset.x * -1.0,	offset.y * -1.0)).a;
mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(0.0,				offset.y * -1.0)).a;
mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(offset.x,			offset.y * -1.0)).a;
mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(offset.x * 2.0,	offset.y * -1.0)).a;

mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(offset.x * -2.0,	0.0)).a;
mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(offset.x * -1.0,	0.0)).a;
mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(0.0,				0.0)).a;
mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(offset.x,			0.0)).a;
mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(offset.x * 2.0,	0.0)).a;

mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(offset.x * -2.0,	offset.y)).a;
mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(offset.x * -1.0,	offset.y)).a;
mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(0.0,				offset.y)).a;
mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(offset.x,			offset.y)).a;
mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(offset.x * 2.0,	offset.y)).a;

mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(offset.x * -2.0,	offset.y * 2.0)).a;
mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(offset.x * -1.0,	offset.y * 2.0)).a;
mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(0.0,               offset.y * 2.0)).a;
mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(offset.x,			offset.y * 2.0)).a;
mask += texture2DRect(u_mask, gl_FragCoord.xy + vec2(offset.x * 2.0,	offset.y * 2.0)).a;

mask /=25.0;
fragData = vec4(0.0, 0.0, 0.0, mask * u_alpha);
*/
fragData = vec4(0.0);
}
