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

layout(location = 0) out vec4 fragColor;

void main() {
    vec2 screenPos = vec2(gl_FragCoord.x, u_resolution.y - gl_FragCoord.y);

    vec2 canvasPos = (screenPos - u_canvasOffset) / u_canvasScale;

    vec2 gridPos = round(canvasPos / u_gridSpacing) * u_gridSpacing;

    vec2 gridScreenPos = gridPos * u_canvasScale + u_canvasOffset;
    float dist = distance(screenPos, gridScreenPos);

    float dotAlpha = 1.0 - smoothstep(u_dotRadius - 0.5, u_dotRadius + 0.5, dist);

    fragColor = mix(u_bgColor, u_dotColor, dotAlpha);
}
)#"
