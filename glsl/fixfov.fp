uniform sampler2D tex;     // sampler of rendered scene’s render target

in vec3 vUV;               // interpolated vertex output data
in vec2 vUVDot;            // interpolated vertex output data
 
void main() {
    vec3 uv = dot(vUVDot, vUVDot) * vec3(-0.5, -0.5, -1.0) + vUV;
    fragData = texture2DProj(tex, uv);
}