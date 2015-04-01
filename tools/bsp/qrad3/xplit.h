typedef enum {
	LT_OMNI,
	LT_SPOT
} xpLightType_t;

typedef struct xpLight_s {
	xpLightType_t	type;

	int			style;

	vec3_t		radius;
	vec3_t		origin;
	vec3_t		color;

	// for spotlights
	float		cone;
	vec3_t		dir;
} xpLight_t;

extern int			numXPLights;
extern xpLight_t	xpLights[MAX_MAP_ENTITIES];

void LoadXPLights (char *filename);
