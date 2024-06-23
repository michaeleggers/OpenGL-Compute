#version 460 core

// NOTE: std430 is very important if we want our data NOT to be rounded
// up to 16 bytes. So if we want to have an array of floats each element
// will be exactly aligned to 4 bytes. In std140 this would not work because
// each float would be rounded up to 16 bytes which makes you miss
// 3 elements with each index increment.
//
// See: https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL)#:~:text=vec3%20at%20all.-,std430,-%3A%20This%20layout%20works

struct BranchComputeData {	
	int  parentIndex;  // -1 = Root branch // 20 bytes
   int  vertexIndexStart;
   int  vertexIndexEnd;
   int  depth;  // hierarchy in tree. 0 is root branch.
   vec4 branchDir; 
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
   int   maxDepth;
   vec3  rotationAxis;
   // float padding;
   vec3  strength;
};

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

// The following quaternion functions are taken from:
// https://www.geeks3d.com/20141201/how-to-rotate-a-vertex-by-a-quaternion-in-glsl/
vec4 quat_from_axis_angle(vec3 axis, float angle) { 
  vec4 qr;
  
  float half_angle = (angle * 0.5) * 3.14159 / 180.0;
  qr.x = axis.x * sin(half_angle);
  qr.y = axis.y * sin(half_angle);
  qr.z = axis.z * sin(half_angle);
  qr.w = cos(half_angle);

  return qr;
}
  
vec3 rotate_vertex_position(vec3 position, vec4 qRot) {   
  vec3 v = position.xyz;
  return v + 2.0 * cross(qRot.xyz, cross(qRot.xyz, v) + qRot.w * v);
}

float compute_influence(float treeDepthPct) {
   return 1.0f - exp( -pow(treeDepthPct, 4) / (0.1f / exp(-treeDepthPct)) );
}

// b = vertical shift
float compute_influence_smooth(float b, float treeDepthPct) {
   return b + 0.8f * ( 1.0f - exp( -pow(treeDepthPct, 2) ) );
}

float compute_influence_steep(float k, float treeDepthPct) {
   float b = 0.0f; // vertical shift
   return b + ( (1.0f - b) / (1.0f - exp(-k)) ) * ( 1.0 - exp(-k*pow(treeDepthPct, 2)) );
}

float compute_influence_sigmoid(float b, float k, float treeDepthPct) {
   return b + (1.0f - b) * ( 1 / ( 1+exp(-k*(treeDepthPct-1.0f)) ) );
}

void main() {
   uint index = gl_GlobalInvocationID.x;  

   if (index < numBranches) {
      BranchComputeData branchIn = branchesIn[index];      

      float depth = float(branchIn.depth);
      float maxDepthF = float(maxDepth);
      float treeDepthPct = depth / maxDepthF;
      // float influence = compute_influence_steep(2.7f, treeDepthPct); 
      // influence = compute_influence_sigmoid(0.0f, 5.0f, treeDepthPct);     
      float influence = compute_influence_smooth(0.02f, treeDepthPct);      

      float angle = strength.x * influence * sin(0.0001f * deltaTime + 0.001f * totalTime);
      vec4 qRotAdd = quat_from_axis_angle( normalize(rotationAxis), angle );

      vec4 branchDir = branchIn.branchDir;
      vec3 newBranchDir = rotate_vertex_position(branchDir.xyz, qRotAdd);
      
      branchesOut[index] = branchIn;      
      branchesOut[index].branchDir = vec4(newBranchDir, 1.0f);         
   }
}
