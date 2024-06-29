/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/*
==============================

SHADING LANGUAGE INTERFACE

==============================
*/
#include "r_local.h"

#define MAX_INFO_LOG		4096

#define	PROGRAM_HASH_SIZE	MAX_PROGRAMS

static glslProgram_t		*programHashTable[PROGRAM_HASH_SIZE];
int r_numPrograms;
static glslProgram_t	r_nullProgram;

static const char * glslGlobals =
"#version 450 core\n"
"#extension GL_ARB_bindless_texture : enable\n"
"#extension GL_ARB_gpu_shader5 : enable\n"
"#extension GL_ARB_explicit_uniform_location : enable\n"
"precision mediump float;\n"
"precision mediump int;\n"

"out vec4 fragData;\n"

"#ifndef saturate\n"
"#define saturate(x)    clamp(x, 0.0, 1.0)\n"
"#endif\n"

"#define	CUTOFF_EPSILON	1.0 / 255.0\n"
"#define	PI				3.14159265358979323846\n"
"#define	HALF_PI			1.57079632679489661923\n"
"#define	SQRT_PI			1.77245385090551602729\n"
"#define	SQRT_THREE		1.73205080756887729352\n"
"#define	INV_PI			(1.0 / PI)\n"

"#define	U_MVP_MATRIX			0\n"
"#define	U_MODELVIEW_MATRIX		1\n"
"#define	U_PROJ_MATRIX			2\n"
"#define	U_ORTHO_MATRIX			3\n"

"#define	U_TEXTURE0_MATRIX		4\n"
"#define	U_TEXTURE1_MATRIX		5\n"
"#define	U_TEXTURE2_MATRIX		6\n"
"#define	U_TEXTURE3_MATRIX		7\n"
"#define	U_TEXTURE4_MATRIX		8\n"
"#define	U_TEXTURE5_MATRIX		9\n"
"#define	U_TEXTURE6_MATRIX		10\n"

"#define	U_ATTEN_MATRIX			11\n"
"#define	U_SPOT_MATRIX			12\n"
"#define	U_CUBE_MATRIX			13\n"

"#define	U_SCREEN_SIZE			14\n"
"#define	U_DEPTH_PARAMS			15\n"
"#define	U_COLOR					16\n"
"#define	U_COLOR_OFFSET			17\n"	// glow shift
"#define	U_COLOR_MUL				18\n"	// color multipler

"#define	U_SCROLL				19\n"
"#define	U_AMBIENT_LEVEL			20\n"
"#define	U_LM_TYPE				21\n"
"#define	U_PARALLAX_TYPE			22\n"
"#define	U_PARALLAX_PARAMS		23\n"
"#define	U_USE_SSAO				24\n"
"#define	U_LAVA_PASS				25\n"
"#define	U_SHELL_PASS			26\n"
"#define	U_SHELL_PARAMS			27\n"
"#define	U_ENV_PASS				28\n"
"#define	U_ENV_SCALE				29\n"

"#define	U_LIGHT_POS				30\n"
"#define	U_VIEW_POS				31\n"
"#define	U_USE_FOG				32\n"
"#define	U_FOG_DENSITY			33\n"
"#define	U_USE_CAUSTICS			34\n"
"#define	U_CAUSTICS_SCALE		35\n"
"#define	U_AMBIENT_LIGHT			36\n"
"#define	U_SPOT_LIGHT			37\n"
"#define	U_SPOT_PARAMS			38\n"
"#define	U_USE_AUTOBUMP			39\n"
"#define	U_AUTOBUMP_PARAMS		40\n"
"#define	U_USE_RGH_MAP			41\n"
"#define	U_RGH_SCALE				42\n"
"#define	U_SPECULAR_SCALE		43\n"

"#define	U_TRANS_PASS			44\n"

"#define	U_COLOR_PARAMS			45\n"
"#define	U_COLOR_VIBRANCE		46\n"

"#define	U_PARTICLE_THICKNESS	47\n"	
"#define	U_PARTICLE_MASK			48\n"
"#define	U_TEXCOORD_OFFSET		49\n"
"#define	U_PARTICLE_ANIM			50\n"

