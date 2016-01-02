/*
==============================

SHADING LANGUAGE INTERFACE

==============================
*/
#include "r_local.h"
#ifdef _WIN32
#include <process.h>
#endif

#define MAX_INFO_LOG		4096

#define	PROGRAM_HASH_SIZE	MAX_PROGRAMS

static glslProgram_t		*programHashTable[PROGRAM_HASH_SIZE];
int r_numPrograms;
static glslProgram_t	r_nullProgram;

static const char *shader4 =
"#extension GL_EXT_gpu_shader4 : enable\n";

static const char *shader5 =
"#extension GL_ARB_gpu_shader5 : enable\n";

static const char *baseExt =
"#extension GL_ARB_texture_rectangle : enable\n"
"out vec4 fragData;\n";

static const char *glslVersion =
"#version 150\n"
"#extension GL_ARB_explicit_attrib_location  : enable\n"
"#extension GL_ARB_separate_shader_objects : enable\n";
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
R_GetProgramDefBits

==============
*/
unsigned R_GetProgramDefBits (glslProgram_t *program, const char *name) {
	int		i;

	if (!program || !program->valid)
		return 0;

	for (i = 0; i < program->numDefs; i++) {
		if (!strcmp (program->defStrings[i], name))
			return program->defBits[i];
	}

	return 0;
}

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
R_ParseDefs

==============
*/
static void R_ParseDefs (glslProgram_t *program, const char *text) {
	parser_t	parser;
	token_t		token;

	Parser_Reset (&parser, va ("program '%s'", program->name), text);

	while (1) {
		if (!Parser_GetToken (&parser, &token))
			return;

		if (!Q_IsLiteral (token.text)) {
			Parser_Warning (&parser, "def '%s' is non-literal\n", token.text);
			return;
		}

		if (program->numDefs == MAX_PROGRAM_DEFS) {
			Parser_Warning (&parser, "MAX_PROGRAM_DEFS hit\n");
			return;
		}

		program->defBits[program->numDefs] = BIT (program->numDefs);
		Q_strncpyz (program->defStrings[program->numDefs], token.text, MAX_DEF_NAME);

		program->numDefs++;
	}
}

/*
==============
R_LoadIncludes

Search shader texts for '#include' directives
and insert included file contents.
==============
*/
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
			glsl = (char*)Q_malloc (l + li + 2);
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

static glslProgram_t *R_CreateProgram (const char *name, const char *defs, const char *vertexSource, const char *fragmentSource) {
	char			log[MAX_INFO_LOG];
	unsigned		hash;
	glslProgram_t	*program;
	const char		*strings[MAX_PROGRAM_DEFS * 3 + 2];
	int				numStrings;
	int				numLinked = 0;
	int				id, vertexId, fragmentId;
	int				status;
	int				i, j;
	int				lineno = 1;

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

	R_ParseDefs (program, defs);

	program->numId = BIT (program->numDefs);

	for (i = 0; i < program->numId; i++) {

		char *defines = NULL;		/// Berserker's fix
		numStrings = 0;
		vertexId = 0;
		fragmentId = 0;

		/// Berserker's fix start
		if (program->numDefs) {
			int len = 0;
			// посчитаем требуемый объём памяти по дифайны
			for (j = 0; j < program->numDefs; j++)
			if (i & program->defBits[j])
				len += 8 + strlen (program->defStrings[j]) + 1; // 8 = strlen("#define "), 1 = strlen("\n")

			len++; // for trailing NULL
			defines = (char*)calloc (len, 1); // calloc = malloc + memclear  ;)
			for (j = 0; j < program->numDefs; j++) {
				if (i & program->defBits[j]) {
					Q_strcat (defines, "#define ", len);
					Q_strcat (defines, program->defStrings[j], len);
					Q_strcat (defines, "\n", len);
				}
			}
			strings[numStrings++] = defines;
		}
		/// Berserker's fix end
	
		strings[numStrings++] = glslVersion; //force version if needit

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
				continue;
			}
		}

		// add ext
		if (gl_state.shader5)
			strings[numStrings++] = shader5;
		else
			strings[numStrings++] = shader4;

		strings[numStrings++] = baseExt;

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
				continue;
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

		qglLinkProgram (id);
		qglGetProgramiv (id, GL_LINK_STATUS, &status);

		R_GetInfoLog (id, log, qtrue);

		if (!status) {
			qglDeleteProgram (id);
			Com_Printf ("program '%s': link error(s): %s\n", program->name, log);
			continue;
		}

		// don't let it be slow (crap)
		if (strstr (log, "fragment shader will run in software")) {
			qglDeleteProgram (id);
			Com_Printf ("program '%s': refusing to perform software emulation\n", program->name);
			continue;
		}

		// TODO: glValidateProgram?
		/// Berserker's fix start
		if (defines)
			free (defines);
		/// Berserker's fix end

		program->id[i] = id;
		numLinked++;
	}

	if (numLinked == program->numId)
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

