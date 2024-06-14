#version 460 core

vec4 positions[3] = vec4[3](
    vec4(-0.5f, -0.5f, 0.0f, 1.0f),
    vec4(0.0f, 0.5f, 0.0f, 1.0f),
    vec4(0.0f, -0.5f, 0.0f, 1.0f)
);

void main () {


    gl_Position = positions[gl_VertexID];

}
