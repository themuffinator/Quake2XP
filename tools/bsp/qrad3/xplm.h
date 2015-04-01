#define XPLM_NUMVECS	3
#define MAX_XPLM_SIZE	(MAX_MAP_LIGHTING * XPLM_NUMVECS)

typedef struct {
	int			dataSize;
	byte		data[MAX_XPLM_SIZE];
} xplm_t;

extern const vec3_t xplm_basisVecs[XPLM_NUMVECS];

void XP_RadWorld (void);
void XP_WriteXPLM (char *filename);
