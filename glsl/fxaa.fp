// FXAA shader
// Original algorithm and code by Timothy Lottes
// Adoptation by Vic

#define FXAA_GREEN_AS_LUMA 1

#include Fxaa3_11.inc

uniform sampler2D	u_ScreenTex;
uniform vec2		u_ScreenSize;

void main(void)
{
    vec2 rcpFrame = vec2(1.0) / u_ScreenSize.xy; 
    vec2 pos = gl_FragCoord.xy / u_ScreenSize.xy;

	// Only used on FXAA Quality.
    // Choose the amount of sub-pixel aliasing removal.
    // This can effect sharpness.
    //   1.00 - upper limit (softer)
    //   0.75 - default amount of filtering
    //   0.50 - lower limit (sharper, less sub-pixel aliasing removal)
    //   0.25 - almost off
    //   0.00 - completely off
    float QualitySubpix = 0.0;

    // The minimum amount of local contrast required to apply algorithm.
    //   0.333 - too little (faster)
    //   0.250 - low quality
    //   0.166 - default
    //   0.125 - high quality 
    //   0.033 - very high quality (slower)
    float QualityEdgeThreshold = 0.033;
    float QualityEdgeThresholdMin = 0.0;

    vec4 ConsolePosPos = vec4(0.0);
    vec4 ConsoleRcpFrameOpt = vec4(0.0);
    vec4 ConsoleRcpFrameOpt2 = vec4(0.0);
    vec4 Console360RcpFrameOpt2 = vec4(0.0);
    float ConsoleEdgeSharpness = 8.0;
    float ConsoleEdgeThreshold = 0.125;
    float ConsoleEdgeThresholdMin = 0.05;
    vec4  Console360ConstDir = vec4(1.0, -1.0, 0.25, -0.25);

    fragData = FxaaPixelShader(pos, ConsolePosPos, u_ScreenTex, u_ScreenTex, u_ScreenTex, rcpFrame, ConsoleRcpFrameOpt, ConsoleRcpFrameOpt2, Console360RcpFrameOpt2, QualitySubpix, QualityEdgeThreshold, QualityEdgeThresholdMin, ConsoleEdgeSharpness, ConsoleEdgeThreshold, ConsoleEdgeThresholdMin, Console360ConstDir);
}
