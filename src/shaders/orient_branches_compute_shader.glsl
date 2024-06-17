#version 460 core

// NOTE: std430 is very important if we want our data NOT to be rounded
// up to 16 bytes. So if we want to have an array of floats each element
// will be exactly aligned to 4 bytes. In std140 this would not work because
// each float would be rounded up to 16 bytes which makes you miss
// 3 elements with each index increment.
//
// See: https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL)#:~:text=vec3%20at%20all.-,std430,-%3A%20This%20layout%20works

struct BranchComputeData {
	vec4 orientation; // 16 bytes
	int  parentIndex;  // -1 = Root branch // 20 bytes
   // vec3 padding; // 32 bytes
   vec4 branchDir; // 48 bytes
};

layout(std140, binding = 0) readonly buffer BranchSSBOIn {
   BranchComputeData branchesIn[ ];
};

layout(std140, binding = 1) buffer BranchSSBOOut {
   BranchComputeData branchesOut[ ];
};

layout(std140, binding = 2) uniform GlobalData {
   float deltaTime;
   float totalTime;
   int   numBranches;
};

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

vec4 quat_from_axis_angle(vec3 axis, float angle) { 
  vec4 qr;
  
  float half_angle = (angle * 0.5) * 3.14159 / 180.0;
  qr.x = axis.x * sin(half_angle);
  qr.y = axis.y * sin(half_angle);
  qr.z = axis.z * sin(half_angle);
  qr.w = cos(half_angle);

  return qr;
}

vec4 quat_conj(vec4 q) { 
  return vec4(-q.x, -q.y, -q.z, q.w); 
}
  
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

void main() {
   uint index = gl_GlobalInvocationID.x;  

   if (index < numBranches) {
      BranchComputeData branchIn = branchesIn[index];      

      vec4 qRot = branchIn.orientation;

      float angle = sin(0.001f * totalTime);
      angle *= 20.0f;
      vec4 qRotAdd = quat_from_axis_angle(vec3(0.0f, 0.0f, 1.0f), angle);

      vec4 branchDir = branchIn.branchDir;
      vec3 newBranchDir = rotate_vertex_position(branchDir.xyz, qRotAdd);

      // particlesOut[index].rotationAxis = vec4(42.0f); // = branchIn;
      branchesOut[index] = branchIn;
      branchesOut[index].orientation = qRotAdd; //quat_mult(qRotAdd, qRot);
      branchesOut[index].branchDir = vec4(newBranchDir, 1.0f);


      // particlesOut[index].x += 0.01f * deltaTime;    
         
   }

   // barrier();   
}