glslProgram_t *R_FindProgram (const char *name, qboolean vertex, qboolean fragment) {
	char			filename[MAX_QPATH];
	char			newname[MAX_QPATH];
	glslProgram_t	*program;
	char			*defs, *vertexSource = NULL, *fragmentSource = NULL;

	if (!vertex && !fragment)
		return &r_nullProgram;


	Q_snprintfz (newname, sizeof(newname), "%s%s", name, !vertex ? "(fragment)" : !fragment ? "(vertex)" : "");

	program = R_ProgramForName (newname);
	if (program)
		return program;

	Q_snprintfz (filename, sizeof(filename), "glsl/%s.defs", name);
	FS_LoadFile (filename, (void **)&defs);

	if (vertex) {
		Q_snprintfz (filename, sizeof(filename), "glsl/%s.vp", name);
		FS_LoadFile (filename, (void **)&vertexSource);
	}

	if (fragment) {
		Q_snprintfz (filename, sizeof(filename), "glsl/%s.fp", name);
		FS_LoadFile (filename, (void **)&fragmentSource);
	}

	if (!vertexSource && !fragmentSource)
		return &r_nullProgram;		// no appropriate shaders found

	program = R_CreateProgram (newname, defs, vertexSource, fragmentSource);


	if (defs)
		FS_FreeFile (defs);
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
	int			missing = 0, start = 0, stop = 0, id;
	float		sec;

	Com_Printf ("\nInitializing programs...\n\n");

	start = Sys_Milliseconds ();

	memset (programHashTable, 0, sizeof(programHashTable));
	memset (&r_nullProgram, 0, sizeof(glslProgram_t));

	Com_Printf ("Load "S_COLOR_YELLOW"null program"S_COLOR_WHITE" ");
	nullProgram = R_FindProgram ("null", qtrue, qtrue);
	if (nullProgram->valid) {
		Com_Printf ("succeeded\n");
		id = nullProgram->id[0];
		null_mvp = qglGetUniformLocation(id, "u_modelViewProjectionMatrix");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"ambient world program"S_COLOR_WHITE" ");
	ambientWorldProgram = R_FindProgram ("ambientWorld", qtrue, qtrue);
	if (ambientWorldProgram->valid) {
		Com_Printf ("succeeded\n");
		id = ambientWorldProgram->id[0];

		ambientWorld_diffuse		= qglGetUniformLocation (id, "u_Diffuse");
		ambientWorld_add			= qglGetUniformLocation (id, "u_Add");
		ambientWorld_lightmap[0]	= qglGetUniformLocation (id, "u_LightMap0");
		ambientWorld_lightmap[1]	= qglGetUniformLocation (id, "u_LightMap1");
		ambientWorld_lightmap[2]	= qglGetUniformLocation (id, "u_LightMap2");
		ambientWorld_lightmapType	= qglGetUniformLocation (id, "u_LightMapType");
		ambientWorld_normalmap		= qglGetUniformLocation (id, "u_NormalMap");
		ambientWorld_ssao			= qglGetUniformLocation (id, "u_ssao");
		ambientWorld_ssaoMap		= qglGetUniformLocation (id, "u_ssaoMap");
		ambientWorld_parallaxParams = qglGetUniformLocation (id, "u_parallaxParams");
		ambientWorld_colorScale		= qglGetUniformLocation (id, "u_ColorModulate");
		ambientWorld_specularScale	= qglGetUniformLocation (id, "u_specularScale");
		ambientWorld_specularExp	= qglGetUniformLocation (id, "u_specularExp");
		ambientWorld_viewOrigin		= qglGetUniformLocation (id, "u_viewOriginES");
		ambientWorld_parallaxType	= qglGetUniformLocation (id, "u_parallaxType");
		ambientWorld_ambientLevel	= qglGetUniformLocation (id, "u_ambientScale");
		ambientWorld_scroll			= qglGetUniformLocation (id, "u_scroll");
		ambientWorld_mvp			= qglGetUniformLocation (id, "u_modelViewProjectionMatrix");

	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"light world program"S_COLOR_WHITE" ");
	lightWorldProgram = R_FindProgram ("lightWorld", qtrue, qtrue);
	if (lightWorldProgram->valid) {
		Com_Printf ("succeeded\n");
		id = lightWorldProgram->id[0];

		lightWorld_diffuse			= qglGetUniformLocation (id, "u_Diffuse");
		lightWorld_normal			= qglGetUniformLocation (id, "u_NormalMap");
		lightWorld_cube				= qglGetUniformLocation (id, "u_CubeFilterMap");
		lightWorld_atten			= qglGetUniformLocation (id, "u_attenMap");
		lightWorld_caustic			= qglGetUniformLocation (id, "u_Caustics");

		lightWorld_parallaxParams	= qglGetUniformLocation (id, "u_parallaxParams");
		lightWorld_colorScale		= qglGetUniformLocation (id, "u_ColorModulate");
		lightWorld_viewOrigin		= qglGetUniformLocation (id, "u_viewOriginES");
		lightWorld_parallaxType		= qglGetUniformLocation (id, "u_parallaxType");

		lightWorld_lightOrigin		= qglGetUniformLocation (id, "u_LightOrg");
		lightWorld_lightColor		= qglGetUniformLocation (id, "u_LightColor");
		lightWorld_fog				= qglGetUniformLocation (id, "u_fog");
		lightWorld_fogDensity		= qglGetUniformLocation (id, "u_fogDensity");

		lightWorld_causticsIntens	= qglGetUniformLocation (id, "u_CausticsModulate");
		lightWorld_caustics			= qglGetUniformLocation (id, "u_isCaustics");

		lightWorld_specularScale	= qglGetUniformLocation (id, "u_specularScale");
		lightWorld_specularExp		= qglGetUniformLocation (id, "u_specularExp");
		lightWorld_ambient			= qglGetUniformLocation (id, "u_isAmbient");
		lightWorld_attenMatrix		= qglGetUniformLocation (id, "u_attenMatrix");
		lightWorld_cubeMatrix		= qglGetUniformLocation (id, "u_cubeMatrix");
		lightWorld_scroll			= qglGetUniformLocation (id, "u_scroll");
		lightWorld_mvp				= qglGetUniformLocation	(id, "u_modelViewProjectionMatrix");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"ambient model program"S_COLOR_WHITE" ");
	aliasAmbientProgram = R_FindProgram ("ambientAlias", qtrue, qtrue);
	if (aliasAmbientProgram->valid) {
		Com_Printf ("succeeded\n");
		id = aliasAmbientProgram->id[0];

		ambientAlias_diffuse		= qglGetUniformLocation (id, "u_Diffuse");
		ambientAlias_normalmap		= qglGetUniformLocation (id, "u_NormalMap");
		ambientAlias_add			= qglGetUniformLocation (id, "u_Add");
		ambientAlias_env			= qglGetUniformLocation (id, "u_env");
		ambientAlias_isEnvMaping	= qglGetUniformLocation (id, "u_isEnvMap");
		ambientAlias_ssao			= qglGetUniformLocation (id, "u_ssao");
		ambientAlias_ssaoMap		= qglGetUniformLocation (id, "u_ssaoMap");
		ambientAlias_colorModulate	= qglGetUniformLocation (id, "u_ColorModulate");
		ambientAlias_addShift		= qglGetUniformLocation (id, "u_AddShift");
		ambientAlias_envScale		= qglGetUniformLocation (id, "u_envScale");
		ambientAlias_isShell		= qglGetUniformLocation (id, "u_isShell");
		ambientAlias_scroll			= qglGetUniformLocation (id, "u_scroll");
		ambientAlias_mvp			= qglGetUniformLocation (id, "u_modelViewProjectionMatrix");
		ambientAlias_viewOrg		= qglGetUniformLocation (id, "u_viewOrg");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}


	Com_Printf ("Load "S_COLOR_YELLOW"light model program"S_COLOR_WHITE" ");
	aliasBumpProgram = R_FindProgram ("lightAlias", qtrue, qtrue);

	if (aliasBumpProgram->valid) {
		Com_Printf ("succeeded\n");
		id = aliasBumpProgram->id[0];

		lightAlias_diffuse	= qglGetUniformLocation (id, "u_diffuseMap");
		lightAlias_normal	= qglGetUniformLocation (id, "u_bumpMap");
		lightAlias_cube		= qglGetUniformLocation (id, "u_CubeFilterMap");
		lightAlias_atten	= qglGetUniformLocation (id, "u_attenMap");
		lightAlias_caustic	= qglGetUniformLocation (id, "u_causticMap");

		lightAlias_viewOrigin		= qglGetUniformLocation (id, "u_ViewOrigin");
		lightAlias_lightOrigin		= qglGetUniformLocation (id, "u_LightOrg");
		lightAlias_lightColor		= qglGetUniformLocation (id, "u_LightColor");
		lightAlias_fog				= qglGetUniformLocation (id, "u_fog");
		lightAlias_fogDensity		= qglGetUniformLocation (id, "u_fogDensity");
		lightAlias_causticsIntens	= qglGetUniformLocation (id, "u_CausticsModulate");
		lightAlias_isCaustics		= qglGetUniformLocation (id, "u_isCaustics");
		lightAlias_specularScale	= qglGetUniformLocation (id, "u_specularScale");
		lightAlias_specularExp		= qglGetUniformLocation (id, "u_specularExp");
		lightAlias_ambient			= qglGetUniformLocation (id, "u_isAmbient");
		lightAlias_attenMatrix		= qglGetUniformLocation (id, "u_attenMatrix");
		lightAlias_cubeMatrix		= qglGetUniformLocation (id, "u_cubeMatrix");
		lightAlias_mvp				= qglGetUniformLocation	(id, "u_modelViewProjectionMatrix");
		lightAlias_mv				= qglGetUniformLocation(id, "u_modelViewMatrix");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"gauss blur program"S_COLOR_WHITE" ");
	gaussXProgram = R_FindProgram ("gaussX", qtrue, qtrue);
	gaussYProgram = R_FindProgram ("gaussY", qtrue, qtrue);
	

	if (gaussXProgram->valid && gaussYProgram->valid){
		Com_Printf("succeeded\n");

		id = gaussXProgram->id[0];
		gaussx_tex		= qglGetUniformLocation(id, "u_map");
		gaussx_matrix	= qglGetUniformLocation(id, "u_orthoMatrix");

		id = gaussYProgram->id[0];
		gaussy_tex		= qglGetUniformLocation(id, "u_map");
		gaussy_matrix	= qglGetUniformLocation(id, "u_orthoMatrix");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"star blur program"S_COLOR_WHITE" ");
	blurStarProgram = R_FindProgram ("blurStar", qtrue, qtrue);

	if (blurStarProgram){
		Com_Printf("succeeded\n");

		id = blurStarProgram->id[0];
		star_tex		= qglGetUniformLocation(id, "u_map");
		star_intens		= qglGetUniformLocation(id, "u_starIntens");
		star_matrix		= qglGetUniformLocation(id, "u_orthoMatrix");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}


	Com_Printf ("Load "S_COLOR_YELLOW"radial blur program"S_COLOR_WHITE" ");
	radialProgram = R_FindProgram ("radialBlur", qtrue, qtrue);

	if (radialProgram->valid){
		Com_Printf("succeeded\n");
		
		id = radialProgram->id[0];
		rb_tex		= qglGetUniformLocation(id, "u_screenMap");
		rb_params	= qglGetUniformLocation(id, "u_radialBlurParams");
		rb_matrix	= qglGetUniformLocation(id, "u_orthoMatrix");
		rb_cont		= qglGetUniformLocation(id, "u_cont");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"dof blur program"S_COLOR_WHITE" ");
	dofProgram = R_FindProgram ("dof", qtrue, qtrue);

	if (dofProgram->valid){
		Com_Printf("succeeded\n");

		id = dofProgram->id[0];
		dof_screenSize	= qglGetUniformLocation(id, "u_screenSize");
		dof_params		= qglGetUniformLocation(id, "u_dofParams");
		dof_tex			= qglGetUniformLocation(id, "u_ScreenTex");
		dof_depth		= qglGetUniformLocation(id, "u_DepthTex");
		dof_matrix		= qglGetUniformLocation(id, "u_orthoMatrix");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"motion blur program"S_COLOR_WHITE" ");
	motionBlurProgram = R_FindProgram ("mblur", qtrue, qtrue);
	
	if (motionBlurProgram->valid){
		Com_Printf("succeeded\n");

		id = motionBlurProgram->id[0];
		mb_params	= qglGetUniformLocation(id, "u_params");
		mb_matrix	= qglGetUniformLocation(id, "u_orthoMatrix");
		mb_tex		= qglGetUniformLocation(id, "u_ScreenTex");
		mb_mask		= qglGetUniformLocation(id, "u_MaskTex");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"ssao program"S_COLOR_WHITE" ");
	ssaoProgram = R_FindProgram ("ssao", qtrue, qtrue);
	depthDownsampleProgram = R_FindProgram("depthDownsample", qtrue, qtrue);
	ssaoBlurProgram = R_FindProgram("ssaoBlur", qtrue, qtrue);

	if (ssaoProgram->valid && depthDownsampleProgram->valid && ssaoBlurProgram->valid){
		Com_Printf("succeeded\n");

		id = depthDownsampleProgram->id[0];
		depthDS_depth		= qglGetUniformLocation(id, "u_depthBufferMap");
		depthDS_params		= qglGetUniformLocation(id, "u_depthParms");
		depthDS_orthoMatrix = qglGetUniformLocation(id, "u_orthoMatrix");

		id = ssaoProgram->id[0];
		ssao_mini			= qglGetUniformLocation(id, "u_DNMiniMap");
		ssao_rand			= qglGetUniformLocation(id, "u_randomNormalMap");
		ssao_params			= qglGetUniformLocation(id, "u_ssaoParms");
		ssao_vp				= qglGetUniformLocation(id, "u_viewport");
		ssao_orthoMatrix	= qglGetUniformLocation(id, "u_orthoMatrix");

		id = ssaoBlurProgram->id[0];
		ssaoB_mColor		= qglGetUniformLocation(id, "u_colorMiniMap");
		ssaoB_mDepth		= qglGetUniformLocation(id, "u_DNMiniMap");
		ssaoB_sapmles		= qglGetUniformLocation(id, "u_numSamples");
		ssaoB_axisMask		= qglGetUniformLocation(id, "u_axisMask");
		ssaoB_orthoMatrix	= qglGetUniformLocation(id, "u_orthoMatrix");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"bloom program"S_COLOR_WHITE" ");
	bloomdsProgram = R_FindProgram ("bloomds", qtrue, qtrue);
	bloomfpProgram = R_FindProgram ("bloomfp", qtrue, qtrue);

	if (bloomdsProgram->valid && bloomfpProgram->valid){
		Com_Printf("succeeded\n");

		id = bloomdsProgram->id[0];
		bloomDS_threshold	= qglGetUniformLocation(id, "u_BloomThreshold");
		bloomDS_map			= qglGetUniformLocation(id, "u_map");
		bloomDS_matrix		= qglGetUniformLocation(id, "u_orthoMatrix");

		id = bloomfpProgram->id[0];
		bloomFP_map0	= qglGetUniformLocation(id, "u_map0");
		bloomFP_map1	= qglGetUniformLocation(id, "u_map1");
		bloomFP_params	= qglGetUniformLocation(id, "u_bloomParams");
		bloom_FP_matrix = qglGetUniformLocation(id, "u_orthoMatrix");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"refraction program"S_COLOR_WHITE" ");
	refractProgram = R_FindProgram ("refract", qtrue, qtrue);

	if (refractProgram->valid) {
		Com_Printf ("succeeded\n");
		id = refractProgram->id[0];

		refract_normalMap	= qglGetUniformLocation (id, "u_deformMap");
		refract_baseMap		= qglGetUniformLocation (id, "u_colorMap");
		refract_screenMap	= qglGetUniformLocation (id, "g_colorBufferMap");
		refract_depthMap	= qglGetUniformLocation (id, "g_depthBufferMap");
		refract_deformMul	= qglGetUniformLocation (id, "u_deformMul");
		refract_alpha		= qglGetUniformLocation (id, "u_alpha");
		refract_thickness	= qglGetUniformLocation (id, "u_thickness");
		refract_screenSize	= qglGetUniformLocation (id, "u_viewport");
		refract_depthParams = qglGetUniformLocation (id, "u_depthParms");
		refract_ambient		= qglGetUniformLocation (id, "u_ambientScale");
		refract_alphaMask	= qglGetUniformLocation (id, "u_ALPHAMASK");
		refract_mask		= qglGetUniformLocation (id, "u_mask");
		refract_thickness2	= qglGetUniformLocation (id, "u_thickness2");
		refract_mvp			= qglGetUniformLocation	(id, "u_modelViewProjectionMatrix");
		refract_mv			= qglGetUniformLocation	(id, "u_modelViewMatrix");
		refract_pm			= qglGetUniformLocation	(id, "u_projectionMatrix");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"thermal vision program"S_COLOR_WHITE" ");
	thermalProgram = R_FindProgram ("thermal", qtrue, qtrue);

	thermalfpProgram = R_FindProgram ("thermalfp", qtrue, qtrue);

	if (thermalProgram->valid && thermalfpProgram){
		Com_Printf("succeeded\n");
		
		id = thermalProgram->id[0];
		therm_map		= qglGetUniformLocation(id, "u_screenTex");
		therm_matrix	= qglGetUniformLocation(id, "u_orthoMatrix");

		id = thermalfpProgram->id[0];
		thermf_map		= qglGetUniformLocation(id, "u_map");
		thermf_matrix	= qglGetUniformLocation(id, "u_orthoMatrix");

	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"water program"S_COLOR_WHITE" ");
	waterProgram = R_FindProgram ("water", qtrue, qtrue);
	if (waterProgram->valid) {
		Com_Printf ("succeeded\n");
		id = waterProgram->id[0];

		water_deformMap		= qglGetUniformLocation (id, "u_dstMap");
		water_baseMap		= qglGetUniformLocation (id, "u_colorMap");
		water_screenMap		= qglGetUniformLocation (id, "g_colorBufferMap");
		water_depthMap		= qglGetUniformLocation (id, "g_depthBufferMap");
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

	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"particles program"S_COLOR_WHITE" ");
	particlesProgram = R_FindProgram ("particles", qtrue, qtrue);

	if (particlesProgram->valid) {
		Com_Printf ("succeeded\n");
		id = particlesProgram->id[0];

		particle_texMap			= qglGetUniformLocation (id, "u_map0");
		particle_depthMap		= qglGetUniformLocation (id, "u_depthBufferMap");
		particle_depthParams	= qglGetUniformLocation (id, "u_depthParms");
		particle_mask			= qglGetUniformLocation (id, "u_mask");
		particle_thickness		= qglGetUniformLocation (id, "u_thickness");
		particle_colorModulate	= qglGetUniformLocation (id, "u_colorScale");
		particle_mvp			= qglGetUniformLocation (id, "u_modelViewProjectionMatrix");
		particle_mv				= qglGetUniformLocation (id, "u_modelViewMatrix");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}
	
	Com_Printf ("Load "S_COLOR_YELLOW"generic program"S_COLOR_WHITE" ");
	genericProgram = R_FindProgram ("generic", qtrue, qtrue);

	if (genericProgram->valid) {
		Com_Printf ("succeeded\n");

		id = genericProgram->id[0];
		gen_attribConsole	= qglGetUniformLocation (id, "u_ATTRIB_CONSOLE");
		gen_attribColors	= qglGetUniformLocation (id, "u_ATTRIB_COLORS");
		gen_tex				= qglGetUniformLocation (id, "u_map");
		gen_tex1			= qglGetUniformLocation (id, "u_map1");
		gen_colorModulate	= qglGetUniformLocation (id, "u_colorScale");
		gen_color			= qglGetUniformLocation (id, "u_color");
		gen_sky				= qglGetUniformLocation (id, "u_isSky");
		gen_mvp				= qglGetUniformLocation (id, "u_modelViewProjectionMatrix");
		gen_orthoMatrix		= qglGetUniformLocation (id, "u_orthoMatrix");
		gen_3d				= qglGetUniformLocation (id, "u_3d");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"cinematic program"S_COLOR_WHITE" ");
	cinProgram = R_FindProgram ("cin", qtrue, qtrue);

	if (cinProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"load screen program"S_COLOR_WHITE" ");
	loadingProgram = R_FindProgram ("loading", qtrue, qtrue);

	if (loadingProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"fxaa program"S_COLOR_WHITE" ");
	fxaaProgram = R_FindProgram ("fxaa", qtrue, qtrue);

	if (fxaaProgram->valid) {
		Com_Printf ("succeeded\n");
		id = fxaaProgram->id[0];
		fxaa_screenTex		= qglGetUniformLocation(id, "u_ScreenTex");
		fxaa_screenSize		= qglGetUniformLocation(id, "u_ScreenSize");
		fxaa_orthoMatrix	= qglGetUniformLocation(id, "u_orthoMatrix");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"film grain program"S_COLOR_WHITE" ");
	filmGrainProgram = R_FindProgram ("filmGrain", qtrue, qtrue);
	id = filmGrainProgram->id[0];
	film_tex	= qglGetUniformLocation(id, "u_ScreenTex");
	film_scroll = qglGetUniformLocation(id, "u_scroll");
	film_matrix = qglGetUniformLocation(id, "u_orthoMatrix");

	if (filmGrainProgram->valid) {
		Com_Printf ("succeeded\n");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf ("Load "S_COLOR_YELLOW"gammaramp program"S_COLOR_WHITE" ");
	gammaProgram = R_FindProgram ("gamma", qtrue, qtrue);
	if (gammaProgram->valid) {
		Com_Printf ("succeeded\n");
		id = gammaProgram->id[0];
		gamma_screenMap		= qglGetUniformLocation (id, "u_ScreenTex");
		gamma_control		= qglGetUniformLocation (id, "u_control");
		gamma_orthoMatrix	= qglGetUniformLocation (id, "u_orthoMatrix");
	}
	else {
		Com_Printf (S_COLOR_RED"Failed!\n");
		missing++;
	}


	Com_Printf("Load "S_COLOR_YELLOW"shadow volumes program"S_COLOR_WHITE" ");
	shadowProgram = R_FindProgram("shadow", qtrue, qtrue);
	if (shadowProgram->valid) {
		Com_Printf("succeeded\n");
		id = shadowProgram->id[0];
		sv_mvp = qglGetUniformLocation(id, "u_modelViewProjectionMatrix");
		sv_lightOrg = qglGetUniformLocation(id, "u_lightOrg");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"light2d program"S_COLOR_WHITE" ");
	light2dProgram = R_FindProgram("light2d", qtrue, qtrue);
	if (light2dProgram->valid) {
		Com_Printf("succeeded\n");
		id = light2dProgram->id[0];
		light2d_orthoMatrix = qglGetUniformLocation(id, "u_orthoMatrix");
		light2d_map = qglGetUniformLocation(id, "u_map");
		light2d_normal = qglGetUniformLocation(id, "u_normalMap");
		light2d_shift = qglGetUniformLocation(id, "u_shift");
		light2d_intens = qglGetUniformLocation(id, "u_intens");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	stop = Sys_Milliseconds ();
	sec = (float)stop - (float)start;
	Com_Printf ("\nGLSL shaders loading time: "S_COLOR_GREEN"%5.4f"S_COLOR_WHITE" sec\n", sec * 0.001);

	Com_Printf ("\n");
}

/*
=============
R_ShutdownPrograms

=============
*/
void R_ShutdownPrograms (void) {
	glslProgram_t	*program;
	int			i, j;

	for (i = 0; i < r_numPrograms; i++) {
		program = &r_programs[i];

		for (j = 0; j < program->numId; j++) {
			// free all non-zero indices
			if (program->id[j])
				qglDeleteProgram (program->id[j]);
		}
	}
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

		Com_Printf ("  %4i: %12i %s%s\n", i, program->numId, program->name, program->valid ? "" : "(INVALID)");
	}

	Com_Printf ("-------------------\n");
	Com_Printf (" %i programs\n", r_numPrograms);
	Com_Printf ("  %i invalid\n", numInvalid);
}

void R_GLSLinfo_f(void) {
	
	int i;
	uint j;
	const char *ver;

	ver = (const char*)qglGetString(GL_SHADING_LANGUAGE_VERSION_ARB);
	Com_Printf("GLSL Version: "S_COLOR_GREEN"%s\n", ver);

	qglGetIntegerv(GL_NUM_SHADING_LANGUAGE_VERSIONS, &j);
	for (i = 0; i < j; ++i) {
		ver = glGetStringi(GL_SHADING_LANGUAGE_VERSION, i);
		if (!ver)
			break;
		Com_Printf(S_COLOR_YELLOW"%s\n", ver);
	}
}
/*
============
GL_BindNullProgram

============
*/
void GL_BindNullProgram (void) {

	if (gl_state.programId) {
		qglUseProgram (0);
		gl_state.programId = 0;
	}
}


/*
============
GL_BindProgram

============
*/
void GL_BindProgram (glslProgram_t *program, int defBits) {
	int		id;

	if (!program || program->numId < defBits) {
		GL_BindNullProgram ();
		return;
	}

	id = program->id[defBits];

	if (gl_state.programId != id) {
		qglUseProgram (id);
		gl_state.programId = id;
	}
}

