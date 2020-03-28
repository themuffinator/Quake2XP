layout (location = 0) in vec3 att_position;
layout (location = 5) in vec2 att_texCoordDiffuse;

layout(location = U_REFR_DEFORM_MUL)	uniform float	u_deformMul;
layout(location = U_MVP_MATRIX)			uniform mat4	u_modelViewProjectionMatrix;
layout(location = U_MODELVIEW_MATRIX)	uniform mat4	u_modelViewMatrix;
layout(location = U_PROJ_MATRIX)		uniform mat4	u_projectionMatrix;

out vec2	v_deformTexCoord;
out vec2	v_deformMul;
out vec2	v_uv;
out float	v_depth;
out float	v_depthS;

void main (void) {
	
	v_deformTexCoord = att_texCoordDiffuse;

	// compute view space depth
	vec4 positionVS = u_modelViewMatrix * vec4(att_position, 1.0);
	positionVS = vec4(1.0, 0.0, positionVS.z, 1.0);
	v_depth = -positionVS.z;
	
	v_depthS = -(u_modelViewMatrix * vec4(att_position, 1.0)).z;

	// compute the deform strength
	vec2 d = (u_projectionMatrix * positionVS).xw;
	d.x /= max(d.y, 1.0);

	v_deformMul = min(d.xx, 0.02) * u_deformMul;

	gl_Position = u_modelViewProjectionMatrix * vec4(att_position, 1.0);
	v_uv = (gl_Position.xy / gl_Position.w) * (0.5) + vec2(0.5);
}
