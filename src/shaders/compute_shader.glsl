#version 460 core

// NOTE: std430 is very important if we want our data NOT to be rounded
// up to 16 bytes. So if we want to have an array of floats each element
// will be exactly aligned to 4 bytes. In std140 this would not work because
// each float would be rounded up to 16 bytes which makes you miss
// 3 elements with each index increment.
//
// See: https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL)#:~:text=vec3%20at%20all.-,std430,-%3A%20This%20layout%20works

layout(std430, binding = 0) readonly buffer ParticleSSBOIn {
   float particlesIn[ ];
};

layout(std430, binding = 1) buffer ParticleSSBOOut {
   float particlesOut[ ];
};

layout(std140, binding = 2) uniform GlobalData {
   float deltaTime;
   int   numVertices;
};

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

void main() {
   uint index = gl_GlobalInvocationID.x;  

   if (index < numVertices) {
      float particleIn = particlesIn[index];

      particlesOut[index] = particleIn;
      particlesOut[index].x += 0.01f * deltaTime;    
         
   }

   // barrier();   
}
