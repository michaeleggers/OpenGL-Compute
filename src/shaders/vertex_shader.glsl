#version 460 core

vec4 positions[3] = vec4[3](
    vec4(-0.5f, -0.5f, -1.0f, 1.0f),
    vec4(0.0f, 0.5f, -1.0f, 1.0f),
    vec4(0.5f, -0.5f, -1.0f, 1.0f)
);

layout (location = 0) in vec3  in_position;
layout (location = 1) in vec4  in_color;
layout (location = 2) in float in_angle;


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

    vec4 pos = vec4(in_position, 1.0f);
    pos.x += in_angle;

    // gl_Position = positions[gl_VertexID];
    gl_Position = proj * view * pos;

}
