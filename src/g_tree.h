#ifndef _G_TREE_H_
#define _G_TREE_H_

#include "r_main.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/quaternion.hpp>

#include <vector>

struct BranchComputeData {
	int       parentIndex;  // -1 = Root branch
	int       vertexIndexStart;
	int		  vertexIndexEnd;
	int		  depth;
	glm::vec4 branchDir;
};

struct Branch {
	Vertex			  start;
	Vertex			  end;
	BranchComputeData computeData;
};

std::vector<Branch> CreateTree(glm::vec3 root_start, glm::vec3 root_end, float branch_angle, int max_depth);
void                AddBranchRec(Branch root, float angle, float branch_length, std::vector<Branch>& branch_list, int max_depth, int parentID, int current_depth);
glm::vec4			CreateColorForDepthPct(float treeDepthPct);


#endif
