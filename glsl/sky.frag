layout (binding = 0) uniform sampler2D	u_map; // todo: use cubemap

layout(location = U_PARAM_INT_0)	uniform int	u_colorPass;

in vec2	v_texCoord; 

void main(void) 
{
	if(u_colorPass == 0){
		
		fragData = vec4(0.0);
		return;	
	}

	fragData = texture(u_map, v_texCoord.xy);
}
