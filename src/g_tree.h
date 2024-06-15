#ifndef _G_TREE_H_
#define _G_TREE_H_

#include "r_main.h"

#include <vector>

struct Branch {
	Vertex start;
	Vertex end;
};

std::vector<Vertex> CreateTree(glm::vec3 root_start, glm::vec3 root_end, float branch_angle, int max_depth);
void                AddBranchRec(Branch root, float angle, float branch_length, std::vector<Branch>& branch_list, int max_depth);

#endif
