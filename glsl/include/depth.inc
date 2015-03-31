/*
==============
DecodeDepth

Converts logarithmic depth into positive linear eye-space Z.
==============
*/
float DecodeDepth (const in float x, const in vec2 parms) {
	return parms.x / (parms.y - x);
}

/*
==============
EncodeDepth

Converts positive linear eye-space Z into logarithmic depth.
==============
*/
float EncodeDepth (const in float x, const in vec2 parms) {
	return parms.y - parms.x / x;
}
