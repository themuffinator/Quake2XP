layout (bindless_sampler, location = U_TMU0) uniform sampler2D  tex;     // sampler of rendered scene’s render target

in vec3 vUV;               // interpolated vertex output data
in vec2 vUVDot;            // interpolated vertex output data
 
void main() {
    vec3 uv = dot(vUVDot, vUVDot) * vec3(-0.5, -0.5, -1.0) + vUV;
    fragData = textureProj(tex, uv);
}