"#define	U_PARAM_VEC2_0			51\n"
"#define	U_PARAM_VEC2_1			52\n"
"#define	U_PARAM_VEC2_2			53\n"
"#define	U_PARAM_VEC2_3			54\n"
"#define	U_PARAM_VEC2_4			55\n"
"#define	U_PARAM_VEC2_5			56\n"

"#define	U_PARAM_VEC3_0			57\n"
"#define	U_PARAM_VEC3_1			58\n"
"#define	U_PARAM_VEC3_2			59\n"
"#define	U_PARAM_VEC3_3			60\n"
"#define	U_PARAM_VEC3_4			61\n"
"#define	U_PARAM_VEC3_5			62\n"

"#define	U_PARAM_VEC4_0			63\n"
"#define	U_PARAM_VEC4_1			64\n"
"#define	U_PARAM_VEC4_2			65\n"
"#define	U_PARAM_VEC4_3			66\n"
"#define	U_PARAM_VEC4_4			67\n"
"#define	U_PARAM_VEC4_5			68\n"

"#define	U_PARAM_FLOAT_0			69\n"
"#define	U_PARAM_FLOAT_1			70\n"
"#define	U_PARAM_FLOAT_2			71\n"
"#define	U_PARAM_FLOAT_3			72\n"
"#define	U_PARAM_FLOAT_4			73\n"
"#define	U_PARAM_FLOAT_5			74\n"

"#define	U_PARAM_INT_0			75\n"
"#define	U_PARAM_INT_1			76\n"
"#define	U_PARAM_INT_2			77\n"
"#define	U_PARAM_INT_3			78\n"
"#define	U_PARAM_INT_4			79\n"
"#define	U_PARAM_INT_5			80\n"

"#define	U_REFR_ALPHA			81\n"
"#define	U_REFR_DEFORM_MUL		82\n"
"#define	U_REFR_THICKNESS0		83\n"
"#define	U_REFR_THICKNESS1		84\n"
"#define	U_REFR_ALPHA_MASK		85\n"
"#define	U_REFR_MASK				86\n"

"#define	U_WATER_DEFORM_MUL		87\n"
"#define	U_WATER_ALPHA			88\n"
"#define	U_WATHER_THICKNESS		89\n"
"#define	U_WATER_TRANS			90\n"
"#define	U_WATER_MIRROR			91\n"

"#define	U_CONSOLE_BACK			92\n"
"#define	U_2D_PICS				93\n"
"#define	U_FRAG_COLOR			94\n"

"#define	U_BINDLESS_ARRAY		95\n"

"#define		U_PARAM_iVEC2_0 96\n"
"#define		U_PARAM_iVEC2_1 97\n"
"#define		U_PARAM_iVEC2_2 98\n"
"#define		U_PARAM_iVEC2_3 99\n"
"#define		U_PARAM_iVEC2_4 100\n"
"#define		U_PARAM_iVEC2_5 101\n"

"#define	U_TMU0		102\n"
"#define	U_TMU1		103\n"
"#define	U_TMU2		104\n"
"#define	U_TMU3		105\n"
"#define	U_TMU4		106\n"
"#define	U_TMU5		107\n"
"#define	U_TMU6		108\n"
"#define	U_TMU7		109\n"
"#define	U_TMU8		110\n"
"#define	U_TMU9		111\n"
"#define	U_TMU10		112\n"
;

typedef enum {
	S_TESSELATION	= 1,
	S_GEO			= 2,
	S_COMP			= 4,
}shaderType;


/*
=================
Com_HashKey

=================
*/
unsigned Com_ProgramHashKey (const char *string, unsigned size) {
	int			i;
	unsigned	hash = 0;
	char		letter;

	for (i = 0; string[i]; i++) {
		letter = tolower (string[i]);

		if (letter == '.') break;				// don't include extension
		if (letter == '\\') letter = '/';		// damn path names

		hash += (unsigned)letter * (i + 119);
	}
	hash = (hash ^ (hash >> 10) ^ (hash >> 20));
	hash %= size;

	return hash;
}

/*
===============
Q_IsLiteral

===============
*/
bool Q_IsLiteral (const char *text) {
	int		i, c, len;

	len = strlen (text);

	for (i = 0; i < len; i++) {
		c = text[i];

		if ((c < 'a' || c > 'z') && (c < 'A' || c > 'Z') && c != '_')
			return false;
	}

	return true;
}

