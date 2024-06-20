#version 460 core

struct BranchComputeData {
	vec4 orientation; // 16 bytes
	int  parentIndex;  // -1 = Root branch // 20 bytes
    int  vertexIndexStart;
    int  vertexIndexEnd;
    int  depth; 
    vec4 branchDir; 
};

struct Vertex {
	vec3 pos;    
	vec4 color;	
};

layout (std140, binding = 0) readonly buffer BranchSSBOIn {
   BranchComputeData branchesIn[ ];
};

layout (std140, binding = 1) readonly buffer VertexSSBOIn {
    Vertex verticesIn[ ];
};

layout (std140, binding = 2) buffer VertexSSBOOut {
    Vertex verticesOut[ ];
};

layout(std140, binding = 3) uniform GlobalData {
   float deltaTime;
   float totalTime;
   int   numBranches;
   int   maxDepth;
   vec3  rotationAxis;
   // float padding;
   vec3  strength;
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

        BranchComputeData branch = branchesIn[index];

        verticesOut[branch.vertexIndexStart] = verticesIn[branch.vertexIndexStart];
        verticesOut[branch.vertexIndexEnd] = verticesIn[branch.vertexIndexEnd];

        int parentIndex = branch.parentIndex;
        vec4 pos = -branch.branchDir;

        while (parentIndex >= 0) {
            
            BranchComputeData parentBranch = branchesIn[parentIndex];
            pos -= parentBranch.branchDir;

            parentIndex = parentBranch.parentIndex;            
        }
        
        pos *= -1.0f;
        vec4 posStart = pos - branch.branchDir;
        
        // verticesOut[branch.vertexIndexEnd].pos += branch.branchDir.xzy;
        // verticesOut[branch.vertexIndexEnd].color = vec4(1.0f, 1.0f, 1.0f, 1.0f);

        verticesOut[branch.vertexIndexStart].pos = posStart.xyz;
        verticesOut[branch.vertexIndexEnd].pos = pos.xyz;

        // verticesOut[branch.vertexIndexStart] = verticesIn[branch.vertexIndexStart];
        // verticesOut[branch.vertexIndexEnd] = verticesIn[branch.vertexIndexEnd];

        // verticesOut[index] = verticesIn[index]; // passthrough        
   }

   // barrier();   
}
