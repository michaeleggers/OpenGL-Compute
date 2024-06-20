#version 460 core

vec4 positions[3] = vec4[3](
    vec4(-0.5f, -0.5f, -1.0f, 1.0f),
    vec4(0.0f, 0.5f, -1.0f, 1.0f),
    vec4(0.5f, -0.5f, -1.0f, 1.0f)
);

// layout (location = 0) in vec3 in_position;
// layout (location = 1) in vec4 in_color;
// layout (location = 2) in int  in_branchIndex;
layout (location = 0) in vec4 in_position_cs;
layout (location = 1) in vec4 in_color_cs;
layout (location = 2) in int  in_branchIndex_cs;


layout (std140, binding = 0) uniform ViewProjMatrices {
    mat4 view;
    mat4 proj;
};

layout (std140, binding = 1) uniform Settings {
    uint someBits;
};

layout (location = 0) out vec3 out_worldspace_pos;
layout (location = 1) out vec4 out_color;

void main () {

    out_worldspace_pos = in_position_cs.xyz;
    out_color = in_color_cs;

    vec3 pos = in_position_cs.xyz;    
    
    gl_Position = proj * view * vec4(pos, 1.0f);

}