/*
==========================================

MISCELLANEOUS

==========================================
*/

/*
==============
R_ProgramForName

==============
*/
static glslProgram_t *R_ProgramForName (const char *name) {
	glslProgram_t	*program;
	unsigned	hash;

	hash = Com_ProgramHashKey(name, PROGRAM_HASH_SIZE);

	for (program = programHashTable[hash]; program; program = program->nextHash) {
		if (!Q_stricmp (program->name, name))
			return program;
	}

	return NULL;
}

/*
===============
R_GetInfoLog

===============
*/
static void R_GetInfoLog (int id, char *log, bool isProgram) {
	int		length, dummy;

	if (isProgram)
		qglGetProgramiv (id, GL_INFO_LOG_LENGTH, &length);
	else
		qglGetShaderiv (id, GL_INFO_LOG_LENGTH, &length);

	if (length < 1) {
		log[0] = 0;
		return;
	}

	if (length >= MAX_INFO_LOG)
		length = MAX_INFO_LOG - 1;

	if (isProgram)
		qglGetProgramInfoLog (id, length, &dummy, log);
	else
		qglGetShaderInfoLog (id, length, &dummy, log);

	log[length] = 0;
}


/*
==============
R_LoadIncludes

Search shader texts for '#include' directives
and insert included file contents.
==============
*/
void *Z_Malloc(int size);

char *R_LoadIncludes (char *glsl) {
	char filename[MAX_QPATH];
	char *token, *p, *oldp, *oldglsl;
	int l, limit = 64;          // limit for prevent infinity recursion

	/// calculate size of glsl with includes
	l = strlen (glsl);
	p = glsl;
	while (1) {
		oldp = p;
		token = Com_ParseExt (&p, true);
		if (!token[0])
			break;

		if (!strcmp (token, "#include")) {
			int	li;
			char	*buf;

			if (limit < 0)
				Com_Error (ERR_FATAL, "R_LoadIncludes: more than 64 includes");

			token = Com_ParseExt (&p, false);
			Com_sprintf (filename, sizeof(filename), "glsl/include/%s", token);
			li = FS_LoadFile (filename, (void **)&buf);
			if (!buf)
				Com_Error (ERR_FATAL, "Couldn't load %s", filename);

			oldglsl = glsl;
			glsl = (char*)Q_malloc(l + li + 2);
			memset (glsl, 0, l + li + 2);
			Q_memcpy (glsl, oldglsl, oldp - oldglsl);
			Q_strcat (glsl, "\n", l + li + 1);
			Q_strcat (glsl, buf, l + li + 1);
			Q_strcat (glsl, p, l + li + 1);
			p = oldp - oldglsl + glsl;
			l = strlen (glsl);
			FS_FreeFile (buf);
			limit--;
		}
	}

	return glsl;
}

/*
===============================
Try To Load Precompiled Shaders
===============================
*/

bool R_LoadBinaryShader(char *shaderName, int shaderId) {

	char			name[MAX_QPATH];
	GLint			binLength;
	GLvoid*			bin;
	GLint			success;
	FILE*			binFile;

	if (!r_useShaderCache->integer)
		return false;

	Com_sprintf(name, sizeof(name), "%s/shadercache/%s.bin", FS_Gamedir(), shaderName);
	FS_CreatePath(name);

	binFile = fopen(name, "rb");
	if (!binFile) {
		return false;
	}
	else {
		fseek(binFile, 0, SEEK_END);
		binLength = (GLint)ftell(binFile);
		bin = (GLvoid*)malloc(binLength);
		fseek(binFile, 0, SEEK_SET);
		fread(bin, binLength, 1, binFile);
		fclose(binFile);

		glProgramBinary(shaderId, gl_state.binaryFormats, bin, binLength);
		qglGetProgramiv(shaderId, GL_LINK_STATUS, &success);
		free(bin);

		if (success) {
			Com_DPrintf(S_COLOR_GREEN">bin\n");
			return true;
		}
	}
	return false;
}

/*
==============
R_CreateProgram

==============
*/

