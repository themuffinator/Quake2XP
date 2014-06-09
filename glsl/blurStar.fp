uniform	sampler2DRect	u_map;
uniform float			u_starIntens;

void main(void)
{
    vec2 tx  = gl_FragCoord.xy;
	vec2 dx = vec2 (1.0) * u_starIntens;
	vec2 dx2 = vec2 (-1.0, 1.0) * u_starIntens; 

	vec2 sdx = dx;
    vec4 sum = texture2DRect(u_map, tx) * 0.134598;

    sum += (texture2DRect(u_map,tx+sdx)+texture2DRect(u_map,tx-sdx) )*0.127325;
    sdx += dx;
    sum += (texture2DRect(u_map,tx+sdx)+texture2DRect(u_map,tx-sdx) )*0.107778;
    sdx += dx;
    sum += (texture2DRect( u_map,tx+sdx)+texture2DRect(u_map,tx-sdx) )*0.081638;
    sdx += dx;
    sum += (texture2DRect(u_map,tx+sdx)+texture2DRect(u_map,tx-sdx) )*0.055335;
    sdx += dx;
    sum += (texture2DRect(u_map,tx+sdx)+texture2DRect(u_map,tx-sdx) )*0.033562;
    sdx += dx;
    sum += (texture2DRect(u_map,tx+sdx)+texture2DRect(u_map,tx-sdx) )*0.018216;
    sdx += dx;
    sum += (texture2DRect(u_map,tx+sdx)+texture2DRect(u_map,tx-sdx) )*0.008847;
    sdx += dx;

	vec2 sdx2 = dx2;
    vec4 sum2 = texture2DRect(u_map, tx) * 0.134598;

    sum2 += (texture2DRect(u_map,tx+sdx2)+texture2DRect(u_map,tx-sdx2) )*0.127325;
    sdx2 += dx2;
    sum2 += (texture2DRect(u_map,tx+sdx2)+texture2DRect(u_map,tx-sdx2) )*0.107778;
    sdx2 += dx2;
    sum2 += (texture2DRect( u_map,tx+sdx2)+texture2DRect(u_map,tx-sdx2) )*0.081638;
    sdx2 += dx2;
    sum2 += (texture2DRect(u_map,tx+sdx2)+texture2DRect(u_map,tx-sdx2) )*0.055335;
    sdx2 += dx2;
    sum2 += (texture2DRect(u_map,tx+sdx2)+texture2DRect(u_map,tx-sdx2) )*0.033562;
    sdx2 += dx2;
    sum2 += (texture2DRect(u_map,tx+sdx2)+texture2DRect(u_map,tx-sdx2) )*0.018216;
    sdx2 += dx2;
    sum2 += (texture2DRect(u_map,tx+sdx2)+texture2DRect(u_map,tx-sdx2) )*0.008847;
    sdx2 += dx2;

    gl_FragColor = sum + sum2;
}