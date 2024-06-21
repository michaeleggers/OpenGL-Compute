#version 460 core

layout (location = 0) in vec3 in_worldspace_pos;
layout (location = 1) in vec4 in_color;

out vec4 FragColor;

void main () {

    float color_strength = smoothstep(-10.0f, 50.0f, in_worldspace_pos.z);
    float shading_factor = 1.0f;

    FragColor = shading_factor * color_strength * in_color;
}

