#version 460 core

vec4 positions[3] = vec4[3](
    vec4(-0.5f, -0.5f, -1.0f, 1.0f),
    vec4(0.0f, 0.5f, -1.0f, 1.0f),
    vec4(0.5f, -0.5f, -1.0f, 1.0f)
);

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_color;
layout (location = 2) in uint in_branchIndex;
layout (location = 3) in vec3 in_position_cs;
layout (location = 4) in vec4 in_color_cs;
layout (location = 5) in uint in_branchIndex_cs;


layout (std140, binding = 0) uniform ViewProjMatrices {
    mat4 view;
    mat4 proj;
};

layout (std140, binding = 1) uniform Settings {
    uint someBits;
};

layout (location = 0) out vec4 out_color;

void main () {

    out_color = in_color;

    vec3 pos = in_position_cs;    
    
    gl_Position = proj * view * vec4(pos, 1.0f);

}
