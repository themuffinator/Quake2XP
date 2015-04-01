#include "cmdlib.h"
#include "mathlib.h"
#include "bspfile.h"
#include "scriplib.h"
#include "xplit.h"

int			numXPLights;
xpLight_t	xpLights[MAX_MAP_ENTITIES];

void AngleVectors (vec3_t angles, vec3_t forward, vec3_t right, vec3_t up) {
	float		angle;
	static float		sr, sp, sy, cr, cp, cy;

	// static to help MS compiler fp bugs

	angle = angles[1] * (Q_PI / 180.f);
	sy = sinf(angle);
	cy = cosf(angle);
	angle = angles[0] * (Q_PI / 180.f);
	sp = sinf(angle);
	cp = cosf(angle);
	angle = angles[2] * (Q_PI / 180.f);
	sr = sinf(angle);
	cr = cosf(angle);

	if (forward)
	{
		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;
	}

	if (right)
	{
		right[0] = (-1*sr*sp*cy+-1*cr*-sy);
		right[1] = (-1*sr*sp*sy+-1*cr*cy);
		right[2] = -1*sr*cp;
	}

	if (up)
	{
		up[0] = (cr*sp*cy+-sr*-sy);
		up[1] = (cr*sp*sy+-sr*cy);
		up[2] = cr*cp;
	}
}

/*
================
LoadXPLights

================
*/
void LoadXPLights (char *filename) {
	int			style;
	char		*header, *c, *token, key[256], *value;
	vec3_t		radius, origin, angles, color;
	float		cone;
	xpLight_t	*l;

	LoadFile (filename, (void **)&header);

	numXPLights = 0;
	
	c = header;

	while (1) {
		token = COM_Parse(&c);
		
		if (!c)
			break;	// end of file

		if (token[0] != '{')
			Error("expected '{', found '%c'", token[0]);

		style = 0;
		cone = 0.f;

		VectorClear(radius);
		VectorClear(angles);
		VectorClear(origin);
		VectorClear(color);

		while (1) {
			token = COM_Parse(&c);

			if (!c)
				Error("unexpected end of .xplit file");

			if (token[0] == '}')
				break;

			strncpy(key, token, sizeof(key)-1);

			value = COM_Parse(&c);
			
			if(!stricmp(key, "radius"))
				sscanf(value, "%f %f %f", &radius[0], &radius[1], &radius[2]);
			else if(!stricmp(key, "origin"))
				sscanf(value, "%f %f %f", &origin[0], &origin[1], &origin[2]);
			else if(!stricmp(key, "color"))
				sscanf(value, "%f %f %f", &color[0], &color[1], &color[2]);
			else if(!stricmp(key, "style"))
				style = atoi(value);
			else if(!stricmp(key, "angles"))
				sscanf(value, "%f %f %f", &angles[0], &angles[1], &angles[2]);
			else if(!stricmp(key, "cone") || !stricmp(key, "_cone"))
				cone = atof(value);
		}
	
		l = &xpLights[numXPLights++];
		l->style = style;
		VectorCopy(origin, l->origin);
		VectorCopy(color, l->color);
		VectorCopy(radius, l->radius);

		l->cone = cone;

		if (l->cone) {
			l->type = LT_SPOT;
			AngleVectors(angles, l->dir, NULL, NULL);
		}
		else
			l->type = LT_OMNI;
	}

	free (header);		// everything has been copied out
}
