#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec2 outUV;

layout(push_constant) uniform Constants {
    mat4 viewProjection;
} PushConstants;

void main() {
    gl_Position = PushConstants.viewProjection * vec4(position, 1.0);
    outUV = uv;
}