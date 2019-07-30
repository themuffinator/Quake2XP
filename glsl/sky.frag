//layout (binding = 0) uniform samplerCube	u_map;

layout (bindless_sampler, location  = U_TMU0) uniform samplerCube	u_map;

layout(location = U_PARAM_INT_0)	uniform int	u_colorPass;

in vec3	v_texCoord; 

void main(void) 
{
	if(u_colorPass == 0){
		
		fragData = vec4(0.0);
		return;	
	}

	fragData = textureLod(u_map, v_texCoord.xyz, 0.0);
}
