layout (binding = 0) uniform sampler2D	u_map;

layout (location = U_COLOR)			uniform vec4	u_fragColor;
layout(location = U_PARAM_INT_0)	uniform int		u_colorPass;

in vec2	v_texCoord; 
in vec4	v_color;

void main(void) 
{
	if(u_colorPass == 1){
		fragData = texture(u_map, v_texCoord.xy) * v_color;
		return;
	}

	fragData = u_fragColor;
}
