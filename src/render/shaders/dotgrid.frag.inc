R"#(
#version 300 es
precision highp float;

uniform mat3 proj;
uniform vec2 u_resolution;
uniform vec2 u_canvasOffset;
uniform float u_canvasScale;
uniform float u_gridSpacing;
uniform float u_dotRadius;
uniform vec4 u_dotColor;
uniform vec4 u_bgColor;
uniform vec2 u_cursorPos;
uniform float u_glowRadius;
uniform vec4 u_glowDotColor;
uniform float u_glowDotRadius;

layout(location = 0) out vec4 fragColor;

void main() {
    vec2 screenPos = vec2(gl_FragCoord.x, u_resolution.y - gl_FragCoord.y);

    vec2 canvasPos = (screenPos - u_canvasOffset) / u_canvasScale;

    vec2 gridPos = round(canvasPos / u_gridSpacing) * u_gridSpacing;

    vec2 gridScreenPos = gridPos * u_canvasScale + u_canvasOffset;
    float dist = distance(screenPos, gridScreenPos);

    float cursorDist = distance(gridScreenPos, u_cursorPos);
    float glowFactor = 1.0 - smoothstep(0.0, u_glowRadius, cursorDist);

    float effectiveRadius = mix(u_dotRadius, u_glowDotRadius, glowFactor);
    vec4 effectiveDotColor = mix(u_dotColor, u_glowDotColor, glowFactor);

    float dotAlpha = 1.0 - smoothstep(effectiveRadius - 0.5, effectiveRadius + 0.5, dist);

    fragColor = mix(u_bgColor, effectiveDotColor, dotAlpha);
}
)#"
