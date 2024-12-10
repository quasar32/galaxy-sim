#version 330 core

layout(location = 0) in vec3 xyz;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 rgb;

out vec2 vs_uv;
out vec3 vs_rgb;

uniform mat4 proj;

void main() {
    gl_Position = proj * vec4(xyz, 1.0f); 
    vs_uv = uv;
    vs_rgb = rgb;
}
