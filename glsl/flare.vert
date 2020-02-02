layout(location = 0) in vec3	att_position;
layout(location = 4) in vec4	att_color4f;
layout(location = 5) in vec2	att_texCoordDiffuse;

layout(location = U_MVP_MATRIX)	uniform mat4	u_modelViewProjectionMatrix;
layout(location = U_MODELVIEW_MATRIX)	uniform mat4	u_modelViewMatrix; 

out float v_depth;
out vec2	v_texCoord; 
out	vec4	v_color;

void main (void) {
	v_texCoord	= att_texCoordDiffuse;	
	v_color			= att_color4f;
  // soft depth
	v_depth = -(u_modelViewMatrix * vec4(att_position, 1.0)).z;
	gl_Position		= u_modelViewProjectionMatrix * vec4(att_position, 1.0);
}
