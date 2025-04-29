#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec2 outUV;

layout(binding = 0) uniform UBO {
    mat4 world;
} ubos[];

layout(push_constant) uniform DrawParameters {
    mat4 viewProjection;
    int world;
} drawParams;

void main() {
    gl_Position = drawParams.viewProjection * ubos[drawParams.world].world * vec4(position, 1.0);
    outUV = uv;
}