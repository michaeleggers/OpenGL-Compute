#version 460 core

vec4 positions[3] = vec4[3](
    vec4(-0.5f, -0.5f, -1.0f, 1.0f),
    vec4(0.0f, 0.5f, -1.0f, 1.0f),
    vec4(0.5f, -0.5f, -1.0f, 1.0f)
);

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_color;
layout (location = 2) in uint in_branchIndex;


layout (std140, binding = 0) uniform ViewProjMatrices {
    mat4 view;
    mat4 proj;
};

layout (std140, binding = 1) uniform Settings {
    uint someBits;
};

struct BranchComputeData {
	vec4 orientation;
	int  parentIndex;  // -1 = Root branch
};

layout(std140, binding = 2) readonly buffer ParticleSSBOIn {
   BranchComputeData particlesIn[ ];
};

layout (location = 0) out vec4 out_color;

vec3 rotate_vertex_position(vec3 position, vec4 qRot) {   
  vec3 v = position.xyz;
  return v + 2.0 * cross(qRot.xyz, cross(qRot.xyz, v) + qRot.w * v);
}

void main () {

    out_color = in_color;

    vec3 pos = in_position;    
    
    BranchComputeData branchData = particlesIn[in_branchIndex];
    pos.x += branchData.orientation.z;


    // pos = rotate_vertex_position(in_position, qRot);

    // gl_Position = positions[gl_VertexID];
    gl_Position = proj * view * vec4(pos, 1.0f);

}