static glslProgram_t *R_CreateProgram (	const char *name, const char *vertexSource, const char *fragmentSource, 
						const char *tessControlSource, const char *tessEvalSource, const char *geoSource, const char *compSource) {
	char			log[MAX_INFO_LOG];
	unsigned		hash;
	glslProgram_t	*program;
	const char		*strings[MAX_PROGRAM_DEFS * 3 + 2];
	int				numStrings;
	int				numLinked = 0;
	int				id, vertexId, fragmentId, geoId, controlId, evalId, compId;
	int				status;
	int				i;

	if ((vertexSource && strlen (vertexSource) < 17) || (fragmentSource && strlen (fragmentSource) < 17))
		return NULL;

	if (r_numPrograms == MAX_PROGRAMS)
		VID_Error (ERR_DROP, "R_CreateProgram: MAX_PROGRAMS hit");


	for (i = 0, program = r_programs; i < r_numPrograms; i++, program++) {
		if (!r_programs->name[0])
			break;
	}

	if (i == r_numPrograms) {
		if (r_numPrograms == MAX_PROGRAMS)
			VID_Error (ERR_DROP, "MAX_PROGRAMS");
		r_numPrograms++;
	}
	program = &r_programs[i];

	memset (program, 0, sizeof(*program));
	Q_strncpyz (program->name, name, sizeof(program->name));

	id = qglCreateProgram();

	if (!R_LoadBinaryShader(program->name, id)) { // can't load shader from cache - recompile it!

		numStrings = 0;
		vertexId = 0;
		fragmentId = 0;
		controlId = 0;
		evalId = 0;
		geoId = 0;
		compId = 0;

		if(!compSource)
			strings[numStrings++] = glslGlobals;

		// compile vertex shader
		if (vertexSource) {
			// link includes
			vertexSource = R_LoadIncludes((char*)vertexSource);

			strings[numStrings] = vertexSource;
			vertexId = qglCreateShader(GL_VERTEX_SHADER);
			qglShaderSource(vertexId, numStrings + 1, strings, NULL);
			qglCompileShader(vertexId);
			qglGetShaderiv(vertexId, GL_COMPILE_STATUS, &status);

			if (!status) {
				R_GetInfoLog(vertexId, log, false);
				qglDeleteShader(vertexId);
				Com_Printf("program '%s': error(s) in vertex shader:\n-----------\n%s\n-----------\n", program->name, log);
				return NULL;
			}
		}

		// tess control
		if (tessControlSource) {
			// link includes
			tessControlSource = R_LoadIncludes((char *)tessControlSource);
			strings[numStrings] = tessControlSource;
			controlId = qglCreateShader(GL_TESS_CONTROL_SHADER);

			//Com_Printf("program '%s': warning(s) in: %s\n", program->name, log); // debug depricated func

			qglShaderSource(controlId, numStrings + 1, strings, NULL);
			qglCompileShader(controlId);
			qglGetShaderiv(controlId, GL_COMPILE_STATUS, &status);

			if (!status) {
				R_GetInfoLog(controlId, log, false);
				qglDeleteShader(controlId);
				Com_Printf("program '%s': error(s) in fragment shader:\n-----------\n%s\n-----------\n", program->name, log);
				return NULL;
			}
		}

		// tess eval
		if (tessEvalSource) {
			// link includes
			tessEvalSource = R_LoadIncludes((char *)tessEvalSource);
			strings[numStrings] = tessEvalSource;
			evalId = qglCreateShader(GL_TESS_EVALUATION_SHADER);

			//Com_Printf("program '%s': warning(s) in: %s\n", program->name, log); // debug depricated func

			qglShaderSource(evalId, numStrings + 1, strings, NULL);
			qglCompileShader(evalId);
			qglGetShaderiv(evalId, GL_COMPILE_STATUS, &status);

			if (!status) {
				R_GetInfoLog(evalId, log, false);
				qglDeleteShader(evalId);
				Com_Printf("program '%s': error(s) in fragment shader:\n-----------\n%s\n-----------\n", program->name, log);
				return NULL;
			}
		}

		// compile geo shader
		if (geoSource) {
			// link includes
			geoSource = R_LoadIncludes((char *)geoSource);
			strings[numStrings] = geoSource;
			geoId = qglCreateShader(GL_GEOMETRY_SHADER);

			//Com_Printf("program '%s': warning(s) in: %s\n", program->name, log); // debug depricated func

			qglShaderSource(geoId, numStrings + 1, strings, NULL);
			qglCompileShader(geoId);
			qglGetShaderiv(geoId, GL_COMPILE_STATUS, &status);

			if (!status) {
				R_GetInfoLog(geoId, log, false);
				qglDeleteShader(geoId);
				Com_Printf("program '%s': error(s) in fragment shader:\n-----------\n%s\n-----------\n", program->name, log);
				return NULL;
			}
		}

		// compile fragment shader
		if (fragmentSource) {
			// link includes
			fragmentSource = R_LoadIncludes((char*)fragmentSource);
			strings[numStrings] = fragmentSource;
			fragmentId = qglCreateShader(GL_FRAGMENT_SHADER);

			//Com_Printf("program '%s': warning(s) in: %s\n", program->name, log); // debug depricated func

			qglShaderSource(fragmentId, numStrings + 1, strings, NULL);
			qglCompileShader(fragmentId);
			qglGetShaderiv(fragmentId, GL_COMPILE_STATUS, &status);

			if (!status) {
				R_GetInfoLog(fragmentId, log, false);
				qglDeleteShader(fragmentId);
				Com_Printf("program '%s': error(s) in fragment shader:\n-----------\n%s\n-----------\n", program->name, log);
				return NULL;
			}
		}

		// compile fragment shader
		if (compSource) {
			// link includes
			compSource = R_LoadIncludes((char *)compSource);
			strings[numStrings] = compSource;
			compId = qglCreateShader(GL_COMPUTE_SHADER);

			//Com_Printf("program '%s': warning(s) in: %s\n", program->name, log); // debug depricated func

			qglShaderSource(compId, numStrings + 1, strings, NULL);
			qglCompileShader(compId);
			qglGetShaderiv(compId, GL_COMPILE_STATUS, &status);

			if (!status) {
				R_GetInfoLog(compId, log, false);
				qglDeleteShader(compId);
				Com_Printf("program '%s': error(s) in compute shader:\n-----------\n%s\n-----------\n", program->name, log);
				return NULL;
			}
		}

		//
		// link the program
		//

		if (vertexId) {
			qglAttachShader(id, vertexId);
			qglDeleteShader(vertexId);
		}

		if (controlId) {
			qglAttachShader(id, controlId);
			qglDeleteShader(controlId);
		}

		if (evalId) {
			qglAttachShader(id, evalId);
			qglDeleteShader(evalId);
		}

		if (geoId) {
			qglAttachShader(id, geoId);
			qglDeleteShader(geoId);
		}

		if (fragmentId) {
			qglAttachShader(id, fragmentId);
			qglDeleteShader(fragmentId);
		}

		if (compId) {
			qglAttachShader(id, compId);
			qglDeleteShader(compId);
		}

		qglLinkProgram(id);
		qglGetProgramiv(id, GL_LINK_STATUS, &status);

		qglObjectLabel(GL_PROGRAM, id, strlen(name), name);

		R_GetInfoLog(id, log, true);

		if (!status) {
			qglDeleteProgram(id);
			Com_Printf("program '%s': link error(s): %s\n", program->name, log);
			return NULL;
		}

		// don't let it be slow (crap)
		if (strstr(log, "fragment shader will run in software")) {
			qglDeleteProgram(id);
			Com_Printf("program '%s': refusing to perform software emulation\n", program->name);
			return NULL;
		}
		
		if (r_useShaderCache->integer) {// make binary shader
			
			char	binName[MAX_QPATH];
			GLint	binLength;
			GLvoid*	bin;
			FILE*	binFile;

			qglGetProgramiv(id, GL_PROGRAM_BINARY_LENGTH, &binLength);
			bin = (GLvoid*)malloc(binLength);
			glGetProgramBinary(id, binLength, &binLength, &gl_state.binaryFormats, bin);

			Com_sprintf(binName, sizeof(binName), "%s/shadercache/%s.bin", FS_Gamedir(), program->name);
			FS_CreatePath(binName);
			binFile = fopen(binName, "wb");
			fwrite(bin, binLength, 1, binFile);
			fclose(binFile);
			free(bin);
		}
	}

	program->id = id;
	program->valid = true;

	// add to the hash
	hash = Com_ProgramHashKey(program->name, PROGRAM_HASH_SIZE);
	program->nextHash = programHashTable[hash];
	programHashTable[hash] = program;

	return program;
}

