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

// Usage of half-floats gives 5-10% additional performance,
// as well as "dark room with a lot of little lights" artefacts.
// GF5xxx and higher have half-precision types, Radeons don't.
static const char *floatDefs16 =
	"#define xpFloat	half\n"
	"#define xpVec2		hvec2\n"
	"#define xpVec3		hvec3\n"
	"#define xpVec4		hvec4\n"
	"#define xpMat2		hmat2\n"
	"#define xpMat3		hmat3\n"
	"#define xpMat4		hmat4\n";

static const char *floatDefs32 =
	"#define xpFloat	float\n"
	"#define xpVec2		vec2\n"
	"#define xpVec3		vec3\n"
	"#define xpVec4		vec4\n"
	"#define xpMat2		mat2\n"
	"#define xpMat3		mat3\n"
	"#define xpMat4		mat4\n";

static glslProgram_t	r_nullProgram;



/*
=================
Com_HashKey

=================
*/
unsigned Com_HashKey (const char *string, unsigned size) {
	int			i;
	unsigned	hash = 0;
	char		letter;

	for (i=0; string[i]; i++) {
		letter = tolower(string[i]);
		
		if (letter =='.') break;				// don't include extension
		if (letter =='\\') letter = '/';		// damn path names

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
qboolean Q_IsLiteral(const char *text) {
	int		i, c, len;

	len = strlen(text);

	for (i=0; i<len; i++) {
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
R_GetProgramDefBits

==============
*/
unsigned R_GetProgramDefBits(glslProgram_t *program, const char *name) {
	int		i;

	if (!program || !program->valid)
		return 0;

	for (i=0; i<program->numDefs; i++) {
		if (!strcmp(program->defStrings[i], name))
			return program->defBits[i];
	}

	return 0;
}

/*
==============
R_ProgramForName

==============
*/
static glslProgram_t *R_ProgramForName(const char *name) {
	glslProgram_t	*program;
	unsigned	hash;

	hash = Com_HashKey(name, PROGRAM_HASH_SIZE);

	for (program=programHashTable[hash]; program; program=program->nextHash) {
		if(!Q_stricmp(program->name, name))
			return program;
	}

	return NULL;
}

/*
===============
R_GetInfoLog

===============
*/
static void R_GetInfoLog(int id, char *log, qboolean isProgram) {
	int		length, dummy;

	if (isProgram)
		qglGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);
	else
		qglGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

	if (length < 1) {
		log[0] = 0;
		return;
	}

	if (length >= MAX_INFO_LOG)
		length = MAX_INFO_LOG - 1;

	if (isProgram)
		qglGetProgramInfoLog(id, length, &dummy, log);
	else
		qglGetShaderInfoLog(id, length, &dummy, log);

	log[length] = 0;
}

/*
==============
R_ParseDefs

==============
*/
static void R_ParseDefs(glslProgram_t *program, const char *text) {
	parser_t	parser;
	token_t		token;

	Parser_Reset(&parser, va("program '%s'", program->name), text);

	while (1) {
		if (!Parser_GetToken(&parser, &token))
			return;

		if (!Q_IsLiteral(token.text)) {
			Parser_Warning(&parser, "def '%s' is non-literal\n", token.text);
			return;
		}

		if (program->numDefs == MAX_PROGRAM_DEFS) {
			Parser_Warning(&parser, "MAX_PROGRAM_DEFS hit\n");
			return;
		}

		program->defBits[program->numDefs] = BIT(program->numDefs);
		Q_strncpyz(program->defStrings[program->numDefs], token.text, MAX_DEF_NAME);

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
 char *R_LoadIncludes(char *glsl)
{
    char filename[MAX_QPATH];
    char *token, *p, *oldp, *oldglsl;
    int l, limit = 64;          // limit for prevent infinity recursion

    /// calculate size of glsl with includes
    l = strlen(glsl);
    p = glsl;
    while ( 1 )
    {
         oldp = p;
         token = Com_ParseExt(&p, true);
         if ( !token[0] )
              break;

		 if (!strcmp(token, "#include"))
         {
              int	li;
              char	*buf;

              if (limit<0)
                   Com_Error (ERR_FATAL, "R_LoadIncludes: more than 64 includes");

              token = Com_ParseExt(&p, false);
              Com_sprintf( filename, sizeof( filename ), "glsl/include/%s", token );
              li = FS_LoadFile(filename, (void **)&buf);
              if ( !buf )
                   Com_Error (ERR_FATAL, "Couldn't load %s", filename);

              oldglsl = glsl;
              glsl = (char*)Q_malloc(l+li+2);
              memset(glsl, 0, l+li+2);
              Q_memcpy(glsl, oldglsl, oldp-oldglsl);
              Q_strcat(glsl, "\n", l+li+1);
              Q_strcat(glsl, buf,  l+li+1);
              Q_strcat(glsl, p,    l+li+1);
              p = oldp - oldglsl + glsl;
              l = strlen(glsl);
              FS_FreeFile( buf );
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
qboolean R_LoadBinaryShader(char *shaderName, int shaderId, int mutatorIndex){

	char			name[MAX_QPATH];
	GLint			binLength;
    GLvoid*			bin;
    GLint			success;
    FILE*			binFile;
	
	Com_sprintf(name, sizeof(name), "%s/glslbin/%s_%i.bin", FS_Gamedir(), shaderName, mutatorIndex);
	FS_CreatePath(name);

	binFile = fopen(name, "rb");
	if(!binFile){
		return false;
	}else{
	fseek(binFile, 0, SEEK_END);
    binLength = (GLint)ftell(binFile);
    bin = (GLvoid*)malloc(binLength);
    fseek(binFile, 0, SEEK_SET);
    fread(bin, binLength, 1, binFile);
    fclose(binFile);

	glProgramBinary(shaderId, gl_state.binaryFormats, bin, binLength);
	qglGetProgramiv(shaderId, GL_LINK_STATUS, &success);
	free(bin);	        
        
		if (success){
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

static glslProgram_t *R_CreateProgram(const char *name, const char *defs, const char *vertexSource, const char *fragmentSource) {
	char			log[MAX_INFO_LOG];
	unsigned		hash;
	glslProgram_t	*program;
	const char		*strings[MAX_PROGRAM_DEFS * 3 + 2];
	int				numStrings;
	int				numLinked = 0;
	int				id, vertexId, fragmentId;
	int				status;
	int				i, j;
	GLint			binLength;
    GLvoid*			bin;
    FILE*			binFile;

	if ((vertexSource && strlen(vertexSource) < 17) || (fragmentSource && strlen(fragmentSource) < 17))
		return NULL;

	if (r_numPrograms == MAX_PROGRAMS)
		VID_Error (ERR_DROP, "R_CreateProgram: MAX_PROGRAMS hit");
	
	
	for (i=0, program = r_programs ; i < r_numPrograms ; i++, program++){
		if(!r_programs->name[0])
			break;
	}

	if (i == r_numPrograms) {
		if (r_numPrograms == MAX_PROGRAMS)
			VID_Error(ERR_DROP, "MAX_PROGRAMS");
		r_numPrograms++;
	}
	program = &r_programs[i];
		
	memset(program, 0, sizeof(*program));
	Q_strncpyz(program->name, name, sizeof(program->name));

	R_ParseDefs(program, defs);

	program->numId = BIT(program->numDefs);

	for (i=0; i<program->numId; i++) {

		char *defines = NULL;		/// Berserker's fix
		numStrings = 0;
		vertexId = 0;
		fragmentId = 0;

		/// Berserker's fix start
		if (program->numDefs)
			{
				int len = 0;
				// посчитаем требуемый объём памяти по дифайны
				for (j = 0; j<program->numDefs; j++)
					if (i & program->defBits[j])
						len += 8 + strlen(program->defStrings[j]) + 1; // 8 = strlen("#define "), 1 = strlen("\n")

			len++; // for trailing NULL
			defines = (char*)calloc(len, 1); // calloc = malloc + memclear  ;)
			for (j = 0; j<program->numDefs; j++)
				{
				if (i & program->defBits[j])
					{
						Q_strcat(defines, "#define ", len);
						Q_strcat(defines, program->defStrings[j], len);
						Q_strcat(defines, "\n", len);
					}
				}
				strings[numStrings++] = defines;
			}
		/// Berserker's fix end

		strings[numStrings++] = floatDefs32;

		// add check for binaries here!!!

		// compile vertex shader
		if (vertexSource) {
			// link includes
			vertexSource = R_LoadIncludes((char*)vertexSource);
			
			strings[numStrings] = vertexSource;
			vertexId = qglCreateShader(GL_VERTEX_SHADER);
			qglShaderSource(vertexId, numStrings + 1, strings, NULL);
			qglCompileShader(vertexId);
			qglGetShaderiv(vertexId, GL_COMPILE_STATUS, &status);

			if(!status) {
				R_GetInfoLog(vertexId, log, false);
				qglDeleteShader(vertexId);
				Com_Printf("program '%s': error(s) in vertex shader:\n-----------\n%s\n-----------\n", program->name, log);
				continue;
			}
		}
		
		// compile fragment shader
		if (fragmentSource) {
			// link includes
			fragmentSource = R_LoadIncludes((char*)fragmentSource);
			strings[numStrings] = fragmentSource;
			fragmentId = qglCreateShader(GL_FRAGMENT_SHADER);
			
		if(0){
        
		int lll;
        char *gls = (char*)fragmentSource;

        Com_Printf("\n" );
		again:  
		lll = strlen(gls);

        if (lll>8){
            char bak = gls[8];
            
			gls[8] = 0;
            Com_Printf( gls );
            gls[8] = bak;
            gls += 8;
            goto again;
         }
         else{
			Com_Printf("%d", gls );
         }
			Com_Printf( "\n\n" );
	}
		
		
			qglShaderSource(fragmentId, numStrings + 1, strings, NULL);
			qglCompileShader(fragmentId);
			qglGetShaderiv(fragmentId, GL_COMPILE_STATUS, &status);

			if(!status) {
				R_GetInfoLog(fragmentId, log, false);
				qglDeleteShader(fragmentId);
				Com_Printf("program '%s': error(s) in fragment shader:\n-----------\n%s\n-----------\n", program->name, log);
				continue;
			}
		}

		//
		// link the program
		//

		id = qglCreateProgram();
		
		if(gl_state.glslBinary){
		if(R_LoadBinaryShader(program->name, id, i))
			goto end;
		}

		qglBindAttribLocation(id, ATRB_POSITION,	"a_vertArray");	
		qglBindAttribLocation(id, ATRB_NORMAL,		"a_normArray");
		qglBindAttribLocation(id, ATRB_TEX0,		"a_texCoord");
		qglBindAttribLocation(id, ATRB_TEX1,		"a_LtexCoord");
		qglBindAttribLocation(id, ATRB_TEX2,		"a_2texCoord");
		qglBindAttribLocation(id, ATRB_TANGENT,		"a_tangent");
		qglBindAttribLocation(id, ATRB_BINORMAL,	"a_binormal");
		qglBindAttribLocation(id, ATRB_COLOR,		"a_colorArray");

		if (vertexId) {
			qglAttachShader(id, vertexId);
			qglDeleteShader(vertexId);
		}

		if (fragmentId) {
			qglAttachShader(id, fragmentId);
			qglDeleteShader(fragmentId);
		}
		
		if(gl_state.glslBinary)
			glProgramParameteri(id, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
		
		qglLinkProgram(id);
		qglGetProgramiv(id, GL_LINK_STATUS, &status);
		
		if(gl_state.glslBinary){
			
		char name[MAX_QPATH];
		
		qglGetProgramiv(id, GL_PROGRAM_BINARY_LENGTH, &binLength);
		bin = (GLvoid*)malloc(binLength);
		glGetProgramBinary(id, binLength, &binLength, &gl_state.binaryFormats, bin);
		
		Com_sprintf(name, sizeof(name), "%s/glslbin/%s_%i.bin", FS_Gamedir(), program->name, i);
		FS_CreatePath(name);
		binFile = fopen(name, "wb");
        fwrite(bin, binLength, 1, binFile);
        fclose(binFile);
        free(bin);
		}

		R_GetInfoLog(id, log, true);

		if(!status) {
			qglDeleteProgram(id);
			Com_Printf("program '%s': link error(s): %s\n", program->name, log);
			continue;
		}

		// don't let it be slow (crap)
		if (strstr(log, "fragment shader will run in software")) {
			qglDeleteProgram(id);
			Com_Printf("program '%s': refusing to perform software emulation\n", program->name);
			continue;
		}

		// TODO: glValidateProgram?
end:
		/// Berserker's fix start
		if (defines)
			free(defines);
		/// Berserker's fix end

		program->id[i] = id;
		numLinked++;
	}

	if (numLinked == program->numId)
		program->valid = true;

	// add to the hash
	hash = Com_HashKey(program->name, PROGRAM_HASH_SIZE);
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

glslProgram_t *R_FindProgram(const char *name, qboolean vertex, qboolean fragment) {
	char			filename[MAX_QPATH];
	char			newname[MAX_QPATH];
	glslProgram_t	*program;
	char			*defs, *vertexSource = NULL, *fragmentSource = NULL;

	if (!vertex && !fragment)
		return &r_nullProgram;


	Q_snprintfz(newname, sizeof(newname), "%s%s", name, !vertex ? "(fragment)" : !fragment ? "(vertex)" : "");

	program = R_ProgramForName(newname);
	if (program)
		return program;

	Q_snprintfz(filename, sizeof(filename), "glsl/%s.defs", name);
	FS_LoadFile(filename, (void **)&defs);

	if (vertex) {
		Q_snprintfz(filename, sizeof(filename), "glsl/%s.vp", name);
		FS_LoadFile(filename, (void **)&vertexSource);
	}

	if (fragment) {
		Q_snprintfz(filename, sizeof(filename), "glsl/%s.fp", name);
		FS_LoadFile(filename, (void **)&fragmentSource);
	}

	if (!vertexSource && !fragmentSource)
		return &r_nullProgram;		// no appropriate shaders found
		
	program = R_CreateProgram(newname, defs, vertexSource, fragmentSource);


	if (defs)
		FS_FreeFile(defs);
	if (vertexSource)
		FS_FreeFile(vertexSource);
	if (fragmentSource)
		FS_FreeFile(fragmentSource);

	if (!program || !program->valid)
		return &r_nullProgram;

	return program;
}

/*
=============
R_InitPrograms

=============
*/


void R_InitPrograms(void) {
	int missing = 0, loadingTime=0, loadingTime2=0;
	float sec;

	Com_Printf("\nInitializing programs...\n\n");
	
	loadingTime = Sys_Milliseconds ();

	memset(programHashTable, 0, sizeof(programHashTable));
	memset(&r_nullProgram, 0, sizeof(glslProgram_t));

		Com_Printf("Load "S_COLOR_YELLOW"null program"S_COLOR_WHITE" ");
	nullProgram = R_FindProgram("null", true, true);
	if(nullProgram->valid){
		Com_Printf("succeeded\n");
	} else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"ambient world program"S_COLOR_WHITE" ");
	ambientWorldProgram = R_FindProgram("ambientWorld", true, true);
	if(ambientWorldProgram->valid){
		Com_Printf("succeeded\n");

		worldDefs.ParallaxBit		= R_GetProgramDefBits(ambientWorldProgram, "PARALLAX");
		worldDefs.LightmapBits		= R_GetProgramDefBits(ambientWorldProgram, "LIGHTMAP");
		worldDefs.VertexLightBits	= R_GetProgramDefBits(ambientWorldProgram, "VERTEXLIGHT");
	} else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"light world program"S_COLOR_WHITE" ");
	lightWorldProgram = R_FindProgram("lightWorld", true, true);
	if(lightWorldProgram->valid){
		Com_Printf("succeeded\n");
		worldDefs.LightParallaxBit	= R_GetProgramDefBits(lightWorldProgram, "PARALLAX");
		worldDefs.AmbientBits		= R_GetProgramDefBits(lightWorldProgram, "AMBIENT");

	} else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}
	
	Com_Printf("Load "S_COLOR_YELLOW"ambient model program"S_COLOR_WHITE" ");
	aliasAmbientProgram  = R_FindProgram("ambientAlias", true, true);
	if(aliasAmbientProgram->valid){
		Com_Printf("succeeded\n");

		worldDefs.CausticsBit	= R_GetProgramDefBits(aliasAmbientProgram, "CAUSTICS");
		worldDefs.ShellBits		= R_GetProgramDefBits(aliasAmbientProgram, "SHELL");
		worldDefs.EnvBits		= R_GetProgramDefBits(aliasAmbientProgram, "ENVMAP");

	} else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}


	Com_Printf("Load "S_COLOR_YELLOW"light model program"S_COLOR_WHITE" ");
	aliasBumpProgram = R_FindProgram("aliasBump", true, true);

	if(aliasBumpProgram->valid){
		Com_Printf("succeeded\n");
		worldDefs.AmbientAliasBits	= R_GetProgramDefBits(aliasBumpProgram, "AMBIENT");
	}
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}
	
	Com_Printf("Load "S_COLOR_YELLOW"gauss blur program"S_COLOR_WHITE" ");
	gaussXProgram = R_FindProgram("gaussX", true, true);
	gaussYProgram = R_FindProgram("gaussY", true, true);
	
	if(gaussXProgram->valid && gaussYProgram->valid)
		Com_Printf("succeeded\n");
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"radial blur program"S_COLOR_WHITE" ");
	radialProgram = R_FindProgram("radialBlur", true, true);
	if(radialProgram->valid)
		Com_Printf("succeeded\n");
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}
	
	Com_Printf("Load "S_COLOR_YELLOW"dof blur program"S_COLOR_WHITE" ");
	dofProgram = R_FindProgram("dof", true, true);
	if(dofProgram->valid)
		Com_Printf("succeeded\n");
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}
	

	Com_Printf("Load "S_COLOR_YELLOW"bloom program"S_COLOR_WHITE" ");
	bloomdsProgram = R_FindProgram("bloomds", true, true);
	bloomfpProgram = R_FindProgram("bloomfp", true, true);

	if(bloomfpProgram->valid)
		Com_Printf("succeeded\n");
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}
	
	Com_Printf("Load "S_COLOR_YELLOW"refraction program"S_COLOR_WHITE" ");
	refractProgram = R_FindProgram("refract", true, true);
	
	if(refractProgram->valid){

		worldDefs.AlphaMaskBits	 = R_GetProgramDefBits(refractProgram, "ALPHAMASK");
		Com_Printf("succeeded\n");
	}else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

		
	Com_Printf("Load "S_COLOR_YELLOW"thermal vision program"S_COLOR_WHITE" ");
	thermalProgram = R_FindProgram("thermal", true, true);
	
	thermalfpProgram =  R_FindProgram("thermalfp", true, true);

	if(thermalProgram->valid && thermalfpProgram)
		Com_Printf("succeeded\n");
	else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"water program"S_COLOR_WHITE" ");
	waterProgram = R_FindProgram("water", true, true);
	worldDefs.WaterTransBits = R_GetProgramDefBits(waterProgram, "TRANS");

	if(waterProgram->valid){
		Com_Printf("succeeded\n");
	}else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"particles program"S_COLOR_WHITE" ");
	particlesProgram =  R_FindProgram("particles", true, true);

	if(particlesProgram->valid){
		Com_Printf("succeeded\n");
	}else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"shadow program"S_COLOR_WHITE" ");
	shadowProgram =  R_FindProgram("shadow", true, true);
	
	if(shadowProgram->valid){
		Com_Printf("succeeded\n");
	}else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"generic program"S_COLOR_WHITE" ");
	genericProgram =  R_FindProgram("generic", true, true);
	
	if(genericProgram->valid){
		Com_Printf("succeeded\n");
	}else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"cinematic program"S_COLOR_WHITE" ");
	cinProgram =  R_FindProgram("cin", true, true);
	
	if(cinProgram->valid){
		Com_Printf("succeeded\n");
	}else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"load screen program"S_COLOR_WHITE" ");
	loadingProgram =  R_FindProgram("loading", true, true);
	
	if(loadingProgram->valid){
		Com_Printf("succeeded\n");
	}else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"fxaa program"S_COLOR_WHITE" ");
	fxaaProgram =  R_FindProgram("fxaa", true, true);
	
	if(fxaaProgram->valid){
		Com_Printf("succeeded\n");
	}else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	Com_Printf("Load "S_COLOR_YELLOW"film grain program"S_COLOR_WHITE" ");
	filmGrainProgram =  R_FindProgram("filmGrain", true, true);
	
	if(filmGrainProgram->valid){
		Com_Printf("succeeded\n");
	}else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}
	
	Com_Printf("Load "S_COLOR_YELLOW"gammaramp program"S_COLOR_WHITE" ");
	gammaProgram = R_FindProgram("gamma", true, true);
	if(gammaProgram->valid){
		Com_Printf("succeeded\n");
	} else {
		Com_Printf(S_COLOR_RED"Failed!\n");
		missing++;
	}

	loadingTime2 = Sys_Milliseconds ();
	sec = (float)loadingTime2 - (float)loadingTime;
	sec *=0.001;
	Com_Printf("Programs loading time = %0.4f sec\n",sec);

	Com_Printf("\n");
