#version 450

layout(location = 0) in vec2 position;
layout(location = 0) out vec3 outColor;

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    outColor = colors[gl_VertexIndex];
}