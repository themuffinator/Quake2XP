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

static const char *glslExt =
"#version 450 core\n"
"precision mediump float;\n"
"precision mediump int;\n"
"out vec4 fragData;\n";	// out fragColor

static const char *mathDefs =
"#define	CUTOFF_EPSILON	1.0 / 255.0\n"
"#define	PI				3.14159265358979323846\n"
"#define	HALF_PI			1.57079632679489661923\n"
"#define	SQRT_PI			1.77245385090551602729\n"
"#define	SQRT_THREE		1.73205080756887729352\n"
"#define	INV_PI			(1.0 / PI)\n";

static const char *glslUniforms =
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
;

/*
=================
Com_HashKey

=================
*/
unsigned Com_HashKey (const char *string, unsigned size) {
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
qboolean Q_IsLiteral (const char *text) {
	int		i, c, len;

	len = strlen (text);

	for (i = 0; i < len; i++) {
		c = text[i];

		if ((c < 'a' || c > 'z') && (c < 'A' || c > 'Z') && c != '_')
			return qfalse;
	}

	return qtrue;
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

	hash = Com_HashKey (name, PROGRAM_HASH_SIZE);

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
static void R_GetInfoLog (int id, char *log, qboolean isProgram) {
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
		token = Com_ParseExt (&p, qtrue);
		if (!token[0])
			break;

		if (!strcmp (token, "#include")) {
			int	li;
			char	*buf;

			if (limit < 0)
				Com_Error (ERR_FATAL, "R_LoadIncludes: more than 64 includes");

			token = Com_ParseExt (&p, qfalse);
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
==============
R_CreateProgram

==============
*/

static glslProgram_t *R_CreateProgram (	const char *name, const char *vertexSource, const char *fragmentSource, 
										const char *GeometrySource, const char *tessControlSource, const char *tessEvSource) {
	char			log[MAX_INFO_LOG];
	unsigned		hash;
	glslProgram_t	*program;
	const char		*strings[MAX_PROGRAM_DEFS * 3 + 2];
	int				numStrings;
	int				numLinked = 0;
	int				id, vertexId, fragmentId, tessControlId, tessEvId, geomId;
	int				status;
	int				i;

	if ((vertexSource && strlen (vertexSource) < 17) || (fragmentSource && strlen (fragmentSource) < 17) || (GeometrySource && strlen(GeometrySource) < 17) ||
		(tessControlSource && strlen(tessControlSource) < 17) || (tessEvSource && strlen(tessEvSource) < 17))
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

	numStrings = 0;
	vertexId = 0;
	fragmentId = 0;
	geomId = 0;
	tessControlId = 0;
	tessEvId = 0;

	strings[numStrings++] = glslExt;
	strings[numStrings++] = mathDefs;
	strings[numStrings++] = glslUniforms;

	// compile vertex shader
	if (vertexSource) {
		// link includes
		vertexSource = R_LoadIncludes ((char*)vertexSource);

		strings[numStrings] = vertexSource;
		vertexId = qglCreateShader (GL_VERTEX_SHADER);
		qglShaderSource (vertexId, numStrings + 1, strings, NULL);
		qglCompileShader (vertexId);
		qglGetShaderiv (vertexId, GL_COMPILE_STATUS, &status);

		if (!status) {
			R_GetInfoLog (vertexId, log, qfalse);
			qglDeleteShader (vertexId);
			Com_Printf ("program '%s': error(s) in vertex shader:\n-----------\n%s\n-----------\n", program->name, log);
			return NULL;
		}
	}
		
	// compile geo shaders
	if (GeometrySource) {
		// link includes
		GeometrySource = R_LoadIncludes((char*)GeometrySource);

		strings[numStrings] = GeometrySource;
		geomId = qglCreateShader(GL_GEOMETRY_SHADER);
		qglShaderSource(geomId, numStrings + 1, strings, NULL);
		qglCompileShader(geomId);
		qglGetShaderiv(geomId, GL_COMPILE_STATUS, &status);

		if (!status) {
			R_GetInfoLog(geomId, log, qfalse);
			qglDeleteShader(geomId);
			Com_Printf("program '%s': error(s) in geo shader:\n-----------\n%s\n-----------\n", program->name, log);
			return NULL;
		}
	}

	// compile tess control shaders
	if (tessControlSource) {
		// link includes
		tessControlSource = R_LoadIncludes((char*)tessControlSource);

		strings[numStrings] = tessControlSource;
		tessControlId = qglCreateShader(GL_TESS_CONTROL_SHADER);
		qglShaderSource(tessControlId, numStrings + 1, strings, NULL);
		qglCompileShader(tessControlId);
		qglGetShaderiv(tessControlId, GL_COMPILE_STATUS, &status);

		if (!status) {
			R_GetInfoLog(tessControlId, log, qfalse);
			qglDeleteShader(tessControlId);
			Com_Printf("program '%s': error(s) in tess control shader:\n-----------\n%s\n-----------\n", program->name, log);
			return NULL;
		}
	}

	// compile tess eval shaders
	if (tessEvSource) {
		// link includes
		tessEvSource = R_LoadIncludes((char*)tessEvSource);

		strings[numStrings] = tessEvSource;
		tessEvId = qglCreateShader(GL_TESS_EVALUATION_SHADER);
		qglShaderSource(tessEvId, numStrings + 1, strings, NULL);
		qglCompileShader(tessEvId);
		qglGetShaderiv(tessEvId, GL_COMPILE_STATUS, &status);

		if (!status) {
			R_GetInfoLog(tessEvId, log, qfalse);
			qglDeleteShader(tessEvId);
			Com_Printf("program '%s': error(s) in tess eval shader:\n-----------\n%s\n-----------\n", program->name, log);
			return NULL;
		}
	}

	// compile fragment shader
	if (fragmentSource) {
		// link includes
		fragmentSource = R_LoadIncludes ((char*)fragmentSource);
		strings[numStrings] = fragmentSource;
		fragmentId = qglCreateShader (GL_FRAGMENT_SHADER);

//		Com_Printf("program '%s': warning(s) in: %s\n", program->name, log); // debug depricated func

		qglShaderSource (fragmentId, numStrings + 1, strings, NULL);
		qglCompileShader (fragmentId);
		qglGetShaderiv (fragmentId, GL_COMPILE_STATUS, &status);

		if (!status) {
			R_GetInfoLog (fragmentId, log, qfalse);
			qglDeleteShader (fragmentId);
			Com_Printf ("program '%s': error(s) in fragment shader:\n-----------\n%s\n-----------\n", program->name, log);
			return NULL;
		}
	}

	//
	// link the program
	//

	id = qglCreateProgram ();

	if (vertexId) {
		qglAttachShader (id, vertexId);
		qglDeleteShader (vertexId);
	}

	if (fragmentId) {
		qglAttachShader (id, fragmentId);
		qglDeleteShader (fragmentId);
	}
		
	if (geomId) {
		qglAttachShader(id, geomId);
		qglDeleteShader(geomId);
	}

	if (tessControlId) {
		qglAttachShader(id, tessControlId);
		qglDeleteShader(tessControlId);
	}

	if (tessEvId) {
		qglAttachShader(id, tessEvId);
		qglDeleteShader(tessEvId);
	}

	qglLinkProgram (id);
	qglGetProgramiv (id, GL_LINK_STATUS, &status);

	R_GetInfoLog (id, log, qtrue);

	if (!status) {
		qglDeleteProgram (id);
		Com_Printf ("program '%s': link error(s): %s\n", program->name, log);
		return NULL;
	}

	// don't let it be slow (crap)
	if (strstr (log, "fragment shader will run in software")) {
		qglDeleteProgram (id);
		Com_Printf ("program '%s': refusing to perform software emulation\n", program->name);
		return NULL;
	}

	program->id = id;
	program->valid = qtrue;

	// add to the hash
	hash = Com_HashKey (program->name, PROGRAM_HASH_SIZE);
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

glslProgram_t *R_FindProgram (const char *name, qboolean vertex, qboolean fragment, qboolean geo, qboolean tess, qboolean tessEv) {
	char			filename[MAX_QPATH];
	char			newname[MAX_QPATH];
	glslProgram_t	*program;
	char			*vertexSource = NULL, *fragmentSource = NULL, *geoSource = NULL, *tessSource = NULL, *evalSource = NULL;

	if (!vertex && !fragment)
		return &r_nullProgram;


	Q_snprintfz (newname, sizeof(newname), "%s%s", name, !vertex ? "(fragment)" : !fragment ? "(vertex)" : "");

	program = R_ProgramForName (newname);
	
	if (program)
		return program;

	if (vertex) {
		Q_snprintfz (filename, sizeof(filename), "glsl/%s.vp", name);
		FS_LoadFile (filename, (void **)&vertexSource);
	}

	if (fragment) {
		Q_snprintfz (filename, sizeof(filename), "glsl/%s.fp", name);
		FS_LoadFile (filename, (void **)&fragmentSource);
	}

	if (geo) {
		Q_snprintfz(filename, sizeof(filename), "glsl/%s.geo", name);
		FS_LoadFile(filename, (void **)&geoSource);
	}
	
	if (tess) {
		Q_snprintfz(filename, sizeof(filename), "glsl/%s.tess", name);
		FS_LoadFile(filename, (void **)&tessSource);
	}
	
	if (tessEv) {
		Q_snprintfz(filename, sizeof(filename), "glsl/%s.eval", name);
		FS_LoadFile(filename, (void **)&evalSource);
	}

	if (!vertexSource && !fragmentSource)
		return &r_nullProgram;		// no appropriate shaders found

	program = R_CreateProgram (newname, vertexSource, fragmentSource, geoSource, tessSource, evalSource);

	if (vertexSource)
		FS_FreeFile (vertexSource);
	if (fragmentSource)
		FS_FreeFile (fragmentSource);

	if (!program || !program->valid)
		return &r_nullProgram;

	return program;
}

/*
=============
R_InitPrograms

=============
*/


void R_InitPrograms (void) {
	int			missing = 0,  id;

	Com_Printf ("\nInitializing programs...\n\n");

#ifdef GLSL_LOADING_TIME
	int		start = 0, stop = 0;
	float	sec;
	start = Sys_Milliseconds ();
#endif
	 
	memset (programHashTable, 0, sizeof(programHashTable));
	memset (&r_nullProgram, 0, sizeof(glslProgram_t));

	Com_Printf ("Load "S_COLOR_YELLOW"null program"S_COLOR_WHITE" ");
	nullProgram = R_FindProgram ("null", qtrue, qtrue, qfalse, qfalse, qfalse);
	if (nullProgram->valid) {
		Com_Printf ("succeeded\n");
		id = nullProgram->id;
		null_mvp = qglGetUniformLocation(id, "u_modelViewProjectionMatrix");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"ambient world program"S_COLOR_WHITE" ");
	ambientWorldProgram = R_FindProgram ("ambientWorld", qtrue, qtrue, qfalse, qfalse, qfalse);
	if (ambientWorldProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"light world program"S_COLOR_WHITE" ");
	lightWorldProgram = R_FindProgram ("lightWorld", qtrue, qtrue, qfalse, qfalse, qfalse);
	if (lightWorldProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"ambient md2 program"S_COLOR_WHITE" ");
	aliasAmbientProgram = R_FindProgram ("ambientMd2", qtrue, qtrue, qfalse, qfalse, qfalse);
	if (aliasAmbientProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"ambient md3 program"S_COLOR_WHITE" ");
	md3AmbientProgram = R_FindProgram("ambientMd3", qtrue, qtrue, qfalse, qfalse, qfalse);
	if (md3AmbientProgram->valid) {
		Com_Printf("succeeded\n");

	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"light model program"S_COLOR_WHITE" ");
	aliasBumpProgram = R_FindProgram ("lightAlias", qtrue, qtrue, qfalse, qfalse, qfalse);

	if (aliasBumpProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"gauss blur program"S_COLOR_WHITE" ");
	gaussXProgram = R_FindProgram ("gaussX", qtrue, qtrue, qfalse, qfalse, qfalse);
	gaussYProgram = R_FindProgram ("gaussY", qtrue, qtrue, qfalse, qfalse, qfalse);
	

	if (gaussXProgram->valid && gaussYProgram->valid){
		Com_Printf("succeeded\n");

		id = gaussXProgram->id;
		gaussx_matrix	= qglGetUniformLocation(id, "u_orthoMatrix");

		id = gaussYProgram->id;
		gaussy_matrix	= qglGetUniformLocation(id, "u_orthoMatrix");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"glare program"S_COLOR_WHITE" ");
	glareProgram = R_FindProgram ("glare", qtrue, qtrue, qfalse, qfalse, qfalse);

	if (glareProgram->valid){
		Com_Printf("succeeded\n");

		id = glareProgram->id;
		glare_params		= qglGetUniformLocation(id, "u_glareParams");
		glare_matrix		= qglGetUniformLocation(id, "u_orthoMatrix");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}


	Com_Printf ("Load "S_COLOR_YELLOW"radial blur program"S_COLOR_WHITE" ");
	radialProgram = R_FindProgram ("radialBlur", qtrue, qtrue, qfalse, qfalse, qfalse);

	if (radialProgram->valid){
		Com_Printf("succeeded\n");
		
		id = radialProgram->id;
		rb_params	= qglGetUniformLocation(id, "u_radialBlurParams");
		rb_matrix	= qglGetUniformLocation(id, "u_orthoMatrix");
		rb_cont		= qglGetUniformLocation(id, "u_cont");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"dof blur program"S_COLOR_WHITE" ");
	dofProgram = R_FindProgram ("dof", qtrue, qtrue, qfalse, qfalse, qfalse);

	if (dofProgram->valid){
		Com_Printf("succeeded\n");

		id = dofProgram->id;
		dof_screenSize	= qglGetUniformLocation(id, "u_screenSize");
		dof_params		= qglGetUniformLocation(id, "u_dofParams");
		dof_orthoMatrix = qglGetUniformLocation(id, "u_orthoMatrix");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"motion blur program"S_COLOR_WHITE" ");
	motionBlurProgram = R_FindProgram ("mblur", qtrue, qtrue, qfalse, qfalse, qfalse);
	
	if (motionBlurProgram->valid){
		Com_Printf("succeeded\n");

		id = motionBlurProgram->id;
		mb_params		= qglGetUniformLocation(id, "u_params");
		mb_orthoMatrix	= qglGetUniformLocation(id, "u_orthoMatrix");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"ssao program"S_COLOR_WHITE" ");
	ssaoProgram = R_FindProgram ("ssao", qtrue, qtrue, qfalse, qfalse, qfalse);
	depthDownsampleProgram = R_FindProgram("depthDownsample", qtrue, qtrue, qfalse, qfalse, qfalse);
	ssaoBlurProgram = R_FindProgram("ssaoBlur", qtrue, qtrue, qfalse, qfalse, qfalse);

	if (ssaoProgram->valid && depthDownsampleProgram->valid && ssaoBlurProgram->valid){
		Com_Printf("succeeded\n");

		id = depthDownsampleProgram->id;
		depthDS_params		= qglGetUniformLocation(id, "u_depthParms");
		depthDS_orthoMatrix = qglGetUniformLocation(id, "u_orthoMatrix");

		id = ssaoProgram->id;
		ssao_params			= qglGetUniformLocation(id, "u_ssaoParms");
		ssao_vp				= qglGetUniformLocation(id, "u_viewport");
		ssao_orthoMatrix	= qglGetUniformLocation(id, "u_orthoMatrix");

		id = ssaoBlurProgram->id;
		ssaoB_sapmles		= qglGetUniformLocation(id, "u_numSamples");
		ssaoB_axisMask		= qglGetUniformLocation(id, "u_axisMask");
		ssaoB_orthoMatrix	= qglGetUniformLocation(id, "u_orthoMatrix");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"bloom program"S_COLOR_WHITE" ");
	bloomdsProgram = R_FindProgram ("bloomds", qtrue, qtrue, qfalse, qfalse, qfalse);
	bloomfpProgram = R_FindProgram ("bloomfp", qtrue, qtrue, qfalse, qfalse, qfalse);

	if (bloomdsProgram->valid && bloomfpProgram->valid){
		Com_Printf("succeeded\n");

		id = bloomdsProgram->id;
		bloomDS_threshold	= qglGetUniformLocation(id, "u_BloomThreshold");
		bloomDS_matrix		= qglGetUniformLocation(id, "u_orthoMatrix");

		id = bloomfpProgram->id;
		bloomFP_params	= qglGetUniformLocation(id, "u_bloomParams");
		bloom_FP_matrix = qglGetUniformLocation(id, "u_orthoMatrix");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"refraction program"S_COLOR_WHITE" ");
	refractProgram = R_FindProgram ("refract", qtrue, qtrue, qfalse, qfalse, qfalse);

	if (refractProgram->valid) {
		Com_Printf ("succeeded\n");
		id = refractProgram->id;
		ref_deformMul		= qglGetUniformLocation(id, "u_deformMul");
		ref_mvp				= qglGetUniformLocation(id, "u_modelViewProjectionMatrix");
		ref_mvm				= qglGetUniformLocation(id, "u_modelViewMatrix");
		ref_pm				= qglGetUniformLocation(id, "u_projectionMatrix");

		ref_alpha			= qglGetUniformLocation(id, "u_alpha");
		ref_thickness		= qglGetUniformLocation(id, "u_thickness");
		ref_thickness2		= qglGetUniformLocation(id, "u_thickness2");
		ref_viewport		= qglGetUniformLocation(id, "u_viewport");
		ref_depthParams		= qglGetUniformLocation(id, "u_depthParms");
		ref_ambientScale	= qglGetUniformLocation(id, "u_ambientScale");
		ref_mask			= qglGetUniformLocation(id, "u_mask");
		ref_alphaMask		= qglGetUniformLocation(id, "u_ALPHAMASK");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"thermal vision program"S_COLOR_WHITE" ");
	thermalProgram = R_FindProgram ("thermal", qtrue, qtrue, qfalse, qfalse, qfalse);

	thermalfpProgram = R_FindProgram ("thermalfp", qtrue, qtrue, qfalse, qfalse, qfalse);

	if (thermalProgram->valid && thermalfpProgram->valid){
		Com_Printf("succeeded\n");
		
		id = thermalProgram->id;
		therm_matrix	= qglGetUniformLocation(id, "u_orthoMatrix");

		id = thermalfpProgram->id;
		thermf_matrix	= qglGetUniformLocation(id, "u_orthoMatrix");

	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"water program"S_COLOR_WHITE" ");
	waterProgram = R_FindProgram ("water", qtrue, qtrue, qfalse, qfalse, qfalse);
	if (waterProgram->valid) {
		Com_Printf ("succeeded\n");
		id = waterProgram->id;

		water_deformMul		= qglGetUniformLocation (id, "u_deformMul");
		water_thickness		= qglGetUniformLocation (id, "u_thickness");
		water_screenSize	= qglGetUniformLocation (id, "u_viewport");
		water_depthParams	= qglGetUniformLocation (id, "u_depthParms");
		water_colorModulate = qglGetUniformLocation (id, "u_ColorModulate");
		water_ambient		= qglGetUniformLocation (id, "u_ambientScale");
		water_trans			= qglGetUniformLocation (id, "u_TRANS");
		water_entity2world	= qglGetUniformLocation (id, "g_entityToWorldRot");
		water_mvp			= qglGetUniformLocation	(id, "u_modelViewProjectionMatrix");
		water_mv			= qglGetUniformLocation	(id, "u_modelViewMatrix");
		water_pm			= qglGetUniformLocation	(id, "u_projectionMatrix");
		water_mirror		= qglGetUniformLocation (id, "u_mirror");

	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"particles program"S_COLOR_WHITE" ");
	particlesProgram = R_FindProgram ("particles", qtrue, qtrue, qfalse, qfalse, qfalse);

	if (particlesProgram->valid) {
		Com_Printf ("succeeded\n");
		id = particlesProgram->id;

		particle_depthParams	= qglGetUniformLocation (id, "u_depthParms");
		particle_mask			= qglGetUniformLocation (id, "u_mask");
		particle_thickness		= qglGetUniformLocation (id, "u_thickness");
		particle_colorModulate	= qglGetUniformLocation (id, "u_colorScale");
		particle_mvp			= qglGetUniformLocation (id, "u_modelViewProjectionMatrix");
		particle_mv				= qglGetUniformLocation (id, "u_modelViewMatrix");
		particle_projMat		= qglGetUniformLocation	(id, "u_projectionMatrix");
		particle_texRotMat		= qglGetUniformLocation (id, "u_texRotateMatrix");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}
	
	Com_Printf ("Load "S_COLOR_YELLOW"generic program"S_COLOR_WHITE" ");
	genericProgram = R_FindProgram ("generic", qtrue, qtrue, qfalse, qfalse, qfalse);

	if (genericProgram->valid) {
		Com_Printf ("succeeded\n");

		id = genericProgram->id;
		gen_attribConsole	= qglGetUniformLocation (id, "u_ATTRIB_CONSOLE");
		gen_attribColors	= qglGetUniformLocation (id, "u_ATTRIB_COLORS");
		gen_colorModulate	= qglGetUniformLocation (id, "u_colorScale");
		gen_color			= qglGetUniformLocation (id, "u_color");
		gen_sky				= qglGetUniformLocation (id, "u_isSky");
		gen_mvp				= qglGetUniformLocation (id, "u_modelViewProjectionMatrix");
		gen_orthoMatrix		= qglGetUniformLocation (id, "u_orthoMatrix");
		gen_3d				= qglGetUniformLocation (id, "u_3d");
		gen_light			= qglGetUniformLocation (id, "u_lightShift");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"cinematic program"S_COLOR_WHITE" ");
	cinProgram	= R_FindProgram ("cin", qtrue, qtrue, qfalse, qfalse, qfalse);

	if (cinProgram->valid) {
		Com_Printf ("succeeded\n");
		id = cinProgram->id;
		cin_params		= qglGetUniformLocation(id, "u_cinParams");
		cin_orthoMatrix = qglGetUniformLocation(id, "u_orthoMatrix");

	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"load screen program"S_COLOR_WHITE" ");
	loadingProgram = R_FindProgram ("loading", qtrue, qtrue, qfalse, qfalse, qfalse);

	if (loadingProgram->valid) {
		Com_Printf ("succeeded\n");
		id = loadingProgram->id;
		ls_fade = qglGetUniformLocation(id, "u_colorScale");
		ls_orthoMatrix = qglGetUniformLocation(id, "u_orthoMatrix");

	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"fxaa program"S_COLOR_WHITE" ");
	fxaaProgram = R_FindProgram ("fxaa", qtrue, qtrue, qfalse, qfalse, qfalse);

	if (fxaaProgram->valid) {
		Com_Printf ("succeeded\n");
		id = fxaaProgram->id;
		fxaa_screenSize		= qglGetUniformLocation(id, "u_ScreenSize");
		fxaa_orthoMatrix	= qglGetUniformLocation(id, "u_orthoMatrix");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"film grain program"S_COLOR_WHITE" ");
	filmGrainProgram = R_FindProgram ("filmGrain", qtrue, qtrue, qfalse, qfalse, qfalse);
	id = filmGrainProgram->id;

	film_screenRes	= qglGetUniformLocation(id, "u_screenSize");
	film_rand		= qglGetUniformLocation(id, "u_rand");
	film_frameTime	= qglGetUniformLocation(id, "u_time");
	film_params		= qglGetUniformLocation(id, "u_params");
	film_matrix		= qglGetUniformLocation(id, "u_orthoMatrix");

	if (filmGrainProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"gammaramp program"S_COLOR_WHITE" ");
	gammaProgram = R_FindProgram ("gamma", qtrue, qtrue, qfalse, qfalse, qfalse);
	if (gammaProgram->valid) {
		Com_Printf ("succeeded\n");
		id = gammaProgram->id;
		gamma_control		= qglGetUniformLocation (id, "u_control");
		gamma_orthoMatrix	= qglGetUniformLocation (id, "u_orthoMatrix");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}


	Com_Printf("Load "S_COLOR_YELLOW"shadow volumes program"S_COLOR_WHITE" ");
	shadowProgram = R_FindProgram("shadow", qtrue, qtrue, qfalse, qfalse, qfalse);
	if (shadowProgram->valid) {
		Com_Printf("succeeded\n");
		id = shadowProgram->id;
		sv_mvp = qglGetUniformLocation(id, "u_modelViewProjectionMatrix");
		sv_lightOrg = qglGetUniformLocation(id, "u_lightOrg");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"light2d program"S_COLOR_WHITE" ");
	light2dProgram = R_FindProgram("light2d", qtrue, qtrue, qfalse, qfalse, qfalse);
	if (light2dProgram->valid) {
		Com_Printf("succeeded\n");
		id = light2dProgram->id;
		light2d_orthoMatrix = qglGetUniformLocation(id, "u_orthoMatrix");
		light2d_params = qglGetUniformLocation(id, "u_params");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"perspective correction program"S_COLOR_WHITE" ");
	fixFovProgram = R_FindProgram("fixfov", qtrue, qtrue, qfalse, qfalse, qfalse);
	if (fixFovProgram->valid) {
		Com_Printf("succeeded\n");
		id = fixFovProgram->id;
		fixfov_orthoMatrix = qglGetUniformLocation(id, "u_orthoMatrix");
		fixfov_params = qglGetUniformLocation(id, "u_params");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"menu background program"S_COLOR_WHITE" ");
	menuProgram = R_FindProgram("menu", qtrue, qtrue, qfalse, qfalse, qfalse);
	if (menuProgram->valid) {
		Com_Printf("succeeded\n");
		id = menuProgram->id;
		menu_orthoMatrix = qglGetUniformLocation(id, "u_orthoMatrix");
		menu_params = qglGetUniformLocation(id, "u_screenSize");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}
/*
	Com_Printf("Load "S_COLOR_YELLOW"fbo program"S_COLOR_WHITE" ");
	fbo2screenProgram = R_FindProgram("fbo2screen", qtrue, qtrue, qfalse, qfalse, qfalse);
	if (fbo2screenProgram->valid) {
		Com_Printf("succeeded\n");
		id = fbo2screenProgram->id;
		fbo2screen_orthoMatrix = qglGetUniformLocation(id, "u_orthoMatrix");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}
*/

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