//	if (missing > 0)
//		Com_Error(ERR_FATAL, "%d programs couldn't be found\n", missing);
}

/*
=============
R_ShutdownPrograms

=============
*/
void R_ShutdownPrograms(void) {
	glslProgram_t	*program;
	int			i, j;

	for (i=0; i<r_numPrograms; i++) {
		program = &r_programs[i];

		for (j=0; j<program->numId; j++) {
			// free all non-zero indices
			if (program->id[j])
				qglDeleteProgram(program->id[j]);
		}
	}
}

/*
=============
R_ListPrograms_f

=============
*/
void R_ListPrograms_f(void) {
	glslProgram_t	*program;
	int			numInvalid = 0;
	int			i;

	Com_Printf("        permutations name\n");
	Com_Printf("-------------------------\n");

	for (i=0; i<r_numPrograms; i++) {
		program = &r_programs[i];
		if (!program->valid)
			numInvalid++;

		Com_Printf("  %4i: %12i %s%s\n", i, program->numId, program->name, program->valid ? "" : "(INVALID)");
	}

	Com_Printf("-------------------\n");
	Com_Printf(" %i programs\n", r_numPrograms);
	Com_Printf("  %i invalid\n", numInvalid);
}

/*
============
GL_BindNullProgram

============
*/
void GL_BindNullProgram(void) {
		
	if (gl_state.programId) {
		qglUseProgram(0);
		gl_state.programId = 0;
	}
}


/*
============
GL_BindProgram

============
*/
void GL_BindProgram(glslProgram_t *program, int defBits) {
	int		id;

	if (!program || program->numId < defBits) {
		GL_BindNullProgram();
		return;
	}

	id = program->id[defBits];

	if (gl_state.programId != id) {
		qglUseProgram(id);
		gl_state.programId = id;
	}
}
 
