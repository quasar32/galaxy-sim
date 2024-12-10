#version 330 core

uniform sampler2D tex;
in vec2 vs_uv;
in vec3 vs_rgb;
out vec4 rgba;

void main() {
    rgba = vec4(vs_rgb, texture(tex, vs_uv).r);
}
