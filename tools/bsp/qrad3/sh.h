typedef vec4_t shCoeffs_t;
typedef shCoeffs_t shSpectralCoeffs_t[3];

void SHS_Clear (shSpectralCoeffs_t c);
void SHS_Copy (const shSpectralCoeffs_t a, shSpectralCoeffs_t b);
void SHS_Add (const shSpectralCoeffs_t a, const shSpectralCoeffs_t b, shSpectralCoeffs_t out);
void SHS_Sub (const shSpectralCoeffs_t a, const shSpectralCoeffs_t b, shSpectralCoeffs_t out);

void SH_MulFloat (const shCoeffs_t c, const float f, shSpectralCoeffs_t out);
void SH_MulVec3 (const shCoeffs_t c, const vec3_t vec, shSpectralCoeffs_t out);

void SHS_MulFloat (const shSpectralCoeffs_t c, const float f, shSpectralCoeffs_t out);
void SHS_MulVec3 (const shSpectralCoeffs_t sc, const vec3_t vec, shSpectralCoeffs_t out);

void SHS_Dot (const shSpectralCoeffs_t sc, const shCoeffs_t c, vec3_t out);

void SH_Dir (const vec3_t dir, shCoeffs_t out);
void SH_Cone (const vec3_t dir, const float halfAngle, shCoeffs_t out);
void SH_Cone60 (const vec3_t dir, shCoeffs_t out);
void SH_Cone90 (const vec3_t dir, shCoeffs_t out);
void SH_Cosine (const vec3_t dir, shCoeffs_t out);
