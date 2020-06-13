#version 450 core

#extension GL_ARB_bindless_texture : enable
#extension GL_ARB_gpu_shader5 : enable
#extension GL_ARB_explicit_uniform_location : enable

precision mediump float;
precision mediump int;

out vec4 fragData;	// out fragColor

#define	CUTOFF_EPSILON	1.0 / 255.0 
#define	PI				3.14159265358979323846 
#define	HALF_PI			1.57079632679489661923 
#define	SQRT_PI			1.77245385090551602729 
#define	SQRT_THREE		1.73205080756887729352 
#define	INV_PI			(1.0 / PI)

#define	U_MVP_MATRIX			0 
#define	U_MODELVIEW_MATRIX		1 
#define	U_PROJ_MATRIX			2 
#define	U_ORTHO_MATRIX			3 

#define	U_TEXTURE0_MATRIX		4 
#define	U_TEXTURE1_MATRIX		5 
#define	U_TEXTURE2_MATRIX		6 
#define	U_TEXTURE3_MATRIX		7 
#define	U_TEXTURE4_MATRIX		8 
#define	U_TEXTURE5_MATRIX		9 
#define	U_TEXTURE6_MATRIX		10 

#define	U_ATTEN_MATRIX			11 
#define	U_SPOT_MATRIX			12 
#define	U_CUBE_MATRIX			13 

#define	U_SCREEN_SIZE			14 
#define	U_DEPTH_PARAMS			15 
#define	U_COLOR					16 
#define	U_COLOR_OFFSET			17 	// glow shift
#define	U_COLOR_MUL				18 	// color multipler

#define	U_SCROLL				19 
#define	U_AMBIENT_LEVEL			20 
#define	U_LM_TYPE				21 
#define	U_PARALLAX_TYPE			22 
#define	U_PARALLAX_PARAMS		23 
#define	U_USE_SSAO				24 
#define	U_LAVA_PASS				25 
#define	U_SHELL_PASS			26 
#define	U_SHELL_PARAMS			27 
#define	U_ENV_PASS				28 
#define	U_ENV_SCALE				29 

#define	U_LIGHT_POS				30 
#define	U_VIEW_POS				31 
#define	U_USE_FOG				32 
#define	U_FOG_DENSITY			33 
#define	U_USE_CAUSTICS			34 
#define	U_CAUSTICS_SCALE		35 
#define	U_AMBIENT_LIGHT			36 
#define	U_SPOT_LIGHT			37 
#define	U_SPOT_PARAMS			38 
#define	U_USE_AUTOBUMP			39 
#define	U_AUTOBUMP_PARAMS		40 
#define	U_USE_RGH_MAP			41 
#define	U_RGH_SCALE				42 
#define	U_SPECULAR_SCALE		43 

#define	U_TRANS_PASS			44 

#define	U_COLOR_PARAMS			45 
#define	U_COLOR_VIBRANCE		46 

#define	U_PARTICLE_THICKNESS	47 	
#define	U_PARTICLE_MASK			48 
#define	U_TEXCOORD_OFFSET		49 
#define	U_PARTICLE_ANIM			50 

#define	U_PARAM_VEC2_0			51 
#define	U_PARAM_VEC2_1			52 
#define	U_PARAM_VEC2_2			53 
#define	U_PARAM_VEC2_3			54 
#define	U_PARAM_VEC2_4			55 
#define	U_PARAM_VEC2_5			56 

#define	U_PARAM_VEC3_0			57 
#define	U_PARAM_VEC3_1			58 
#define	U_PARAM_VEC3_2			59 
#define	U_PARAM_VEC3_3			60 
#define	U_PARAM_VEC3_4			61 
#define	U_PARAM_VEC3_5			62 

#define	U_PARAM_VEC4_0			63 
#define	U_PARAM_VEC4_1			64 
#define	U_PARAM_VEC4_2			65 
#define	U_PARAM_VEC4_3			66 
#define	U_PARAM_VEC4_4			67 
#define	U_PARAM_VEC4_5			68 

#define	U_PARAM_FLOAT_0			69 
#define	U_PARAM_FLOAT_1			70 
#define	U_PARAM_FLOAT_2			71 
#define	U_PARAM_FLOAT_3			72 
#define	U_PARAM_FLOAT_4			73 
#define	U_PARAM_FLOAT_5			74 

#define	U_PARAM_INT_0			75 
#define	U_PARAM_INT_1			76 
#define	U_PARAM_INT_2			77 
#define	U_PARAM_INT_3			78 
#define	U_PARAM_INT_4			79 
#define	U_PARAM_INT_5			80 

#define	U_REFR_ALPHA			81 
#define	U_REFR_DEFORM_MUL		82 
#define	U_REFR_THICKNESS0		83 
#define	U_REFR_THICKNESS1		84 
#define	U_REFR_ALPHA_MASK		85 
#define	U_REFR_MASK				86 

#define	U_WATER_DEFORM_MUL		87 
#define	U_WATER_ALPHA			88 
#define	U_WATHER_THICKNESS		89 
#define	U_WATER_TRANS			90 
#define	U_WATER_MIRROR			91 

#define	U_CONSOLE_BACK			92 
#define	U_2D_PICS				93 
#define	U_FRAG_COLOR			94 

#define	U_BINDLESS_ARRAY		95 

#define	U_TMU0		96 
#define	U_TMU1		97 
#define	U_TMU2		98 
#define	U_TMU3		99 
#define	U_TMU4		100 
#define	U_TMU5		101 
#define	U_TMU6		102 
#define	U_TMU7		103 
#define	U_TMU8		104 
#define	U_TMU9		105 
#define	U_TMU10		106 