void Q_snprintfz (char *dst, int dstSize, const char *fmt, ...);
/*
==============
R_FindProgram

==============
*/

glslProgram_t *R_FindProgram (const char *name, int flags) {
	char			filename[MAX_QPATH];
	glslProgram_t	*program;
	char			*vertexSource = NULL, *fragmentSource = NULL, *geoSource = NULL, *tessEvalSource = NULL, *tessControlSource = NULL, *compSource = NULL;

	if (flags & S_COMP) {
		Q_snprintfz(filename, sizeof(filename), "glsl/%s.comp", name);
		FS_LoadFile(filename, (void **)&compSource);
		
		if (!compSource)
			return &r_nullProgram;
		
		program = R_CreateProgram(name, NULL, NULL, NULL, NULL, NULL, compSource);
	}
	else {
		Q_snprintfz(filename, sizeof(filename), "glsl/%s.vert", name);
		FS_LoadFile(filename, (void **)&vertexSource);
		Q_snprintfz(filename, sizeof(filename), "glsl/%s.frag", name);
		FS_LoadFile(filename, (void **)&fragmentSource);

		if (flags & S_TESSELATION) {
			Q_snprintfz(filename, sizeof(filename), "glsl/%s.tesc", name);
			FS_LoadFile(filename, (void **)&tessControlSource);
			Q_snprintfz(filename, sizeof(filename), "glsl/%s.tese", name);
			FS_LoadFile(filename, (void **)&tessEvalSource);
		}

		if (flags & S_GEO) {
			Q_snprintfz(filename, sizeof(filename), "glsl/%s.geom", name);
			FS_LoadFile(filename, (void **)&geoSource);
		}


		if (!vertexSource | !fragmentSource)
			return &r_nullProgram;		// no appropriate shaders found
		
		program = R_CreateProgram(name, vertexSource, fragmentSource,
			flags & S_TESSELATION ? tessControlSource : NULL,
			flags & S_TESSELATION ? tessEvalSource : NULL,
			flags & S_GEO ? geoSource : NULL,
			NULL);
	}

	if (vertexSource)
		FS_FreeFile (vertexSource);
	if (fragmentSource)
		FS_FreeFile (fragmentSource);

	if (tessControlSource)
		FS_FreeFile(tessControlSource);
	if (tessEvalSource)
		FS_FreeFile(tessEvalSource);
	if (geoSource)
		FS_FreeFile(geoSource);

	if (compSource)
		FS_FreeFile(compSource);

	if (!program || !program->valid)
		return &r_nullProgram;

	return program;
}

