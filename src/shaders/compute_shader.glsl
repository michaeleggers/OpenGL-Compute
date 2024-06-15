#version 460 core

layout(std140, binding = 0) readonly buffer ParticleSSBOIn {
   float particlesIn[ ];
};

layout(std140, binding = 1) buffer ParticleSSBOOut {
   float particlesOut[ ];
};

layout(std140, binding = 2) uniform GlobalData {
   float deltaTime;
   int   numVertices;
};

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

void main() {
    uint index = gl_GlobalInvocationID.x;  
        
    float particleIn = particlesIn[index];

    particlesOut[index] = particleIn;
    particlesOut[index].x += deltaTime;    

    // barrier();
    
}
