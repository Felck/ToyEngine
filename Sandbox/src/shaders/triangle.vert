#version 450

layout(location = 0) in vec2 position;
layout(location = 0) out vec3 outColor;
layout(push_constant) uniform Constants {
    mat4 viewProjection;
} PushConstants;

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    gl_Position = PushConstants.viewProjection * vec4(position, 0.0, 1.0);
    outColor = colors[gl_VertexIndex];
}