/*
=============
R_InitPrograms

=============
*/

#define GLSL_LOADING_TIME 1

void R_InitPrograms (void) {
	
	int	missing = 0;

	Com_Printf ("\nInitializing programs...\n\n");

#ifdef GLSL_LOADING_TIME
	int		start = 0, stop = 0;
	float	sec;
	start = Sys_Milliseconds ();
#endif
	 
	memset (programHashTable, 0, sizeof(programHashTable));
	memset (&r_nullProgram, 0, sizeof(glslProgram_t));

	Com_Printf ("Load "S_COLOR_YELLOW"null program"S_COLOR_WHITE" ");
	nullProgram = R_FindProgram ("null", 0);
	if (nullProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"ambient world program"S_COLOR_WHITE" ");
	ambientWorldProgram = R_FindProgram ("ambientWorld", 0);
	if (ambientWorldProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"world interaction program"S_COLOR_WHITE" ");
	lightWorldProgram = R_FindProgram ("lightWorld", 0);
	if (lightWorldProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"ambient md2 program"S_COLOR_WHITE" ");
	aliasAmbientProgram = R_FindProgram ("ambientMd2", 0);
	if (aliasAmbientProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"ambient md3 program"S_COLOR_WHITE" ");
	md3AmbientProgram = R_FindProgram("ambientMd3", 0);
	if (md3AmbientProgram->valid) {
		Com_Printf("succeeded\n");

	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"alias interaction program"S_COLOR_WHITE" ");
	aliasBumpProgram = R_FindProgram ("lightAlias", 0);

	if (aliasBumpProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"glare program"S_COLOR_WHITE" ");
	glareProgram = R_FindProgram ("glare", 0);
	glareFinalProgram = R_FindProgram("glareFinal", 0);
	if (glareProgram->valid && glareFinalProgram->valid){
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"radial blur program"S_COLOR_WHITE" ");
	radialProgram = R_FindProgram ("radialBlur", 0);

	if (radialProgram->valid){
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"dof blur program"S_COLOR_WHITE" ");
	dofProgram = R_FindProgram ("dof", 0);

	if (dofProgram->valid){
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"motion blur program"S_COLOR_WHITE" ");
	motionBlurProgram = R_FindProgram ("mblur", 0);
	
	if (motionBlurProgram->valid){
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"linear depth program"S_COLOR_WHITE" ");
	linearDepthProgram = R_FindProgram("linearDepth", 0);

	if (linearDepthProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"ssao program"S_COLOR_WHITE" ");
	ssaoProgram = R_FindProgram ("ssao", 0);
	depthDownsampleProgram = R_FindProgram("depthDownsample", 0);
	ssaoBlurProgram = R_FindProgram("ssaoBlur", 0);

	if (ssaoProgram->valid && depthDownsampleProgram->valid && ssaoBlurProgram->valid){
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"bloom program"S_COLOR_WHITE" ");
	brightProgram = R_FindProgram ("bright", 0);
	bloomFinalProgram = R_FindProgram ("bloomFinal", 0);

	if (brightProgram->valid && bloomFinalProgram->valid){
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"gauss blur program"S_COLOR_WHITE" ");
	bloomBlurProgram = R_FindProgram("bloomBlur", 0);

	if (bloomBlurProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}


	Com_Printf ("Load "S_COLOR_YELLOW"glass program"S_COLOR_WHITE" ");
	glassProgram = R_FindProgram ("glass", 0);

	if (glassProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"glass interaction program"S_COLOR_WHITE" ");
	lightGlassProgram = R_FindProgram("glassLight", 0);

	if (lightGlassProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"sprite program"S_COLOR_WHITE" ");
	spriteProgram = R_FindProgram("sprite", 0);

	if (spriteProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"thermal vision program"S_COLOR_WHITE" ");
	thermalProgram = R_FindProgram ("thermal", 0);

	thermalfpProgram = R_FindProgram ("thermalfp", 0);

	if (thermalProgram->valid && thermalfpProgram->valid){
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"water program"S_COLOR_WHITE" ");
	waterProgram = R_FindProgram ("water", 0);
	if (waterProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"particles program"S_COLOR_WHITE" ");
	particlesProgram = R_FindProgram ("particles", 0);

	if (particlesProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"fxaa program"S_COLOR_WHITE" ");
	fxaaProgram = R_FindProgram ("fxaa", 0);

	if (fxaaProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"filmicFx program"S_COLOR_WHITE" ");
	filmicFxProgram = R_FindProgram ("filmicFx", 0);

	if (filmicFxProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}


/*	Com_Printf("Load "S_COLOR_YELLOW"lookup color table program"S_COLOR_WHITE" ");
	lutProgram = R_FindProgram("lut", 0);
	if (lutProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}
*/	
	Com_Printf("Load "S_COLOR_YELLOW"white balance program"S_COLOR_WHITE" ");
	whiteBalanceProgram = R_FindProgram("whitebalance", 0);
	if (whiteBalanceProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}
	Com_Printf("Load "S_COLOR_YELLOW"shadow volumes program"S_COLOR_WHITE" ");
	shadowProgram = R_FindProgram("shadow", 0);
	if (shadowProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"perspective correction program"S_COLOR_WHITE" ");
	fixFovProgram = R_FindProgram("fixfov", 0);
	if (fixFovProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"menu background program"S_COLOR_WHITE" ");
	menuProgram = R_FindProgram("menu", 0);
	if (menuProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"sky program"S_COLOR_WHITE" ");
	skyProgram = R_FindProgram("sky", 0);
	if (skyProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"color program"S_COLOR_WHITE" ");
	colorProgram = R_FindProgram("color", 0);
	if (colorProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"flare program"S_COLOR_WHITE" ");
	flareProgram = R_FindProgram("flare", 0);
	if (flareProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"global fog program"S_COLOR_WHITE" ");
	globalFogProgram = R_FindProgram("globalFog", 0);
	if (globalFogProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"screen flash program"S_COLOR_WHITE" ");
	screenFlashProgram = R_FindProgram("screenFlash", 0);
	if (screenFlashProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"heat haze program"S_COLOR_WHITE" ");
	heatHazeProgram = R_FindProgram("heatHaze", 0);
	if (heatHazeProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"debug TBN program"S_COLOR_WHITE" ");
	tbnDebugProgram = R_FindProgram("tbnDebug", S_GEO);
	if (tbnDebugProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"show tris program"S_COLOR_WHITE" ");
	showTrisProgram = R_FindProgram("showTris", 0);
	if (showTrisProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"tonemap program"S_COLOR_WHITE" ");
	tonemapProgram = R_FindProgram("tonemap", 0);
	if (tonemapProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"final pass program"S_COLOR_WHITE" ");
	finalPassProgram = R_FindProgram("finalPass", 0);
	if (finalPassProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"2d program"S_COLOR_WHITE" ");
	picProgram = R_FindProgram("drawPics", 0);
	if (picProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"gauss compute program"S_COLOR_WHITE" ");
	blur_xComputeProgram = R_FindProgram("gauss_x", S_COMP);
	blur_yComputeProgram = R_FindProgram("gauss_y", S_COMP);
	if (blur_xComputeProgram->valid && blur_yComputeProgram->valid) {
		Com_Printf("succeeded\n");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

#ifdef GLSL_LOADING_TIME
	stop = Sys_Milliseconds ();
	sec = (float)stop - (float)start;
	Com_Printf ("\nGLSL shaders loading time: "S_COLOR_GREEN"%5.4f"S_COLOR_WHITE" sec\n", sec * 0.001);
#endif
	Com_Printf ("\n");
}

/*
=============
R_ShutdownPrograms

=============
*/
void R_ShutdownPrograms (void) {
	glslProgram_t	*program;
	int				i;

	for (i = 0; i < r_numPrograms; i++) {
		program = &r_programs[i];
		qglDeleteProgram (program->id);
		}
	r_numPrograms = 0;
}

/*
=============
R_ListPrograms_f

=============
*/
void R_ListPrograms_f (void) {
	glslProgram_t	*program;
	int			numInvalid = 0;
	int			i;

	Com_Printf ("        permutations name\n");
	Com_Printf ("-------------------------\n");

	for (i = 0; i < r_numPrograms; i++) {
		program = &r_programs[i];
		if (!program->valid)
			numInvalid++;

		Com_Printf ("  %4i: %s%s\n", i, program->name, program->valid ? "" : "(INVALID)");
	}

	Com_Printf ("-------------------\n");
	Com_Printf (" %i programs\n", r_numPrograms);
	Com_Printf ("  %i invalid\n", numInvalid);
}

void R_GLSLinfo_f(void) {
	
	int i;
	GLint j;
	const char *ver;

	ver = (const char*)qglGetString(GL_SHADING_LANGUAGE_VERSION);
	Com_Printf("GLSL Version: "S_COLOR_GREEN"%s\n", ver);

	qglGetIntegerv(GL_NUM_SHADING_LANGUAGE_VERSIONS, &j);
	for (i = 0; i < j; ++i) {
		ver = (const char*)glGetStringi(GL_SHADING_LANGUAGE_VERSION, i);
		if (!ver)
			break;
		Com_Printf(S_COLOR_YELLOW"%s\n", ver);
	}
}

/*
============
GL_BindProgram

============
*/
void GL_BindProgram (glslProgram_t *program) {
	int	id = program->id;

	if (gl_state.programId != id) {
		qglUseProgram (id);
		gl_state.programId = id;
	}
}

