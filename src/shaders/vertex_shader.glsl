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
    // vec3 padding;
    vec4 branchDir;
};

layout(std140, binding = 2) readonly buffer ParticleSSBOIn {
   BranchComputeData particlesIn[ ];
};

layout (location = 0) out vec4 out_color;

vec4 quat_mult(vec4 q1, vec4 q2) { 
  vec4 qr;
  
  qr.x = (q1.w * q2.x) + (q1.x * q2.w) + (q1.y * q2.z) - (q1.z * q2.y);
  qr.y = (q1.w * q2.y) - (q1.x * q2.z) + (q1.y * q2.w) + (q1.z * q2.x);
  qr.z = (q1.w * q2.z) + (q1.x * q2.y) - (q1.y * q2.x) + (q1.z * q2.w);
  qr.w = (q1.w * q2.w) - (q1.x * q2.x) - (q1.y * q2.y) - (q1.z * q2.z);

  return qr;
}

vec3 rotate_vertex_position(vec3 position, vec4 qRot) {   
  vec3 v = position.xyz;
  return v + 2.0 * cross(qRot.xyz, cross(qRot.xyz, v) + qRot.w * v);
}

void main () {

    out_color = in_color;

    vec3 pos = in_position;    
    
    BranchComputeData branchData = particlesIn[in_branchIndex];
    vec4 qRot = branchData.orientation;
    vec4 branchDir = -branchData.branchDir;
    int parentIndex = branchData.parentIndex;
    while (parentIndex >= 0) {
        BranchComputeData parentBranchData = particlesIn[parentIndex];        

        vec4 qRotParent = branchData.orientation;
        
        qRot = quat_mult(qRotParent, qRot);

        vec4 parentDir = branchData.branchDir;
        branchDir -= parentDir;

        parentIndex = parentBranchData.parentIndex;
    }

    branchDir *= -1.0f;


    pos = rotate_vertex_position(pos, qRot);
    // pos += branchDir.xyz;

    // pos = rotate_vertex_position(in_position, qRot);

    // gl_Position = positions[gl_VertexID];
    gl_Position = proj * view * vec4(pos, 1.0f);

}
