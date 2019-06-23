layout(location = 0) in vec3	att_position;
layout(location = U_MVP_MATRIX)	uniform mat4	u_modelViewProjectionMatrix;

out vec3		v_texCoord; 

void main (void) {

	mat3 idmat;
	idmat[0][0] = 1.0;
	idmat[0][1] = 0.0;
	idmat[0][2] = 0.0;
		
	idmat[1][0] = 0.0;
	idmat[1][1] = 1.0;
	idmat[1][2] = 0.0;
	
	idmat[2][0] = 0.0;
	idmat[2][1] = 0.0;
	idmat[2][2] = 1.0;


	v_texCoord		= idmat * att_position;	
	gl_Position		= u_modelViewProjectionMatrix * vec4(att_position, 1.0);
}
