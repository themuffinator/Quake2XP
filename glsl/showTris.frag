layout (location = U_COLOR)	uniform vec3	u_trisColor;
void main(void) 
{
fragData = vec4(u_trisColor.r, u_trisColor.g, u_trisColor.b, 1.0);
}
