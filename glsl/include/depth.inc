
float DecodeDepth (const float d, const in vec2 parms) {
	return parms.x / (parms.y - d);
}